package org.neogeoemu.app.data

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.*
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

val Context.dataStore: DataStore<Preferences> by preferencesDataStore(name = "settings")

/**
 * Emulator settings management (PPSSPP-style settings architecture).
 */
object Prefs {
    // ROM paths
    val ROM_DIRECTORIES = stringPreferencesKey("rom_directories")

    // Video
    val VIDEO_RENDERER = stringPreferencesKey("video_renderer") // "opengl", "vulkan"
    val VIDEO_SCALING = intPreferencesKey("video_scaling") // 0=1x, 1=2x, 2=4x
    val VIDEO_FILTER = stringPreferencesKey("video_filter") // "nearest", "linear", "crt"
    val VIDEO_INTEGER_SCALING = booleanPreferencesKey("video_integer_scaling")
    val VIDEO_STRETCH_TO_FIT = booleanPreferencesKey("video_stretch_to_fit")
    val VIDEO_SHOW_FPS = booleanPreferencesKey("video_show_fps")
    val VIDEO_HARDWARE_ACCEL = booleanPreferencesKey("video_hardware_accel")

    // Audio
    val AUDIO_ENABLED = booleanPreferencesKey("audio_enabled")
    val AUDIO_VOLUME = intPreferencesKey("audio_volume") // 0-100
    val AUDIO_LATENCY = intPreferencesKey("audio_latency") // frames of latency

    // Input
    val INPUT_TOUCH_ENABLED = booleanPreferencesKey("input_touch_enabled")
    val INPUT_TOUCH_OPACITY = intPreferencesKey("input_touch_opacity") // 0-100
    val INPUT_TOUCH_SCALE = intPreferencesKey("input_touch_scale") // 0-200
            val INPUT_GAMEPAD_ENABLED = booleanPreferencesKey("input_gamepad_enabled")
    val INPUT_VIBRATION = booleanPreferencesKey("input_vibration")

    // Emulation
    val EMU_REGION = stringPreferencesKey("emu_region") // "japan", "usa", "asia", "europe"
    val EMU_BIOS = stringPreferencesKey("emu_bios") // "unibios", "mvs", "aes"
    val EMU_FRAMESKIP = intPreferencesKey("emu_frameskip") // 0=auto, 1-5
    val EMU_SPEED_LIMIT = booleanPreferencesKey("emu_speed_limit")
    val EMU_CLOCK_SPEED = intPreferencesKey("emu_clock_speed") // percentage

    // Savestates
    val SAVESTATE_AUTO_SAVE = booleanPreferencesKey("savestate_auto_save")
    val SAVESTATE_AUTO_LOAD = booleanPreferencesKey("savestate_auto_load")
    val SAVE_STATE_DIR = stringPreferencesKey("savestate_dir")

    // Default values
    object Defaults {
        const val VIDEO_RENDERER = "opengl"
        const val VIDEO_SCALING = 0
        const val VIDEO_FILTER = "nearest"
        const val VIDEO_INTEGER_SCALING = true
        const val VIDEO_STRETCH_TO_FIT = false
        const val VIDEO_SHOW_FPS = false
        const val VIDEO_HARDWARE_ACCEL = true

        const val AUDIO_ENABLED = true
        const val AUDIO_VOLUME = 100
        const val AUDIO_LATENCY = 2

        const val INPUT_TOUCH_ENABLED = true
        const val INPUT_TOUCH_OPACITY = 60
        const val INPUT_TOUCH_SCALE = 100
        const val INPUT_GAMEPAD_ENABLED = true
        const val INPUT_VIBRATION = true

        const val EMU_REGION = "japan"
        const val EMU_BIOS = "unibios"
        const val EMU_FRAMESKIP = 0
        const val EMU_SPEED_LIMIT = true
        const val EMU_CLOCK_SPEED = 100

        const val SAVESTATE_AUTO_SAVE = false
        const val SAVESTATE_AUTO_LOAD = false
    }
}
