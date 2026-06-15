package org.neogeoemu.app.emulation

import android.view.KeyEvent
import android.view.MotionEvent

/**
 * Maps Android key codes to NeoGeo inputs.
 */
object KeyMapper {
    fun mapKeyCode(keyCode: Int): Int {
        return when (keyCode) {
            KeyEvent.KEYCODE_DPAD_UP -> 0      // Up
            KeyEvent.KEYCODE_DPAD_DOWN -> 1    // Down
            KeyEvent.KEYCODE_DPAD_LEFT -> 2    // Left
            KeyEvent.KEYCODE_DPAD_RIGHT -> 3   // Right
            KeyEvent.KEYCODE_BUTTON_A,
            KeyEvent.KEYCODE_BUTTON_X -> 4     // NeoGeo A
            KeyEvent.KEYCODE_BUTTON_Y -> 5     // NeoGeo B
            KeyEvent.KEYCODE_BUTTON_B -> 6     // NeoGeo C
            KeyEvent.KEYCODE_BUTTON_L1 -> 7    // NeoGeo D
            KeyEvent.KEYCODE_BUTTON_START,
            KeyEvent.KEYCODE_BUTTON_R2 -> 8    // Start
            KeyEvent.KEYCODE_BUTTON_SELECT -> 9 // Coin
            else -> -1 // Unmapped
        }
    }
}

/**
 * Handles gamepad/motion input events.
 */
object GamepadHandler {
    fun handleMotionEvent(event: MotionEvent, bridge: EmulatorBridge): Boolean {
        // Handle analog stick as D-pad
        val x = event.getAxisValue(MotionEvent.AXIS_X)
        val y = event.getAxisValue(MotionEvent.AXIS_Y)

        val deadzone = 0.25f
        bridge.setInputLeft(x < -deadzone)
        bridge.setInputRight(x > deadzone)
        bridge.setInputUp(y < -deadzone)
        bridge.setInputDown(y > deadzone)

        // Handle right analog stick / hat
        val hatX = event.getAxisValue(MotionEvent.AXIS_HAT_X)
        val hatY = event.getAxisValue(MotionEvent.AXIS_HAT_Y)

        if (hatX != 0f || hatY != 0f) {
            bridge.setInputLeft(hatX == -1f)
            bridge.setInputRight(hatX == 1f)
            bridge.setInputUp(hatY == -1f)
            bridge.setInputDown(hatY == 1f)
        }

        return true
    }
}
