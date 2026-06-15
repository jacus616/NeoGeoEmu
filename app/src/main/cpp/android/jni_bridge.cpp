/**
 * jni_bridge.cpp — JNI bridge connecting Kotlin to FBNeo core.
 *
 * This is the main entry point for all native calls from Android.
 * It orchestrates:
 *   - FBNeo library initialization (BurnLibInit)
 *   - Game loading (BurnDrvInit via ROM path)
 *   - Frame execution (BurnDrvFrame)
 *   - Video rendering (pBurnDraw → OpenGL ES)
 *   - Audio output (pBurnSoundOut → OpenSL ES)
 *   - Input (Android → FBNeo input system)
 *   - Save states (BurnStateSave/Load)
 */

#include "globals.h"
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <cstring>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

// strcasestr may not be available on all Android API levels
#if defined(__ANDROID__)
#include <cstring>
#include <cctype>
static const char* my_strcasestr(const char* haystack, const char* needle) {
    if (!needle[0]) return haystack;
    for (; *haystack; haystack++) {
        if (tolower(*haystack) == tolower(*needle)) {
            const char* h = haystack + 1;
            const char* n = needle + 1;
            while (*n && tolower(*h) == tolower(*n)) { h++; n++; }
            if (!*n) return haystack;
        }
    }
    return nullptr;
}
#define strcasestr my_strcasestr
#endif

// ============================================================
// FBNeo API declarations (from burn.h / burner.h)
// ============================================================
extern "C" {
    // Library lifecycle
    INT32 BurnLibInit();
    INT32 BurnLibExit();

    // Driver lifecycle
    INT32 BurnDrvInit();
    INT32 BurnDrvExit();
    INT32 BurnDrvFrame();
    INT32 BurnDrvRedraw();

    // Driver info
    INT32 BurnDrvGetVisibleSize(INT32* pnWidth, INT32* pnHeight);
    INT32 BurnDrvGetFullSize(INT32* pnWidth, INT32* pnHeight);
    INT32 BurnDrvGetHardwareCode();
    INT32 BurnDrvGetFlags();
    bool  BurnDrvIsWorking();
    INT32 BurnDrvGetMaxPlayers();
    char* BurnDrvGetTextA(UINT32 i);

    // ROM info
    INT32 BurnDrvGetRomInfo(struct BurnRomInfo* pri, UINT32 i);
    INT32 BurnDrvGetRomName(char** pszName, UINT32 i, INT32 nAka);

    // Input
    INT32 BurnDrvGetInputInfo(struct BurnInputInfo* pii, UINT32 i);

    // State (from state.cpp in burner/)
    INT32 BurnStateSave(TCHAR* szName, INT32 bAll);
    INT32 BurnStateLoad(TCHAR* szName, INT32 bAll, INT32 (*pLoadGame)());
    INT32 BurnStateCompress(UINT8** pDef, INT32* pnDefLen, INT32 bAll);
    INT32 BurnStateDecompress(UINT8* Def, INT32 nDefLen, INT32 bAll);

    // Callbacks (function pointers we must set)
    extern INT32 (__cdecl *BurnExtLoadRom)(UINT8* Dest, INT32* pnWrote, INT32 i);
    extern INT32 (__cdecl *BurnExtProgressRangeCallback)(double dProgressRange);
    extern INT32 (*BurnExtProgressUpdateCallback)(double dProgress, const TCHAR* pszText, bool bAbs);
    extern INT32 (__cdecl *BurnAcb)(struct BurnArea* pba);

    // Video globals (FBNeo writes rendered frame here)
    extern UINT8* pBurnDraw;     // Framebuffer pointer
    extern INT32  nBurnPitch;    // Line pitch in bytes
    extern INT32  nBurnBpp;      // Bytes per pixel (2, 3, or 4)
    extern INT32  nBurnFPS;      // Frame rate * 100 (e.g. 6000 = 60.00Hz)

    // Audio globals (FBNeo fills pBurnSoundOut after BurnSoundRender)
    extern INT32  nBurnSoundRate;    // Sample rate (e.g. 44100)
    extern INT32  nBurnSoundLen;     // Samples per frame
    extern INT16* pBurnSoundOut;     // Output buffer pointer
    extern INT32  nInterpolation;    // ADPCM interpolation level
    extern INT32  nFMInterpolation;  // FM interpolation level

    // Driver globals
    extern UINT32 nBurnDrvCount;     // Total number of drivers
    extern UINT32 nBurnDrvActive;    // Currently active driver index
    extern UINT32 nCurrentFrame;     // Current frame counter

    // Memory allocation
    void* BurnMalloc(INT32 size);
    void  BurnFree(void* ptr);

    // State scan
    INT32 BurnAreaScan(INT32 nAction, INT32* pnMin);
}

// ============================================================
// Forward declarations from other android_*.cpp files
// ============================================================
namespace video {
    bool init(ANativeWindow* window);
    void shutdown();
    void setWindow(ANativeWindow* window);
    void setSize(int w, int h);
    void setFilterMode(int mode);
    void setIntegerScale(bool enable);
    void uploadFramebuffer(const uint16_t* data, int w, int h);
    void render();
    bool isInitialized();
    uint16_t* getFramebuffer();
}

namespace audio {
    bool init(int sampleRate, int bufferFrames);
    void shutdown();
    void play();
    void stop();
    void pause();
    void resume();
    void submitBuffer(const int16_t* data, int numFrames);
    void setVolume(int vol);
    bool isInitialized();
    bool isPlaying();
    int getSampleRate();
    int getBufferFrames();
}

namespace input {
    void setInput(int id, bool pressed);
    bool getInput(int id);
    void resetAll();
    void initMapping();
    void applyToFBNeo();
}

namespace romloader {
    void addRomPath(const char* path);
    void clearRomPaths();
    void setRomFile(const char* filePath);
    const char* getRomDir();
    const char* getRomFile();
    void setupCallbacks();
    extern "C" INT32 BurnExtLoadRomCallback(UINT8* Dest, INT32* pnWrote, INT32 i);
}

namespace savestate {
    void setSaveDirectory(const char* dir);
    const char* getSaveDirectory();
    bool saveToFile(const char* filename);
    bool loadFromFile(const char* filename);
    void getSlotFilename(int slot, const char* gameId, char* outPath, int maxLen);
    extern "C" INT32 BurnAcbCallback(struct BurnArea* pba);
}

// ============================================================
// Emulator state
// ============================================================
struct EmuState {
    std::thread emuThread;
    std::atomic<bool> running{false};
    std::atomic<bool> paused{false};
    std::mutex mutex;
    std::condition_variable cv;

    ANativeWindow* window = nullptr;
    int surfaceW = 0, surfaceH = 0;

    char romPath[MAX_PATH] = {0};
    char romDir[MAX_PATH] = {0};
    char gameId[64] = {0};
    bool romLoaded = false;
    bool fbInitialized = false;

    // Framebuffer for FBNeo to render into
    uint16_t* framebuffer = nullptr;
    int fbWidth  = 320;
    int fbHeight = 224;

    // Audio buffer
    int16_t* audioBuffer = nullptr;
    int audioBufFrames = 0;

    // FPS tracking
    std::atomic<int> currentFPS{0};
};

static EmuState g_emu;

// ============================================================
// Emulation thread
// ============================================================
static void emulationThread() {
    LOGI("Emulation thread started");

    // Frame timing
    const int64_t frameTimeUs = 1000000 / 60; // ~16667us for 60fps
    int64_t frameCount = 0;
    int64_t fpsTimer = 0;

    while (g_emu.running) {
        // Wait if paused
        {
            std::unique_lock<std::mutex> lock(g_emu.mutex);
            g_emu.cv.wait(lock, [] { return !g_emu.paused || !g_emu.running; });
        }
        if (!g_emu.running) break;

        if (!g_emu.romLoaded) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Run one frame of emulation
        // FBNeo renders to pBurnDraw (our framebuffer)
        // and fills pBurnSoundOut with audio
        BurnDrvFrame();

        // Render video
        if (video::isInitialized() && g_emu.framebuffer) {
            video::uploadFramebuffer(g_emu.framebuffer, g_emu.fbWidth, g_emu.fbHeight);
            video::render();
        }

        // Submit audio
        if (audio::isPlaying() && g_emu.audioBuffer) {
            audio::submitBuffer(g_emu.audioBuffer, g_emu.audioBufFrames);
        }

        // FPS counter
        frameCount++;
        fpsTimer += frameTimeUs;
        if (fpsTimer >= 1000000) {
            g_emu.currentFPS = (int)frameCount;
            frameCount = 0;
            fpsTimer = 0;
        }

        // Frame limiter (simple)
        std::this_thread::sleep_for(std::chrono::microseconds(frameTimeUs));
    }

    LOGI("Emulation thread ended");
}

// ============================================================
// Initialize FBNeo callbacks
// ============================================================
static void initFBNeoCallbacks() {
    BurnExtLoadRom = romloader::BurnExtLoadRomCallback;
    BurnExtProgressRangeCallback = nullptr;
    BurnExtProgressUpdateCallback = nullptr;
    BurnAcb = savestate::BurnAcbCallback;
    LOGI("FBNeo callbacks initialized");
}

// ============================================================
// Setup framebuffer for FBNeo to render into
// ============================================================
static bool setupFramebuffer() {
    if (g_emu.framebuffer) free(g_emu.framebuffer);

    g_emu.fbWidth  = 320;  // NeoGeo native width
    g_emu.fbHeight = 224;  // NeoGeo native height

    g_emu.framebuffer = (uint16_t*)malloc(g_emu.fbWidth * g_emu.fbHeight * sizeof(uint16_t));
    if (!g_emu.framebuffer) {
        LOGE("Failed to allocate framebuffer");
        return false;
    }
    memset(g_emu.framebuffer, 0, g_emu.fbWidth * g_emu.fbHeight * sizeof(uint16_t));

    // Tell FBNeo where to render
    pBurnDraw  = (UINT8*)g_emu.framebuffer;
    nBurnPitch = g_emu.fbWidth * sizeof(uint16_t);
    nBurnBpp   = 2; // RGB565

    g_emu.fbInitialized = true;
    LOGI("Framebuffer: %dx%d, pitch=%d", g_emu.fbWidth, g_emu.fbHeight, nBurnPitch);
    return true;
}

// ============================================================
// Find NeoGeo driver index by ROM file
// ============================================================
static int findNeoGeoDriver(const char* romFile) {
    // Extract filename without extension
    const char* fname = strrchr(romFile, '/');
    fname = fname ? fname + 1 : romFile;

    char name[64];
    strncpy(name, fname, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    char* dot = strrchr(name, '.');
    if (dot) *dot = '\0';

    // Search FBNeo driver list for matching game
    UINT32 driverCount = nBurnDrvCount;
    for (UINT32 i = 0; i < driverCount; i++) {
        // Set active driver to query its info
        nBurnDrvActive = i;
        char* drvName = BurnDrvGetTextA(DRV_NAME);
        if (drvName && strcasecmp(drvName, name) == 0) {
            LOGI("Found driver[%d]: %s", i, drvName);
            return (int)i;
        }
    }

    // If not found by exact name, try case-insensitive substring match
    for (UINT32 i = 0; i < driverCount; i++) {
        nBurnDrvActive = i;
        char* drvName = BurnDrvGetTextA(DRV_NAME);
        if (drvName && strcasestr(drvName, name) != nullptr) {
            LOGI("Found driver by substring[%d]: %s (matching %s)", i, drvName, name);
            return (int)i;
        }
    }

    LOGW("No driver found for: %s", name);
    return -1;
}

// ============================================================
// JNI Functions
// ============================================================
extern "C" {

JNIEXPORT jlong JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeCreate(
    JNIEnv* /*env*/, jclass /*clazz*/
) {
    LOGI("Creating emulator instance");

    auto* state = new EmuState();

    // Initialize FBNeo library (once per process)
    static bool fbneoInitialized = false;
    if (!fbneoInitialized) {
        BurnLibInit();
        initFBNeoCallbacks();
        fbneoInitialized = true;
        LOGI("FBNeo library initialized, %d drivers available", nBurnDrvCount);
    }

    return reinterpret_cast<jlong>(state);
}

JNIEXPORT void JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeDestroy(
    JNIEnv* /*env*/, jclass /*clazz*/, jlong handle
) {
    auto* state = reinterpret_cast<EmuState*>(handle);
    if (!state) return;

    LOGI("Destroying emulator instance");

    state->running = false;
    state->paused = false;
    state->cv.notify_all();

    if (state->emuThread.joinable()) {
        state->emuThread.join();
    }

    if (state->romLoaded) {
        BurnDrvExit();
    }

    video::shutdown();
    audio::shutdown();

    free(state->framebuffer);
    free(state->audioBuffer);
    delete state;

    LOGI("Emulator destroyed");
}

JNIEXPORT jboolean JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeLoadRom(
    JNIEnv* env, jclass /*clazz*/, jlong handle, jstring romPath
) {
    auto* state = reinterpret_cast<EmuState*>(handle);
    if (!state) return JNI_FALSE;

    const char* path = env->GetStringUTFChars(romPath, nullptr);
    strncpy(state->romPath, path, MAX_PATH - 1);
    env->ReleaseStringUTFChars(romPath, path);

    // Extract directory
    const char* lastSlash = strrchr(state->romPath, '/');
    if (lastSlash) {
        size_t dirLen = lastSlash - state->romPath;
        strncpy(state->romDir, state->romPath, dirLen);
        state->romDir[dirLen] = '\0';

        // Extract game ID from filename
        const char* fname = lastSlash + 1;
        char* dot = strrchr(const_cast<char*>(fname), '.');
        size_t nameLen = dot ? (size_t)(dot - fname) : strlen(fname);
        strncpy(state->gameId, fname, nameLen);
        state->gameId[nameLen] = '\0';
    }

    LOGI("Loading ROM: %s (dir=%s, id=%s)", state->romPath, state->romDir, state->gameId);

    // Setup ROM paths for loader
    romloader::clearRomPaths();
    romloader::addRomPath(state->romDir);
    romloader::setRomFile(state->romPath);

    // Find the NeoGeo driver
    int drvIndex = findNeoGeoDriver(state->romPath);
    if (drvIndex < 0) {
        LOGE("No NeoGeo driver found for: %s", state->gameId);
        return JNI_FALSE;
    }

    // Select the driver
    nBurnDrvActive = (UINT32)drvIndex;

    // Initialize the driver (loads ROMs)
    INT32 result = BurnDrvInit();
    if (result != 0) {
        LOGE("BurnDrvInit failed: %d", result);
        return JNI_FALSE;
    }

    // Setup framebuffer
    if (!setupFramebuffer()) {
        BurnDrvExit();
        return JNI_FALSE;
    }

    // Get video dimensions from driver
    INT32 visW, visH;
    BurnDrvGetVisibleSize(&visW, &visH);
    LOGI("Visible area: %dx%d", visW, visH);

    // Initialize audio
    int sampleRate = nBurnSoundRate;
    if (sampleRate <= 0) sampleRate = NEOGEO_SAMPLE_RATE;
    int soundLen = nBurnSoundLen;
    if (soundLen <= 0) soundLen = NEOGEO_AUDIO_BUFFER_SIZE;

    audio::init(sampleRate, soundLen);

    // Allocate audio buffer
    state->audioBufFrames = soundLen;
    state->audioBuffer = (int16_t*)malloc(soundLen * 2 * sizeof(int16_t)); // stereo
    pBurnSoundOut = state->audioBuffer;

    // Initialize input mapping
    input::initMapping();

    // Setup savestate directory
    char stateDir[MAX_PATH];
    snprintf(stateDir, MAX_PATH, "/data/data/org.neogeoemu.app/files/states");
    savestate::setSaveDirectory(stateDir);

    state->romLoaded = true;
    LOGI("ROM loaded successfully: %s", state->gameId);
    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeSetSurface(
    JNIEnv* env, jclass /*clazz*/, jlong handle, jobject surface
) {
    auto* state = reinterpret_cast<EmuState*>(handle);
    if (!state) return;

    std::lock_guard<std::mutex> lock(state->mutex);

    if (state->window) {
        ANativeWindow_release(state->window);
        state->window = nullptr;
    }

    if (surface) {
        state->window = ANativeWindow_fromSurface(env, surface);
        LOGI("Surface set: %p", state->window);

        if (!video::isInitialized()) {
            video::init(state->window);
        } else {
            video::setWindow(state->window);
        }
    }
}

JNIEXPORT void JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeSetSurfaceSize(
    JNIEnv* /*env*/, jclass /*clazz*/, jlong handle, jint width, jint height
) {
    auto* state = reinterpret_cast<EmuState*>(handle);
    if (!state) return;

    std::lock_guard<std::mutex> lock(state->mutex);
    state->surfaceW = width;
    state->surfaceH = height;
    video::setSize(width, height);
}

JNIEXPORT void JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeSetInput(
    JNIEnv* /*env*/, jclass /*clazz*/, jlong handle, jint inputId, jboolean pressed
) {
    input::setInput(inputId, pressed == JNI_TRUE);
}

JNIEXPORT void JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeRunFrame(
    JNIEnv* /*env*/, jclass /*clazz*/, jlong handle
) {
    // Not used — frame running is handled by emulation thread
}

JNIEXPORT jboolean JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeSaveState(
    JNIEnv* /*env*/, jclass /*clazz*/, jlong handle, jint slot
) {
    auto* state = reinterpret_cast<EmuState*>(handle);
    if (!state || !state->romLoaded) return JNI_FALSE;

    char filename[MAX_PATH];
    savestate::getSlotFilename(slot, state->gameId, filename, MAX_PATH);

    bool result = savestate::saveToFile(filename);
    return result ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeLoadState(
    JNIEnv* /*env*/, jclass /*clazz*/, jlong handle, jint slot
) {
    auto* state = reinterpret_cast<EmuState*>(handle);
    if (!state || !state->romLoaded) return JNI_FALSE;

    char filename[MAX_PATH];
    savestate::getSlotFilename(slot, state->gameId, filename, MAX_PATH);

    bool result = savestate::loadFromFile(filename);
    return result ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeReset(
    JNIEnv* /*env*/, jclass /*clazz*/, jlong handle
) {
    auto* state = reinterpret_cast<EmuState*>(handle);
    if (!state || !state->romLoaded) return;

    LOGI("Resetting emulation");
    BurnDrvExit();
    BurnDrvInit();
}

JNIEXPORT void JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativePause(
    JNIEnv* /*env*/, jclass /*clazz*/, jlong handle
) {
    auto* state = reinterpret_cast<EmuState*>(handle);
    if (!state) return;

    state->paused = true;
    audio::pause();
    LOGI("Emulation paused");
}

JNIEXPORT void JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeResume(
    JNIEnv* /*env*/, jclass /*clazz*/, jlong handle
) {
    auto* state = reinterpret_cast<EmuState*>(handle);
    if (!state) return;

    state->paused = false;
    state->cv.notify_all();
    audio::resume();
    LOGI("Emulation resumed");
}

JNIEXPORT void JNICALL
Java_org_neogeoemu_app_emulation_EmulatorBridge_nativeStop(
    JNIEnv* /*env*/, jclass /*clazz*/, jlong handle
) {
    auto* state = reinterpret_cast<EmuState*>(handle);
    if (!state) return;

    state->running = false;
    state->paused = false;
    state->cv.notify_all();

    if (state->emuThread.joinable()) {
        state->emuThread.join();
    }

    audio::stop();
    LOGI("Emulation stopped");
}

} // extern "C"
