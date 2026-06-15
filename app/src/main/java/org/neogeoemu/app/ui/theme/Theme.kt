package org.neogeoemu.app.ui.theme

import android.app.Activity
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalView
import androidx.core.view.WindowCompat

// PPSSPP-inspired color scheme — dark with NeoGeo orange accents
private val DarkColorScheme = darkColorScheme(
    primary = Color(0xFFFF6B35),        // NeoGeo orange
    onPrimary = Color.White,
    primaryContainer = Color(0xFF8B3A1A),
    onPrimaryContainer = Color(0xFFFFDBCA),
    secondary = Color(0xFFFFD700),      // Gold
    onSecondary = Color.Black,
    secondaryContainer = Color(0xFF6B5B00),
    onSecondaryContainer = Color(0xFFFFF0B0),
    tertiary = Color(0xFF4FC3F7),       // Light blue
    onTertiary = Color.Black,
    background = Color(0xFF1A1A2E),     // Dark navy
    onBackground = Color(0xFFE0E0E0),
    surface = Color(0xFF16213E),        // Slightly lighter navy
    onSurface = Color(0xFFE0E0E0),
    surfaceVariant = Color(0xFF0F3460),
    onSurfaceVariant = Color(0xFFB0B0B0),
    error = Color(0xFFCF6679),
    onError = Color.Black,
    outline = Color(0xFF4A4A6A)
)

@Composable
fun NeoGeoEmuTheme(
    darkTheme: Boolean = true, // Always dark (PPSSPP style)
    content: @Composable () -> Unit
) {
    val colorScheme = DarkColorScheme

    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as Activity).window
            window.statusBarColor = colorScheme.background.toArgb()
            window.navigationBarColor = colorScheme.background.toArgb()
            WindowCompat.getInsetsController(window, view).isAppearanceLightStatusBars = false
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography(),
        content = content
    )
}
