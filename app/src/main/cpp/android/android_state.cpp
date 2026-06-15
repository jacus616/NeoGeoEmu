/**
 * android_state.cpp — Save state support for FBNeo on Android.
 *
 * FBNeo's savestate system uses BurnAreaScan() to discover all
 * memory regions, then BurnAcb() callback to serialize/deserialize.
 * We implement BurnAcb to write/read a memory buffer.
 *
 * File format: FBNeo's "FB1 " / "FS1 " format (handled by state.cpp)
 */

#include "globals.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

// ============================================================
// FBNeo callback declarations
// ============================================================
extern "C" {
    extern INT32 (__cdecl *BurnAcb)(struct BurnArea* pba);
    INT32 BurnStateSave(TCHAR* szName, INT32 bAll);
    INT32 BurnStateLoad(TCHAR* szName, INT32 bAll, INT32 (*pLoadGame)());
    INT32 BurnAreaScan(INT32 nAction, INT32* pnMin);
}

namespace savestate {

// ============================================================
// Savestate buffer for BurnAcb callback
// ============================================================
static struct {
    uint8_t* buffer   = nullptr;
    size_t   size     = 0;
    size_t   pos      = 0;
    size_t   capacity = 0;
    bool     writing  = true;  // true=save, false=load
} g_state;

// Savestate directory
static char g_saveDir[MAX_PATH] = {0};

// ============================================================
// BurnAcb callback — FBNeo calls this for each memory region
// ============================================================
extern "C" INT32 BurnAcb(struct BurnArea* pba) {
    if (!pba || !g_state.buffer) return 1;

    if (g_state.writing) {
        // Saving: copy from Data to buffer
        if (g_state.pos + pba->nLen > g_state.capacity) {
            // Grow buffer
            size_t newCap = (g_state.pos + pba->nLen) * 2;
            uint8_t* newBuf = (uint8_t*)realloc(g_state.buffer, newCap);
            if (!newBuf) {
                LOGE("Savestate buffer realloc failed");
                return 1;
            }
            g_state.buffer = newBuf;
            g_state.capacity = newCap;
        }
        memcpy(g_state.buffer + g_state.pos, pba->Data, pba->nLen);
        g_state.pos += pba->nLen;
    } else {
        // Loading: copy from buffer to Data
        if (g_state.pos + pba->nLen > g_state.size) {
            LOGE("Savestate buffer underrun");
            return 1;
        }
        memcpy(pba->Data, g_state.buffer + g_state.pos, pba->nLen);
        g_state.pos += pba->nLen;
    }

    return 0;
}

// ============================================================
// Public API
// ============================================================

void setSaveDirectory(const char* dir) {
    if (dir) {
        strncpy(g_saveDir, dir, MAX_PATH - 1);
        g_saveDir[MAX_PATH - 1] = '\0';
    }
}

const char* getSaveDirectory() {
    return g_saveDir;
}

/**
 * Save state to a file.
 * Uses FBNeo's BurnStateSave() which internally calls BurnAreaScan()
 * with our BurnAcb callback.
 */
bool saveToFile(const char* filename) {
    if (!filename || !filename[0]) return false;

    // Initialize buffer
    g_state.capacity = 1024 * 1024; // 1MB initial
    g_state.buffer = (uint8_t*)malloc(g_state.capacity);
    if (!g_state.buffer) return false;
    g_state.pos = 0;
    g_state.writing = true;

    // Use FBNeo's state save function
    // BurnStateSave handles the FB1/FS1 format header
    INT32 result = BurnStateSave(const_cast<TCHAR*>(filename), 1); // bAll=1

    free(g_state.buffer);
    g_state.buffer = nullptr;

    if (result == 0) {
        LOGI("State saved: %s", filename);
        return true;
    } else {
        LOGE("State save failed: %s (error %d)", filename, result);
        return false;
    }
}

/**
 * Load state from a file.
 */
bool loadFromFile(const char* filename) {
    if (!filename || !filename[0]) return false;

    // Read file into buffer
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        LOGE("Cannot open state file: %s", filename);
        return false;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    g_state.buffer = (uint8_t*)malloc(fileSize);
    if (!g_state.buffer) {
        fclose(fp);
        return false;
    }

    size_t bytesRead = fread(g_state.buffer, 1, fileSize, fp);
    fclose(fp);

    if ((long)bytesRead != fileSize) {
        LOGE("State file read error");
        free(g_state.buffer);
        g_state.buffer = nullptr;
        return false;
    }

    g_state.size = fileSize;
    g_state.pos = 0;
    g_state.writing = false;

    // Use FBNeo's state load function
    INT32 result = BurnStateLoad(const_cast<TCHAR*>(filename), 1, nullptr);

    free(g_state.buffer);
    g_state.buffer = nullptr;

    if (result == 0) {
        LOGI("State loaded: %s", filename);
        return true;
    } else {
        LOGE("State load failed: %s (error %d)", filename, result);
        return false;
    }
}

/**
 * Generate savestate filename for a slot.
 */
void getSlotFilename(int slot, const char* gameId, char* outPath, int maxLen) {
    if (g_saveDir[0]) {
        snprintf(outPath, maxLen, "%s/%s.fs%d", g_saveDir, gameId, slot);
    } else {
        snprintf(outPath, maxLen, "/data/data/org.neogeoemu.app/files/states/%s.fs%d",
                 gameId, slot);
    }
}

} // namespace savestate
