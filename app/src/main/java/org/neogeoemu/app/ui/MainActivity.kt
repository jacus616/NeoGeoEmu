package org.neogeoemu.app.ui

import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.ui.Modifier
import org.neogeoemu.app.ui.screens.GameListScreen
import org.neogeoemu.app.ui.theme.NeoGeoEmuTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            NeoGeoEmuTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    GameListScreen(
                        onGameSelected = { game ->
                            val intent = Intent(this, EmulationActivity::class.java).apply {
                                putExtra(EmulationActivity.EXTRA_GAME_PATH, game.romPath)
                                putExtra(EmulationActivity.EXTRA_GAME_TITLE, game.title)
                            }
                            startActivity(intent)
                        }
                    )
                }
            }
        }
    }
}
