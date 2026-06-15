/**
 * android_audio.cpp — OpenSL ES audio output for FBNeo.
 * FBNeo fills pBurnSoundOut with 16-bit stereo PCM.
 * We buffer it via OpenSL ES buffer queue.
 */

#include "globals.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <cstring>
#include <atomic>

namespace audio {

struct State {
    // OpenSL ES objects
    SLObjectItf engineObj  = nullptr;
    SLEngineItf engine     = nullptr;
    SLObjectItf outputMix  = nullptr;
    SLObjectItf playerObj  = nullptr;
    SLPlayItf   playerPlay = nullptr;
    SLAndroidSimpleBufferQueueItf playerBufQueue = nullptr;

    // Audio config (from FBNeo)
    int sampleRate = NEOGEO_SAMPLE_RATE;
    int bufferFrames = NEOGEO_AUDIO_BUFFER_SIZE;
    int channels = 2;  // stereo

    // Double buffer
    static constexpr int NUM_BUFFERS = 2;
    int16_t buffers[NUM_BUFFERS][NEOGEO_AUDIO_BUFFER_SIZE * 2]; // stereo
    int currentBuffer = 0;

    bool initialized = false;
    bool playing = false;
    int volume = 100; // 0-100
};

static State g;

// ============================================================
// Buffer queue callback
// ============================================================
static void bufferQueueCallback(SLAndroidSimpleBufferQueueItf bq, void* /*context*/) {
    // This is called when OpenSL ES has finished playing a buffer.
    // We need to fill the next buffer with audio data from FBNeo.
    // For now, fill with silence — the actual audio data comes from
    // the emulation thread calling fillBuffer().
    (void)bq;
}

// ============================================================
// Public API
// ============================================================

bool init(int sampleRate, int bufferFrames) {
    if (g.initialized) return true;

    g.sampleRate = sampleRate;
    g.bufferFrames = bufferFrames;

    SLresult result;

    // Create engine
    result = slCreateEngine(&g.engineObj, 0, nullptr, 0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("slCreateEngine failed: %d", result);
        return false;
    }
    result = (*g.engineObj)->Realize(g.engineObj, SL_BOOLEAN_FALSE);
    result = (*g.engineObj)->GetInterface(g.engineObj, SL_IID_ENGINE, &g.engine);

    // Create output mix
    result = (*g.engine)->CreateOutputMix(g.engine, &g.outputMix, 0, nullptr, nullptr);
    result = (*g.outputMix)->Realize(g.outputMix, SL_BOOLEAN_FALSE);

    // Configure audio source
    SLDataLocator_AndroidSimpleBufferQueue locBufq = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, State::NUM_BUFFERS
    };

    SLDataFormat_PCM formatPcm = {
        SL_DATAFORMAT_PCM,
        2,                              // stereo
        SL_SAMPLINGRATE_44_1,           // 44.1kHz
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
        SL_BYTEORDER_LITTLEENDIAN
    };

    SLDataSource audioSrc = { &locBufq, &formatPcm };

    SLDataLocator_OutputMix locOutmix = {
        SL_DATALOCATOR_OUTPUTMIX, g.outputMix
    };
    SLDataSink audioSnk = { &locOutmix, nullptr };

    // Create audio player
    SLInterfaceID ids[] = { SL_IID_BUFFERQUEUE, SL_IID_VOLUME };
    SLboolean     req[] = { SL_BOOLEAN_TRUE,    SL_BOOLEAN_TRUE };

    result = (*g.engine)->CreateAudioPlayer(
        g.engine, &g.playerObj, &audioSrc, &audioSnk, 2, ids, req
    );
    if (result != SL_RESULT_SUCCESS) {
        LOGE("CreateAudioPlayer failed: %d", result);
        return false;
    }

    result = (*g.playerObj)->Realize(g.playerObj, SL_BOOLEAN_FALSE);
    result = (*g.playerObj)->GetInterface(g.playerObj, SL_IID_PLAY, &g.playerPlay);
    result = (*g.playerObj)->GetInterface(g.playerObj, SL_IID_BUFFERQUEUE, &g.playerBufQueue);
    result = (*g.playerObj)->GetInterface(g.playerObj, SL_IID_VOLUME, nullptr);

    // Register callback
    result = (*g.playerBufQueue)->RegisterCallback(g.playerBufQueue, bufferQueueCallback, nullptr);

    // Clear buffers
    memset(g.buffers, 0, sizeof(g.buffers));

    g.initialized = true;
    LOGI("Audio initialized: %dHz, %d frames/buffer", sampleRate, bufferFrames);
    return true;
}

void shutdown() {
    if (!g.initialized) return;

    if (g.playerObj) {
        (*g.playerObj)->Destroy(g.playerObj);
        g.playerObj = nullptr;
    }
    if (g.outputMix) {
        (*g.outputMix)->Destroy(g.outputMix);
        g.outputMix = nullptr;
    }
    if (g.engineObj) {
        (*g.engineObj)->Destroy(g.engineObj);
        g.engineObj = nullptr;
    }

    g.initialized = false;
    g.playing = false;
    LOGI("Audio shutdown");
}

void play() {
    if (!g.initialized || g.playing) return;

    // Enqueue initial buffers (silence)
    for (int i = 0; i < State::NUM_BUFFERS; i++) {
        memset(g.buffers[i], 0, g.bufferFrames * g.channels * sizeof(int16_t));
        (*g.playerBufQueue)->Enqueue(g.playerBufQueue, g.buffers[i],
                                     g.bufferFrames * g.channels * sizeof(int16_t));
    }

    (*g.playerPlay)->SetPlayState(g.playerPlay, SL_PLAYSTATE_PLAYING);
    g.playing = true;
    LOGI("Audio playback started");
}

void stop() {
    if (!g.initialized || !g.playing) return;

    (*g.playerPlay)->SetPlayState(g.playerPlay, SL_PLAYSTATE_STOPPED);
    (*g.playerBufQueue)->Clear(g.playerBufQueue);
    g.playing = false;
    LOGI("Audio playback stopped");
}

void pause() {
    if (!g.initialized) return;
    (*g.playerPlay)->SetPlayState(g.playerPlay, SL_PLAYSTATE_PAUSED);
    g.playing = false;
}

void resume() {
    if (!g.initialized) return;
    (*g.playerPlay)->SetPlayState(g.playerPlay, SL_PLAYSTATE_PLAYING);
    g.playing = true;
}

/**
 * Submit audio data from FBNeo's pBurnSoundOut.
 * Called after BurnSoundRender() fills the buffer.
 */
void submitBuffer(const int16_t* data, int numFrames) {
    if (!g.initialized || !g.playing) return;

    int bytes = numFrames * g.channels * sizeof(int16_t);

    // Get next buffer slot
    int idx = g.currentBuffer;
    g.currentBuffer = (g.currentBuffer + 1) % State::NUM_BUFFERS;

    // Copy audio data
    int copyBytes = bytes;
    if (copyBytes > (int)sizeof(g.buffers[0])) {
        copyBytes = sizeof(g.buffers[0]);
    }
    memcpy(g.buffers[idx], data, copyBytes);

    // Enqueue
    (*g.playerBufQueue)->Enqueue(g.playerBufQueue, g.buffers[idx], copyBytes);
}

void setVolume(int vol) {
    g.volume = vol;
    // OpenSL ES volume control would go here
    // SLVolumeItf volumeItf;
    // (*g.playerObj)->GetInterface(g.playerObj, SL_IID_VOLUME, &volumeItf);
    // (*volumeItf)->SetVolumeLevel(volumeItf, millibels);
}

bool isInitialized() {
    return g.initialized;
}

bool isPlaying() {
    return g.playing;
}

int getSampleRate() {
    return g.sampleRate;
}

int getBufferFrames() {
    return g.bufferFrames;
}

} // namespace audio
