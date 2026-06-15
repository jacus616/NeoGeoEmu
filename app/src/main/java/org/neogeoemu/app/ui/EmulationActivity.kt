package org.neogeoemu.app.ui

import android.os.Bundle
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.WindowManager
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.ui.Modifier
import org.neogeoemu.app.emulation.EmulatorBridge
import org.neogeoemu.app.ui.screens.EmulationScreen
import org.neogeoemu.app.ui.theme.NeoGeoEmuTheme

class EmulationActivity : ComponentActivity() {

    private lateinit var emulatorBridge: EmulatorBridge

    companion object {
        const val EXTRA_GAME_PATH = "game_path"
        const val EXTRA_GAME_TITLE = "game_title"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Keep screen on during emulation
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        enableEdgeToEdge()

        val gamePath = intent.getStringExtra(EXTRA_GAME_PATH) ?: run {
            finish()
            return
        }
        val gameTitle = intent.getStringExtra(EXTRA_GAME_TITLE) ?: "Unknown"

        emulatorBridge = EmulatorBridge()

        setContent {
            NeoGeoEmuTheme(darkTheme = true) {
                EmulationScreen(
                    modifier = Modifier.fillMaxSize(),
                    gameTitle = gameTitle,
                    emulatorBridge = emulatorBridge,
                    onBack = { finish() },
                    onSaveState = { slot -> emulatorBridge.saveState(slot) },
                    onLoadState = { slot -> emulatorBridge.loadState(slot) },
                    onReset = { emulatorBridge.reset() }
                )
            }
        }

        // Start emulation
        emulatorBridge.start(gamePath)
    }

    override fun onResume() {
        super.onResume()
        if (::emulatorBridge.isInitialized) {
            emulatorBridge.resume()
        }
    }

    override fun onPause() {
        super.onPause()
        if (::emulatorBridge.isInitialized) {
            emulatorBridge.pause()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        if (::emulatorBridge.isInitialized) {
            emulatorBridge.stop()
        }
    }

    // Gamepad input
    override fun onKeyDown(keyCode: Int, event: KeyEvent?): Boolean {
        if (::emulatorBridge.isInitialized) {
            val mapped = KeyMapper.mapKeyCode(keyCode)
            if (mapped != 0) {
                return true
            }
        }
        return super.onKeyDown(keyCode, event)
    }

    override fun onGenericMotionEvent(event: MotionEvent?): Boolean {
        if (::emulatorBridge.isInitialized && event != null) {
            return GamepadHandler.handleMotionEvent(event, emulatorBridge)
        }
        return super.onGenericMotionEvent(event)
    }
}
