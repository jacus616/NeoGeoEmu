package org.neogeoemu.app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.launch
import org.neogeoemu.app.data.GameInfo
import org.neogeoemu.app.data.GameScanner

/**
 * PPSSPP-style game list screen.
 * Shows discovered NeoGeo ROMs with metadata, similar to PPSSPP's game browser.
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun GameListScreen(
    onGameSelected: (GameInfo) -> Unit
) {
    val context = LocalContext.current
    val scope = rememberCoroutineScope()

    var games by remember { mutableStateOf<List<GameInfo>>(emptyList()) }
    var isScanning by remember { mutableStateOf(false) }
    var hasBios by remember { mutableStateOf(false) }
    var showSettings by remember { mutableStateOf(false) }
    var showAbout by remember { mutableStateOf(false) }

    // Scan on first composition
    LaunchedEffect(Unit) {
        isScanning = true
        val scanner = GameScanner(context)
        hasBios = scanner.hasBios()
        games = scanner.scanForGames()
        isScanning = false
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Icon(
                            Icons.Default.VideogameAsset,
                            contentDescription = null,
                            tint = MaterialTheme.colorScheme.primary,
                            modifier = Modifier.size(28.dp)
                        )
                        Spacer(Modifier.width(8.dp))
                        Text(
                            "NeoGeoEmu",
                            fontWeight = FontWeight.Bold
                        )
                    }
                },
                actions = {
                    IconButton(onClick = {
                        scope.launch {
                            isScanning = true
                            val scanner = GameScanner(context)
                            games = scanner.scanForGames()
                            hasBios = scanner.hasBios()
                            isScanning = false
                        }
                    }) {
                        Icon(Icons.Default.Refresh, "Rescan ROMs")
                    }
                    IconButton(onClick = { showSettings = true }) {
                        Icon(Icons.Default.Settings, "Settings")
                    }
                    IconButton(onClick = { showAbout = true }) {
                        Icon(Icons.Default.Info, "About")
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = MaterialTheme.colorScheme.surface
                )
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
        ) {
            // BIOS warning banner (PPSSPP-style)
            if (!hasBios && !isScanning) {
                BiosWarningBanner()
            }

            // Scanning indicator
            if (isScanning) {
                LinearProgressIndicator(
                    modifier = Modifier.fillMaxWidth()
                )
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(16.dp),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        "Scanning for NeoGeo ROMs...",
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }

            // Game list
            if (games.isEmpty() && !isScanning) {
                EmptyState(hasBios = hasBios)
            } else {
                LazyColumn(
                    modifier = Modifier.fillMaxSize(),
                    contentPadding = PaddingValues(horizontal = 12.dp, vertical = 8.dp),
                    verticalArrangement = Arrangement.spacedBy(6.dp)
                ) {
                    items(games, key = { it.gameId }) { game ->
                        GameListItem(
                            game = game,
                            onClick = { onGameSelected(game) }
                        )
                    }
                }
            }
        }
    }

    // Settings dialog
    if (showSettings) {
        SettingsDialog(onDismiss = { showSettings = false })
    }

    // About dialog
    if (showAbout) {
        AboutDialog(onDismiss = { showAbout = false })
    }
}

/**
 * PPSSPP-style game list item with game info.
 */
@Composable
fun GameListItem(
    game: GameInfo,
    onClick: () -> Unit
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick),
        shape = RoundedCornerShape(12.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.5f)
        ),
        elevation = CardDefaults.cardElevation(defaultElevation = 0.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Game icon placeholder (would show actual game art)
            Box(
                modifier = Modifier
                    .size(56.dp)
                    .clip(RoundedCornerShape(8.dp))
                    .background(
                        Brush.linearGradient(
                            colors = listOf(
                                MaterialTheme.colorScheme.primary.copy(alpha = 0.3f),
                                MaterialTheme.colorScheme.tertiary.copy(alpha = 0.3f)
                            )
                        )
                    ),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    Icons.Default.VideogameAsset,
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.primary,
                    modifier = Modifier.size(32.dp)
                )
            }

            Spacer(Modifier.width(12.dp))

            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = game.title,
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.SemiBold,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                Spacer(Modifier.height(2.dp))
                Text(
                    text = game.gameId,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    maxLines = 1
                )
                if (game.year != null || game.manufacturer != null) {
                    Spacer(Modifier.height(2.dp))
                    Row {
                        game.year?.let {
                            Text(
                                text = it,
                                style = MaterialTheme.typography.labelSmall,
                                color = MaterialTheme.colorScheme.outline
                            )
                        }
                        if (game.year != null && game.manufacturer != null) {
                            Text(
                                text = " • ",
                                style = MaterialTheme.typography.labelSmall,
                                color = MaterialTheme.colorScheme.outline
                            )
                        }
                        game.manufacturer?.let {
                            Text(
                                text = it,
                                style = MaterialTheme.typography.labelSmall,
                                color = MaterialTheme.colorScheme.outline
                            )
                        }
                    }
                }
            }

            Icon(
                Icons.Default.ChevronRight,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
    }
}

/**
 * BIOS warning banner (PPSSPP-style notification).
 */
@Composable
fun BiosWarningBanner() {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 12.dp, vertical = 6.dp),
        shape = RoundedCornerShape(8.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.errorContainer
        )
    ) {
        Row(
            modifier = Modifier.padding(12.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Icon(
                Icons.Default.Warning,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.error,
                modifier = Modifier.size(20.dp)
            )
            Spacer(Modifier.width(8.dp))
            Column {
                Text(
                    "NeoGeo BIOS not found",
                    style = MaterialTheme.typography.labelLarge,
                    fontWeight = FontWeight.SemiBold,
                    color = MaterialTheme.colorScheme.onErrorContainer
                )
                Text(
                    "Place neogeo.zip in your ROMs directory",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onErrorContainer.copy(alpha = 0.8f)
                )
            }
        }
    }
}

/**
 * Empty state when no games found.
 */
@Composable
fun EmptyState(hasBios: Boolean) {
    Box(
        modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            modifier = Modifier.padding(32.dp)
        ) {
            Icon(
                Icons.Default.SentimentDissatisfied,
                contentDescription = null,
                modifier = Modifier.size(64.dp),
                tint = MaterialTheme.colorScheme.outline
            )
            Spacer(Modifier.height(16.dp))
            Text(
                "No NeoGeo ROMs found",
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Spacer(Modifier.height(8.dp))
            Text(
                "Place your NeoGeo ROMs (.zip) in:\n/NeoGeo/ROMs/",
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.outline
            )
            if (!hasBios) {
                Spacer(Modifier.height(8.dp))
                Text(
                    "Don't forget neogeo.zip (BIOS)!",
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.error
                )
            }
        }
    }
}

/**
 * Settings dialog (PPSSPP-style).
 */
@Composable
fun SettingsDialog(onDismiss: () -> Unit) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Settings") },
        text = {
            Column {
                Text("Video", fontWeight = FontWeight.Bold)
                Text("• Renderer: OpenGL ES 3.0")
                Text("• Scaling: 1x (Native)")
                Text("• Filter: Nearest")
                Spacer(Modifier.height(8.dp))
                Text("Audio", fontWeight = FontWeight.Bold)
                Text("• Enabled: Yes")
                Text("• Volume: 100%")
                Spacer(Modifier.height(8.dp))
                Text("Emulation", fontWeight = FontWeight.Bold)
                Text("• Region: Japan")
                Text("• BIOS: UniBIOS")
                Text("• Speed limit: On")
            }
        },
        confirmButton = {
            TextButton(onClick = onDismiss) { Text("Close") }
        }
    )
}

/**
 * About dialog.
 */
@Composable
fun AboutDialog(onDismiss: () -> Unit) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("About NeoGeoEmu") },
        text = {
            Column {
                Text("NeoGeoEmu v0.1.0-alpha", fontWeight = FontWeight.Bold)
                Spacer(Modifier.height(8.dp))
                Text("A PPSSPP-style NeoGeo emulator for Android.")
                Spacer(Modifier.height(8.dp))
                Text("Based on FinalBurn Neo emulation core.")
                Spacer(Modifier.height(8.dp))
                Text("Licensed under GPL-2.0")
            }
        },
        confirmButton = {
            TextButton(onClick = onDismiss) { Text("Close") }
        }
    )
}
