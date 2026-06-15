# 🎮 NeoGeoEmu

[![Build APK](https://github.com/KongoPrint3D/NeoGeoEmu/actions/workflows/build.yml/badge.svg)](https://github.com/KongoPrint3D/NeoGeoEmu/actions/workflows/build.yml)
[![License](https://img.shields.io/badge/license-GPL--2.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Android-green.svg)](https://android.com)

A PPSSPP-style NeoGeo emulator for Android, powered by [FinalBurn Neo](https://github.com/finalburnneo/FBNeo).

![NeoGeoEmu](docs/screenshot.png)

## ✨ Features

- 🎮 **PPSSPP-inspired UI** — Clean Material 3 design with dark theme
- 🕹️ **Touch controls** — D-pad + 4 action buttons (A/B/C/D) + Start/Coin
- 💾 **Save states** — 3 slots with quick save/load menu
- 🎵 **Audio** — YM2610 + ADPCM via OpenSL ES
- 📺 **Video** — OpenGL ES 3.0 rendering with nearest/linear filtering
- 🎮 **Gamepad support** — Android gamepad/joystick input
- 🔍 **ROM scanner** — Auto-discovers NeoGeo ROMs on device

## 📋 Requirements

- Android 8.0+ (API 26+)
- ARM64 or ARMv7 device
- NeoGeo BIOS (`neogeo.zip`) in your ROMs directory
- NeoGeo ROMs in `.zip` format (non-merged FBNeo format)

## 🚀 Download

Get the latest APK from the [Releases](https://github.com/KongoPrint3D/NeoGeoEmu/releases) page.

## 🛠️ Building

### Prerequisites

- Android Studio Hedgehog (2023.1.1) or newer
- Android SDK 34
- NDK r27+
- CMake 3.22+
- JDK 17+

### Steps

```bash
# Clone with submodules
git clone --recurse-submodules https://github.com/KongoPrint3D/NeoGeoEmu.git
cd NeoGeoEmu

# Build debug APK
./gradlew assembleDebug

# APK location: app/build/outputs/apk/debug/app-debug.apk
```

### ROM Setup

Place your NeoGeo ROMs in:
```
/NeoGeo/ROMs/
```

Required files:
- `neogeo.zip` — NeoGeo BIOS (required)
- Game ROMs as `.zip` files (e.g. `kof97.zip`, `mslug3.zip`)

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────┐
│              Kotlin UI (Compose)                  │
│  GameList │ EmulationScreen │ Settings           │
├─────────────────────────────────────────────────┤
│              JNI Bridge (C++)                     │
├─────────────────────────────────────────────────┤
│           FBNeo Emulation Core                    │
│  M68K CPU │ Z80 CPU │ YM2610 │ Video │ Input    │
└─────────────────────────────────────────────────┘
```

## 📁 Project Structure

```
NeoGeoEmu/
├── app/
│   ├── src/main/
│   │   ├── cpp/           # Native code (JNI + FBNeo bridge)
│   │   │   ├── android/   # Android platform layer
│   │   │   │   ├── jni_bridge.cpp
│   │   │   │   ├── android_video.cpp
│   │   │   │   ├── android_audio.cpp
│   │   │   │   ├── android_input.cpp
│   │   │   │   ├── android_romloader.cpp
│   │   │   │   ├── android_state.cpp
│   │   │   │   └── globals.h
│   │   │   └── CMakeLists.txt
│   │   ├── java/          # Kotlin source
│   │   │   └── org/neogeoemu/app/
│   │   │       ├── ui/    # Screens, theme
│   │   │       ├── emulation/  # JNI bridge
│   │   │       └── data/  # Game scanner, prefs
│   │   └── res/           # Resources
│   └── build.gradle.kts
├── fbneo-core/            # FBNeo submodule
├── .github/workflows/     # CI/CD
└── README.md
```

## 📜 License

- NeoGeoEmu: [GPL-2.0](LICENSE)
- FBNeo core: [GPL-2.0](fbneo-core/LICENSE.md)

## 🙏 Credits

- [FinalBurn Neo](https://github.com/finalburnneo/FBNeo) — Emulation core
- [PPSSPP](https://github.com/hrydgard/ppsspp) — UI inspiration
