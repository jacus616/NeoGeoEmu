package org.neogeoemu.app.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp

/**
 * Full settings screen with categories (PPSSPP-style).
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(
    onBack: () -> Unit
) {
    var selectedCategory by remember { mutableStateOf(0) }
    val categories = listOf("Video", "Audio", "Input", "Emulation", "Savestates")

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Settings", fontWeight = FontWeight.Bold) },
                navigationIcon = {
                    IconButton(onClick = onBack) {
                        Icon(Icons.Default.ArrowBack, "Back")
                    }
                }
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
        ) {
            // Category tabs
            ScrollableTabRow(
                selectedTabIndex = selectedCategory,
                edgePadding = 8.dp
            ) {
                categories.forEachIndexed { index, title ->
                    Tab(
                        selected = selectedCategory == index,
                        onClick = { selectedCategory = index },
                        text = { Text(title) }
                    )
                }
            }

            // Settings content
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .verticalScroll(rememberScrollState())
                    .padding(16.dp)
            ) {
                when (selectedCategory) {
                    0 -> VideoSettings()
                    1 -> AudioSettings()
                    2 -> InputSettings()
                    3 -> EmulationSettings()
                    4 -> SavestateSettings()
                }
            }
        }
    }
}

@Composable
fun VideoSettings() {
    SettingsCategory(title = "Video") {
        SettingsDropdown(
            title = "Renderer",
            options = listOf("OpenGL ES 3.0", "Vulkan"),
            selected = 0
        )
        SettingsDropdown(
            title = "Resolution Scale",
            options = listOf("1x (Native 320x224)", "2x", "3x", "4x"),
            selected = 0
        )
        SettingsDropdown(
            title = "Filter",
            options = listOf("Nearest (Pixel-perfect)", "Linear", "CRT Shader"),
            selected = 0
        )
        SettingsToggle("Integer Scaling", true)
        SettingsToggle("Stretch to Fit Screen", false)
        SettingsToggle("Show FPS", false)
        SettingsToggle("Hardware Acceleration", true)
    }
}

@Composable
fun AudioSettings() {
    SettingsCategory(title = "Audio") {
        SettingsToggle("Enable Audio", true)
        SettingsSlider("Volume", 0f, 100f, 100f)
        SettingsDropdown(
            title = "Latency",
            options = listOf("Low (1 frame)", "Medium (2 frames)", "High (4 frames)"),
            selected = 1
        )
    }
}

@Composable
fun InputSettings() {
    SettingsCategory(title = "Input") {
        SettingsToggle("Touch Controls", true)
        SettingsSlider("Touch Opacity", 0f, 100f, 60f)
        SettingsSlider("Touch Scale", 50f, 200f, 100f)
        SettingsToggle("Gamepad Support", true)
        SettingsToggle("Vibration", true)
        HorizontalDivider(modifier = Modifier.padding(vertical = 8.dp))
        Text("Button Mapping", fontWeight = FontWeight.SemiBold)
        Text(
            "Configure gamepad button mapping in the next update.",
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.outline
        )
    }
}

@Composable
fun EmulationSettings() {
    SettingsCategory(title = "Emulation") {
        SettingsDropdown(
            title = "Region",
            options = listOf("Japan", "USA", "Asia", "Europe"),
            selected = 0
        )
        SettingsDropdown(
            title = "BIOS",
            options = listOf("UniBIOS 4.0", "MVS (Arcade)", "AES (Home)"),
            selected = 0
        )
        SettingsDropdown(
            title = "Frameskip",
            options = listOf("Auto", "Off", "1", "2", "3", "4", "5"),
            selected = 0
        )
        SettingsToggle("Speed Limit (60 FPS)", true)
        SettingsSlider("Clock Speed", 50f, 200f, 100f)
    }
}

@Composable
fun SavestateSettings() {
    SettingsCategory(title = "Save States") {
        SettingsToggle("Auto-save on Exit", false)
        SettingsToggle("Auto-load on Start", false)
        SettingsDropdown(
            title = "Save Directory",
            options = listOf("Default", "Custom..."),
            selected = 0
        )
    }
}

// === Reusable settings components ===

@Composable
fun SettingsCategory(
    title: String,
    content: @Composable ColumnScope.() -> Unit
) {
    Text(
        text = title,
        style = MaterialTheme.typography.titleLarge,
        fontWeight = FontWeight.Bold,
        modifier = Modifier.padding(bottom = 16.dp)
    )
    Column(
        verticalArrangement = Arrangement.spacedBy(12.dp),
        content = content
    )
}

@Composable
fun SettingsToggle(title: String, defaultValue: Boolean) {
    var checked by remember { mutableStateOf(defaultValue) }
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(title, style = MaterialTheme.typography.bodyLarge)
        Switch(checked = checked, onCheckedChange = { checked = it })
    }
}

@Composable
fun SettingsDropdown(
    title: String,
    options: List<String>,
    selected: Int
) {
    var expanded by remember { mutableStateOf(false) }
    var selectedIndex by remember { mutableStateOf(selected) }

    Column {
        Text(title, style = MaterialTheme.typography.bodyLarge)
        Spacer(Modifier.height(4.dp))
        ExposedDropdownMenuBox(
            expanded = expanded,
            onExpandedChange = { expanded = it }
        ) {
            OutlinedTextField(
                value = options[selectedIndex],
                onValueChange = {},
                readOnly = true,
                trailingIcon = { ExposedDropdownMenuDefaults.TrailingIcon(expanded) },
                modifier = Modifier
                    .fillMaxWidth()
                    .menuAnchor()
            )
            ExposedDropdownMenu(
                expanded = expanded,
                onDismissRequest = { expanded = false }
            ) {
                options.forEachIndexed { index, option ->
                    DropdownMenuItem(
                        text = { Text(option) },
                        onClick = {
                            selectedIndex = index
                            expanded = false
                        }
                    )
                }
            }
        }
    }
}

@Composable
fun SettingsSlider(
    title: String,
    min: Float,
    max: Float,
    defaultValue: Float
) {
    var value by remember { mutableStateOf(defaultValue) }
    Column {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(title, style = MaterialTheme.typography.bodyLarge)
            Text(
                "${value.toInt()}%",
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.primary
            )
        }
        Slider(
            value = value,
            onValueChange = { value = it },
            valueRange = min..max
        )
    }
}
