package org.neogeoemu.app.ui.screens

import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import org.neogeoemu.app.emulation.EmulatorBridge

/**
 * PPSSPP-style emulation screen with:
 * - OpenGL ES video output via SurfaceView
 * - On-screen touch controls (D-pad + 4 buttons)
 * - Quick menu overlay (save/load state, settings, exit)
 */
@Composable
fun EmulationScreen(
    modifier: Modifier = Modifier,
    gameTitle: String,
    emulatorBridge: EmulatorBridge,
    onBack: () -> Unit,
    onSaveState: (Int) -> Unit,
    onLoadState: (Int) -> Unit,
    onReset: () -> Unit
) {
    var showQuickMenu by remember { mutableStateOf(false) }
    var fps by remember { mutableStateOf(0) }
    var showFps by remember { mutableStateOf(false) }

    Box(modifier = modifier.fillMaxSize()) {
        // === Video output (OpenGL ES SurfaceView) ===
        AndroidView(
            factory = { context ->
                SurfaceView(context).apply {
                    holder.addCallback(object : SurfaceHolder.Callback {
                        override fun surfaceCreated(holder: SurfaceHolder) {
                            emulatorBridge.setSurface(holder.surface)
                        }
                        override fun surfaceChanged(
                            holder: SurfaceHolder,
                            format: Int,
                            width: Int,
                            height: Int
                        ) {
                            emulatorBridge.setSurfaceSize(width, height)
                        }
                        override fun surfaceDestroyed(holder: SurfaceHolder) {
                            emulatorBridge.setSurface(null)
                        }
                    })
                }
            },
            modifier = Modifier.fillMaxSize()
        )

        // === Touch controls overlay ===
        TouchControlsOverlay(
            modifier = Modifier.fillMaxSize(),
            emulatorBridge = emulatorBridge,
            onMenuToggle = { showQuickMenu = !showQuickMenu }
        )

        // === FPS counter (top-left, PPSSPP-style) ===
        if (showFps) {
            Text(
                text = "$fps FPS",
                modifier = Modifier
                    .align(Alignment.TopStart)
                    .padding(8.dp)
                    .background(
                        Color.Black.copy(alpha = 0.5f),
                        RoundedCornerShape(4.dp)
                    )
                    .padding(horizontal = 6.dp, vertical = 2.dp),
                color = Color.Green,
                fontSize = 12.sp,
                fontFamily = androidx.compose.ui.text.font.FontFamily.Monospace
            )
        }

        // === Quick menu overlay (PPSSPP-style) ===
        if (showQuickMenu) {
            QuickMenuOverlay(
                gameTitle = gameTitle,
                onDismiss = { showQuickMenu = false },
                onSaveState = onSaveState,
                onLoadState = onLoadState,
                onReset = onReset,
                onBack = onBack
            )
        }
    }
}

/**
 * PPSSPP-style on-screen touch controls.
 * Left: D-pad, Right: 4 action buttons (A, B, C, D)
 * Top: Start, Coin/Select, Menu
 */
@Composable
fun TouchControlsOverlay(
    modifier: Modifier = Modifier,
    emulatorBridge: EmulatorBridge,
    onMenuToggle: () -> Unit
) {
    val buttonAlpha = 0.4f

    Box(modifier = modifier) {
        // === Top bar: Start, Coin, Menu ===
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(top = 8.dp, start = 8.dp, end = 8.dp),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            // Coin button
            TouchButton(
                text = "COIN",
                modifier = Modifier.alpha(buttonAlpha),
                onPress = { emulatorBridge.setInputCoin(true) },
                onRelease = { emulatorBridge.setInputCoin(false) }
            )

            // Menu button
            IconButton(onClick = onMenuToggle) {
                Icon(
                    Icons.Default.Menu,
                    contentDescription = "Menu",
                    tint = Color.White.copy(alpha = 0.6f)
                )
            }

            // Start button
            TouchButton(
                text = "START",
                modifier = Modifier.alpha(buttonAlpha),
                onPress = { emulatorBridge.setInputStart(true) },
                onRelease = { emulatorBridge.setInputStart(false) }
            )
        }

        // === Left: D-Pad ===
        Box(
            modifier = Modifier
                .align(Alignment.CenterStart)
                .padding(start = 16.dp)
        ) {
            DPad(
                modifier = Modifier.size(140.dp),
                alpha = buttonAlpha,
                onUp = { pressed -> emulatorBridge.setInputUp(pressed) },
                onDown = { pressed -> emulatorBridge.setInputDown(pressed) },
                onLeft = { pressed -> emulatorBridge.setInputLeft(pressed) },
                onRight = { pressed -> emulatorBridge.setInputRight(pressed) }
            )
        }

        // === Right: Action buttons (A, B, C, D) ===
        // NeoGeo layout: 4 buttons in arc pattern
        Box(
            modifier = Modifier
                .align(Alignment.CenterEnd)
                .padding(end = 16.dp)
                .size(160.dp)
        ) {
            // Top-right: B
            NeoButton(
                text = "B",
                modifier = Modifier
                    .align(Alignment.TopEnd)
                    .alpha(buttonAlpha),
                onPress = { emulatorBridge.setInputButton(1, true) },
                onRelease = { emulatorBridge.setInputButton(1, false) }
            )
            // Top-left: A
            NeoButton(
                text = "A",
                modifier = Modifier
                    .align(Alignment.TopStart)
                    .alpha(buttonAlpha),
                onPress = { emulatorBridge.setInputButton(0, true) },
                onRelease = { emulatorBridge.setInputButton(0, false) }
            )
            // Bottom-right: D
            NeoButton(
                text = "D",
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .alpha(buttonAlpha),
                onPress = { emulatorBridge.setInputButton(3, true) },
                onRelease = { emulatorBridge.setInputButton(3, false) }
            )
            // Bottom-left: C
            NeoButton(
                text = "C",
                modifier = Modifier
                    .align(Alignment.BottomStart)
                    .alpha(buttonAlpha),
                onPress = { emulatorBridge.setInputButton(2, true) },
                onRelease = { emulatorBridge.setInputButton(2, false) }
            )
        }
    }
}

/**
 * NeoGeo action button (A/B/C/D).
 */
@Composable
fun NeoButton(
    text: String,
    modifier: Modifier = Modifier,
    onPress: () -> Unit,
    onRelease: () -> Unit
) {
    Box(
        modifier = modifier
            .size(56.dp)
            .background(
                Color.White.copy(alpha = 0.3f),
                CircleShape
            )
            .pointerInput(Unit) {
                detectTapGestures(
                    onPress = {
                        onPress()
                        tryAwaitRelease()
                        onRelease()
                    }
                )
            },
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = text,
            color = Color.White,
            fontWeight = FontWeight.Bold,
            fontSize = 18.sp
        )
    }
}

/**
 * Generic touch button.
 */
@Composable
fun TouchButton(
    text: String,
    modifier: Modifier = Modifier,
    onPress: () -> Unit,
    onRelease: () -> Unit
) {
    Box(
        modifier = modifier
            .background(
                Color.White.copy(alpha = 0.2f),
                RoundedCornerShape(20.dp)
            )
            .padding(horizontal = 16.dp, vertical = 8.dp)
            .pointerInput(Unit) {
                detectTapGestures(
                    onPress = {
                        onPress()
                        tryAwaitRelease()
                        onRelease()
                    }
                )
            },
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = text,
            color = Color.White,
            fontWeight = FontWeight.SemiBold,
            fontSize = 12.sp
        )
    }
}

/**
 * D-Pad control.
 */
@Composable
fun DPad(
    modifier: Modifier = Modifier,
    alpha: Float = 0.4f,
    onUp: (Boolean) -> Unit,
    onDown: (Boolean) -> Unit,
    onLeft: (Boolean) -> Unit,
    onRight: (Boolean) -> Unit
) {
    Box(modifier = modifier) {
        // Up
        Box(
            modifier = Modifier
                .align(Alignment.TopCenter)
                .size(46.dp, 48.dp)
                .alpha(alpha)
                .background(Color.White.copy(alpha = 0.3f))
                .pointerInput(Unit) {
                    detectTapGestures(
                        onPress = { onUp(true); tryAwaitRelease(); onUp(false) }
                    )
                }
        )
        // Down
        Box(
            modifier = Modifier
                .align(Alignment.BottomCenter)
                .size(46.dp, 48.dp)
                .alpha(alpha)
                .background(Color.White.copy(alpha = 0.3f))
                .pointerInput(Unit) {
                    detectTapGestures(
                        onPress = { onDown(true); tryAwaitRelease(); onDown(false) }
                    )
                }
        )
        // Left
        Box(
            modifier = Modifier
                .align(Alignment.CenterStart)
                .size(48.dp, 46.dp)
                .alpha(alpha)
                .background(Color.White.copy(alpha = 0.3f))
                .pointerInput(Unit) {
                    detectTapGestures(
                        onPress = { onLeft(true); tryAwaitRelease(); onLeft(false) }
                    )
                }
        )
        // Right
        Box(
            modifier = Modifier
                .align(Alignment.CenterEnd)
                .size(48.dp, 46.dp)
                .alpha(alpha)
                .background(Color.White.copy(alpha = 0.3f))
                .pointerInput(Unit) {
                    detectTapGestures(
                        onPress = { onRight(true); tryAwaitRelease(); onRight(false) }
                    )
                }
        )
        // Center
        Box(
            modifier = Modifier
                .align(Alignment.Center)
                .size(46.dp, 46.dp)
                .alpha(alpha)
                .background(Color.White.copy(alpha = 0.15f))
        )
    }
}

/**
 * PPSSPP-style quick menu overlay.
 */
@Composable
fun QuickMenuOverlay(
    gameTitle: String,
    onDismiss: () -> Unit,
    onSaveState: (Int) -> Unit,
    onLoadState: (Int) -> Unit,
    onReset: () -> Unit,
    onBack: () -> Unit
) {
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black.copy(alpha = 0.7f))
    ) {
        Card(
            modifier = Modifier
                .align(Alignment.Center)
                .width(300.dp),
            shape = RoundedCornerShape(16.dp),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.surface
            )
        ) {
            Column(
                modifier = Modifier.padding(16.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = gameTitle,
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold,
                    textAlign = TextAlign.Center
                )
                Spacer(Modifier.height(16.dp))

                // Save state buttons
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceEvenly
                ) {
                    repeat(3) { slot ->
                        Column(horizontalAlignment = Alignment.CenterHorizontally) {
                            FilledTonalButton(
                                onClick = { onSaveState(slot) }
                            ) {
                                Icon(Icons.Default.Save, "Save $slot", Modifier.size(18.dp))
                            }
                            Text("Save ${slot + 1}", fontSize = 10.sp)
                        }
                    }
                }

                Spacer(Modifier.height(8.dp))

                // Load state buttons
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceEvenly
                ) {
                    repeat(3) { slot ->
                        Column(horizontalAlignment = Alignment.CenterHorizontally) {
                            FilledTonalButton(
                                onClick = { onLoadState(slot) }
                            ) {
                                Icon(Icons.Default.FolderOpen, "Load $slot", Modifier.size(18.dp))
                            }
                            Text("Load ${slot + 1}", fontSize = 10.sp)
                        }
                    }
                }

                Spacer(Modifier.height(12.dp))
                HorizontalDivider()
                Spacer(Modifier.height(12.dp))

                // Reset
                OutlinedButton(
                    onClick = onReset,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Icon(Icons.Default.RestartAlt, null, Modifier.size(18.dp))
                    Spacer(Modifier.width(8.dp))
                    Text("Reset")
                }

                Spacer(Modifier.height(8.dp))

                // Back to game list
                Button(
                    onClick = onBack,
                    modifier = Modifier.fillMaxWidth(),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.error
                    )
                ) {
                    Icon(Icons.Default.ExitToApp, null, Modifier.size(18.dp))
                    Spacer(Modifier.width(8.dp))
                    Text("Exit Game")
                }

                Spacer(Modifier.height(8.dp))

                // Close menu
                TextButton(onClick = onDismiss) {
                    Text("Resume Game")
                }
            }
        }
    }
}
