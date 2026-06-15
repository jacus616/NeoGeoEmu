package org.neogeoemu.app.emulation

import android.view.Surface

/**
 * JNI bridge to the FBNeo emulation core.
 * Manages emulation lifecycle: start, pause, resume, stop.
 */
class EmulatorBridge {

    private var nativeHandle: Long = 0
    private var isRunning = false

    companion object {
        init {
            System.loadLibrary("neogeo-core")
        }

        // Native methods
        @JvmStatic private external fun nativeCreate(): Long
        @JvmStatic private external fun nativeDestroy(handle: Long)
        @JvmStatic private external fun nativeLoadRom(handle: Long, romPath: String): Boolean
        @JvmStatic private external fun nativeRunFrame(handle: Long)
        @JvmStatic private external fun nativeSetSurface(handle: Long, surface: Surface?)
        @JvmStatic private external fun nativeSetSurfaceSize(handle: Long, width: Int, height: Int)
        @JvmStatic private external fun nativeSetInput(handle: Long, inputId: Int, pressed: Boolean)
        @JvmStatic private external fun nativeSaveState(handle: Long, slot: Int): Boolean
        @JvmStatic private external fun nativeLoadState(handle: Long, slot: Int): Boolean
        @JvmStatic private external fun nativeReset(handle: Long)
        @JvmStatic private external fun nativePause(handle: Long)
        @JvmStatic private external fun nativeResume(handle: Long)
        @JvmStatic private external fun nativeStop(handle: Long)
    }

    /**
     * Start emulation with the given ROM.
     */
    fun start(romPath: String): Boolean {
        if (nativeHandle == 0L) {
            nativeHandle = nativeCreate()
        }
        val success = nativeLoadRom(nativeHandle, romPath)
        if (success) {
            isRunning = true
        }
        return success
    }

    fun pause() {
        if (nativeHandle != 0L) nativePause(nativeHandle)
    }

    fun resume() {
        if (nativeHandle != 0L) nativeResume(nativeHandle)
    }

    fun stop() {
        if (nativeHandle != 0L) {
            nativeStop(nativeHandle)
            nativeDestroy(nativeHandle)
            nativeHandle = 0L
        }
        isRunning = false
    }

    fun reset() {
        if (nativeHandle != 0L) nativeReset(nativeHandle)
    }

    fun setSurface(surface: Surface?) {
        if (nativeHandle != 0L) nativeSetSurface(nativeHandle, surface)
    }

    fun setSurfaceSize(width: Int, height: Int) {
        if (nativeHandle != 0L) nativeSetSurfaceSize(nativeHandle, width, height)
    }

    fun saveState(slot: Int): Boolean {
        return if (nativeHandle != 0L) nativeSaveState(nativeHandle, slot) else false
    }

    fun loadState(slot: Int): Boolean {
        return if (nativeHandle != 0L) nativeLoadState(nativeHandle, slot) else false
    }

    // === Input mapping ===
    // NeoGeo input IDs match FBNeo's input system

    fun setInputUp(pressed: Boolean)    = setInput(0, pressed)
    fun setInputDown(pressed: Boolean)  = setInput(1, pressed)
    fun setInputLeft(pressed: Boolean)  = setInput(2, pressed)
    fun setInputRight(pressed: Boolean) = setInput(3, pressed)
    fun setInputButton(button: Int, pressed: Boolean) = setInput(4 + button, pressed) // A=4, B=5, C=6, D=7
    fun setInputStart(pressed: Boolean) = setInput(8, pressed)
    fun setInputCoin(pressed: Boolean)  = setInput(9, pressed)

    private fun setInput(inputId: Int, pressed: Boolean) {
        if (nativeHandle != 0L) nativeSetInput(nativeHandle, inputId, pressed)
    }
}
