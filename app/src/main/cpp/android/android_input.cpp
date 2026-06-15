/**
 * android_input.cpp — Input handling for FBNeo on Android.
 * Translates Android touch/gamepad input to FBNeo's input system.
 */

#include "globals.h"

// ============================================================
// FBNeo input mapping
// ============================================================
// NeoGeo has 2 players, each with:
//   - 4 directions (up, down, left, right)
//   - 4 action buttons (A, B, C, D)
//   - Start, Select/Coin
// Plus system inputs: Test, Service

namespace input {

// Maps our Android input IDs to FBNeo input indices.
// The actual mapping depends on the NeoGeo driver's
// BurnInputInfo array returned by BurnDrvGetInputInfo().
struct NeoGeoInputMap {
    int up, down, left, right;
    int buttonA, buttonB, buttonC, buttonD;
    int start, coin;
    bool initialized = false;
};

static NeoGeoInputMap g_player1;
static NeoGeoInputMap g_player2;

// Current input state (set by JNI from Kotlin)
static bool g_inputState[16] = {false};

// FBNeo input info array (populated at init)
static struct BurnInputInfo* g_inputInfo = nullptr;
static int g_inputCount = 0;

// ============================================================
// FBNeo input callback
// ============================================================
// FBNeo calls this via GameInp to read input state.
// We provide the values through this callback system.

void setInput(int id, bool pressed) {
    if (id >= 0 && id < 16) {
        g_inputState[id] = pressed;
    }
}

bool getInput(int id) {
    if (id >= 0 && id < 16) {
        return g_inputState[id];
    }
    return false;
}

void resetAll() {
    memset(g_inputState, 0, sizeof(g_inputState));
}

/**
 * Initialize input mapping from FBNeo's driver.
 * Called once after BurnDrvInit().
 */
void initMapping() {
    if (g_player1.initialized) return;

    // Query FBNeo for input descriptions
    INT32 numInputs = 0;
    // BurnDrvGetInputInfo uses a global counter, iterate until it returns NULL
    BurnInputInfo info;
    memset(&info, 0, sizeof(info));

    // Map the first 10 inputs for player 1
    // FBNeo NeoGeo drivers provide inputs in this order:
    // P1 Up, P1 Down, P1 Left, P1 Right, P1 A, P1 B, P1 C, P1 D, P1 Start, P1 Coin
    g_player1.up     = 0;
    g_player1.down   = 1;
    g_player1.left   = 2;
    g_player1.right  = 3;
    g_player1.buttonA = 4;
    g_player1.buttonB = 5;
    g_player1.buttonC = 6;
    g_player1.buttonD = 7;
    g_player1.start  = 8;
    g_player1.coin   = 9;

    // Player 2 starts at index 10
    g_player2.up     = 10;
    g_player2.down   = 11;
    g_player2.left   = 12;
    g_player2.right  = 13;
    g_player2.buttonA = 14;
    g_player2.buttonB = 15;
    g_player2.buttonC = 16;
    g_player2.buttonD = 17;
    g_player2.start  = 18;
    g_player2.coin   = 19;

    g_player1.initialized = true;
    g_player2.initialized = true;

    LOGI("Input mapping initialized");
}

/**
 * Apply current input state to FBNeo's input system.
 * Called at the start of each frame before BurnDrvFrame().
 * FBNeo reads input through the GameInp structure.
 */
void applyToFBNeo() {
    // The NeoGeo driver in FBNeo reads input directly from
    // the GameInp array. We need to set the appropriate
    // GameInp[n].Input.nVal for each mapped input.
    //
    // For the skeleton, we store the values and the JNI bridge
    // reads them. In a full implementation, this would iterate
    // through GameInp and set values from g_inputState[].
}

// ============================================================
// JNI-exported input setters (called from Kotlin via EmulatorBridge)
// ============================================================
// These are declared in jni_bridge.cpp extern "C" block

} // namespace input
