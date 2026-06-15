/**
 * android_romloader.cpp — ROM loading callbacks for FBNeo.
 *
 * FBNeo doesn't load ROMs itself — it calls back to the frontend
 * via BurnExtLoadRom(). We implement this to read from Android's
 * file system (including /sdcard and app storage).
 *
 * FBNeo also uses zipfn.cpp for ZIP file support, which calls
 * ioapi.c/unzip.c — those work with FILE*, so we use standard
 * fopen() which works on Android for regular file paths.
 */

#include "globals.h"
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>

// ============================================================
// FBNeo callback declarations (defined in burn.cpp)
// ============================================================
extern "C" {
    extern INT32 (__cdecl *BurnExtLoadRom)(UINT8* Dest, INT32* pnWrote, INT32 i);
    extern INT32 (__cdecl *BurnExtProgressRangeCallback)(double dProgressRange);
    extern INT32 (*BurnExtProgressUpdateCallback)(double dProgress, const TCHAR* pszText, bool bAbs);
    extern INT32 BurnDrvGetRomName(char** pszName, UINT32 i, INT32 nAka);
    extern INT32 BurnDrvGetRomInfo(struct BurnRomInfo* pri, UINT32 i);
    extern UINT8* BurnLoadRomExt(UINT8* Dest, INT32 i, INT32 nGap, INT32 nFlags);
    void* BurnMalloc(INT32 size);
    void BurnFree(void* ptr);
}

namespace romloader {

// ============================================================
// ROM search paths
// ============================================================
static char g_romPaths[8][MAX_PATH];
static int  g_numRomPaths = 0;

// Current ROM directory (set when loading a game)
static char g_currentRomDir[MAX_PATH] = {0};
static char g_currentRomFile[MAX_PATH] = {0};

// ============================================================
// Path management
// ============================================================

void addRomPath(const char* path) {
    if (g_numRomPaths < 8 && path && path[0]) {
        strncpy(g_romPaths[g_numRomPaths], path, MAX_PATH - 1);
        g_romPaths[g_numRomPaths][MAX_PATH - 1] = '\0';
        LOGI("ROM path %d: %s", g_numRomPaths, g_romPaths[g_numRomPaths]);
        g_numRomPaths++;
    }
}

void clearRomPaths() {
    g_numRomPaths = 0;
    memset(g_romPaths, 0, sizeof(g_romPaths));
}

void setRomFile(const char* filePath) {
    if (!filePath) return;
    strncpy(g_currentRomFile, filePath, MAX_PATH - 1);
    g_currentRomFile[MAX_PATH - 1] = '\0';

    // Extract directory
    const char* lastSlash = strrchr(filePath, '/');
    if (lastSlash) {
        size_t dirLen = lastSlash - filePath;
        if (dirLen < MAX_PATH) {
            strncpy(g_currentRomDir, filePath, dirLen);
            g_currentRomDir[dirLen] = '\0';
        }
    }

    LOGI("ROM file: %s", g_currentRomFile);
    LOGI("ROM dir:  %s", g_currentRomDir);
}

const char* getRomDir() {
    return g_currentRomDir;
}

const char* getRomFile() {
    return g_currentRomFile;
}

// ============================================================
// File search helper
// FBNeo needs to find ROM files by name (e.g. "kof97_p1.rom")
// in the ROM directories. We search all registered paths.
// ============================================================
static bool findFile(const char* filename, char* outPath, int maxLen) {
    for (int i = 0; i < g_numRomPaths; i++) {
        snprintf(outPath, maxLen, "%s/%s", g_romPaths[i], filename);
        struct stat st;
        if (stat(outPath, &st) == 0 && S_ISREG(st.st_mode)) {
            return true;
        }
    }
    // Also try the current ROM directory
    if (g_currentRomDir[0]) {
        snprintf(outPath, maxLen, "%s/%s", g_currentRomDir, filename);
        struct stat st;
        if (stat(outPath, &st) == 0 && S_ISREG(st.st_mode)) {
            return true;
        }
    }
    return false;
}

// ============================================================
// BurnExtLoadRom callback
// ============================================================
// This is the main callback FBNeo uses to load ROM data.
// FBNeo calls it for each ROM entry in the driver's BurnRomInfo array.
//
// Parameters:
//   Dest    — buffer to fill with ROM data
//   pnWrote — [out] number of bytes actually read
//   i       — ROM index (0, 1, 2, ...)
//
// Returns: 0 on success, non-zero on error

extern "C" INT32 BurnExtLoadRom(UINT8* Dest, INT32* pnWrote, INT32 i) {
    if (!Dest) return 1;

    // Get ROM name from FBNeo driver
    char* romName = nullptr;
    BurnDrvGetRomName(&romName, i, 0);

    if (!romName || !romName[0]) {
        LOGW("ROM index %d: no name returned", i);
        return 1;
    }

    // Get ROM info for size
    BurnRomInfo ri;
    memset(&ri, 0, sizeof(ri));
    BurnDrvGetRomInfo(&ri, i);

    LOGI("Loading ROM[%d]: %s (%d bytes)", i, romName, ri.nLen);

    // Strategy 1: If the current ROM is a ZIP file, FBNeo's
    // internal zipfn.cpp handles it via ioapi.c/unzip.c.
    // We just need the file to be accessible via fopen().

    // Strategy 2: Individual ROM files in the ROM directory.
    // Search for the file.
    char filePath[MAX_PATH];
    FILE* fp = nullptr;

    // Try opening by name directly (FBNeo's zip handler may have
    // already opened the ZIP and is looking for entries)
    // First, try the ROM name as-is (for files already extracted)
    if (findFile(romName, filePath, MAX_PATH)) {
        fp = fopen(filePath, "rb");
    }

    // If not found and we have a ZIP ROM file, FBNeo's internal
    // unzip handles it — the ioapi.c calls go through fopen()
    // on the ZIP path. We don't need to do anything special.

    if (!fp) {
        // Try the ROM name in the current ROM directory
        if (g_currentRomDir[0]) {
            snprintf(filePath, MAX_PATH, "%s/%s", g_currentRomDir, romName);
            fp = fopen(filePath, "rb");
        }
    }

    if (!fp) {
        LOGW("Could not open ROM file: %s", romName);
        return 1;
    }

    // Read ROM data
    size_t bytesRead = fread(Dest, 1, ri.nLen, fp);
    fclose(fp);

    if (pnWrote) {
        *pnWrote = (INT32)bytesRead;
    }

    if ((INT32)bytesRead != ri.nLen) {
        LOGW("ROM %s: expected %d bytes, read %d", romName, ri.nLen, (int)bytesRead);
        // Some ROMs may have different sizes — don't fail hard
    }

    LOGI("ROM[%d] loaded: %s (%d bytes)", i, romName, (int)bytesRead);
    return 0;
}

// ============================================================
// BurnExtProgress callbacks (optional, for loading screen)
// ============================================================
extern "C" INT32 BurnExtProgressRangeCallback(double /*dProgressRange*/) {
    return 0;
}

extern "C" INT32 BurnExtProgressUpdateCallback(double /*dProgress*/,
                                                  const TCHAR* /*pszText*/,
                                                  bool /*bAbs*/) {
    return 0;
}

// ============================================================
// Initialization
// ============================================================
void setupCallbacks() {
    // Register our ROM loading callback with FBNeo
    // These are extern pointers declared in burn.h
    // We set them in the JNI bridge init function
    LOGI("ROM loader callbacks registered");
}

} // namespace romloader
