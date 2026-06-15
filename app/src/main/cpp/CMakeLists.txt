cmake_minimum_required(VERSION 3.22.1)
project(neogeo-core C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# FBNeo source root
# ============================================================
set(FBNEO_ROOT ${CMAKE_SOURCE_DIR}/../../../../fbneo-core/src)

# ============================================================
# Burn Engine Core
# ============================================================
set(BURN_SOURCES
    ${FBNEO_ROOT}/burn/burn.cpp
    ${FBNEO_ROOT}/burn/burn_bitmap.cpp
    ${FBNEO_ROOT}/burn/burn_gun.cpp
    ${FBNEO_ROOT}/burn/burn_led.cpp
    ${FBNEO_ROOT}/burn/burn_memory.cpp
    ${FBNEO_ROOT}/burn/burn_pal.cpp
    ${FBNEO_ROOT}/burn/burn_sha1.cpp
    ${FBNEO_ROOT}/burn/burn_shift.cpp
    ${FBNEO_ROOT}/burn/burn_sound.cpp
    ${FBNEO_ROOT}/burn/burn_sound_c.cpp
    ${FBNEO_ROOT}/burn/cheat.cpp
    ${FBNEO_ROOT}/burn/debug_track.cpp
    ${FBNEO_ROOT}/burn/hiscore.cpp
    ${FBNEO_ROOT}/burn/load.cpp
    ${FBNEO_ROOT}/burn/tilemap_generic.cpp
    ${FBNEO_ROOT}/burn/tiles_generic.cpp
    ${FBNEO_ROOT}/burn/timer.cpp
    ${FBNEO_ROOT}/burn/vector.cpp
)

# ============================================================
# NeoGeo Driver
# ============================================================
set(NEOGEO_SOURCES
    ${FBNEO_ROOT}/burn/drv/d_parent.cpp
    ${FBNEO_ROOT}/burn/drv/neogeo/d_neogeo.cpp
    ${FBNEO_ROOT}/burn/drv/neogeo/neo_decrypt.cpp
    ${FBNEO_ROOT}/burn/drv/neogeo/neogeo.cpp
    ${FBNEO_ROOT}/burn/drv/neogeo/neo_palette.cpp
    ${FBNEO_ROOT}/burn/drv/neogeo/neo_run.cpp
    ${FBNEO_ROOT}/burn/drv/neogeo/neo_sprite.cpp
    ${FBNEO_ROOT}/burn/drv/neogeo/neo_text.cpp
    ${FBNEO_ROOT}/burn/drv/neogeo/neo_upd4990a.cpp
)

# ============================================================
# Sound Chips (NeoGeo needs YM2610 + AY8910)
# ============================================================
set(SND_SOURCES
    # Core sound (always needed)
    ${FBNEO_ROOT}/burn/snd/fm.c
    ${FBNEO_ROOT}/burn/snd/fmopl.c
    ${FBNEO_ROOT}/burn/snd/ymdeltat.c
    ${FBNEO_ROOT}/burn/snd/dac.cpp
    ${FBNEO_ROOT}/burn/snd/samples.cpp
    ${FBNEO_ROOT}/burn/snd/flt_rc.cpp
    # NeoGeo sound chips
    ${FBNEO_ROOT}/burn/snd/ay8910.c
    ${FBNEO_ROOT}/burn/snd/burn_ym2610.cpp
    ${FBNEO_ROOT}/burn/snd/msm5205.cpp
    ${FBNEO_ROOT}/burn/snd/msm6295.cpp
    # Additional sound chips (needed by FBNeo's sound system)
    ${FBNEO_ROOT}/burn/snd/ym2151.c
    ${FBNEO_ROOT}/burn/snd/ym2413.c
    ${FBNEO_ROOT}/burn/snd/ym2612.c
    ${FBNEO_ROOT}/burn/snd/ymf262.cpp
    ${FBNEO_ROOT}/burn/snd/ymf271.cpp
    ${FBNEO_ROOT}/burn/snd/ymf278b.c
    ${FBNEO_ROOT}/burn/snd/ymz280b.cpp
    ${FBNEO_ROOT}/burn/snd/upd7759.cpp
    ${FBNEO_ROOT}/burn/snd/burn_md2612.cpp
    ${FBNEO_ROOT}/burn/snd/burn_y8950.cpp
    ${FBNEO_ROOT}/burn/snd/burn_ym2151.cpp
    ${FBNEO_ROOT}/burn/snd/burn_ym2203.cpp
    ${FBNEO_ROOT}/burn/snd/burn_ym2413.cpp
    ${FBNEO_ROOT}/burn/snd/burn_ym2608.cpp
    ${FBNEO_ROOT}/burn/snd/burn_ym2612.cpp
    ${FBNEO_ROOT}/burn/snd/burn_ym3526.cpp
    ${FBNEO_ROOT}/burn/snd/burn_ym3812.cpp
    ${FBNEO_ROOT}/burn/snd/burn_ymf262.cpp
    ${FBNEO_ROOT}/burn/snd/burn_ymf271.cpp
    ${FBNEO_ROOT}/burn/snd/burn_ymf278b.cpp
    ${FBNEO_ROOT}/burn/snd/segapcm.cpp
    ${FBNEO_ROOT}/burn/snd/ics2115.cpp
    ${FBNEO_ROOT}/burn/snd/rf5c68.cpp
    ${FBNEO_ROOT}/burn/snd/c140.cpp
    ${FBNEO_ROOT}/burn/snd/c352.cpp
    ${FBNEO_ROOT}/burn/snd/c6280.cpp
    ${FBNEO_ROOT}/burn/snd/es5506.cpp
    ${FBNEO_ROOT}/burn/snd/es8712.cpp
    ${FBNEO_ROOT}/burn/snd/k051649.cpp
    ${FBNEO_ROOT}/burn/snd/k053260.cpp
    ${FBNEO_ROOT}/burn/snd/k054539.cpp
    ${FBNEO_ROOT}/burn/snd/multipcm.cpp
    ${FBNEO_ROOT}/burn/snd/namco_snd.cpp
    ${FBNEO_ROOT}/burn/snd/nes_apu.cpp
    ${FBNEO_ROOT}/burn/snd/pokey.cpp
    ${FBNEO_ROOT}/burn/snd/saa1099.cpp
    ${FBNEO_ROOT}/burn/snd/sn76477.cpp
    ${FBNEO_ROOT}/burn/snd/sn76496.cpp
    ${FBNEO_ROOT}/burn/snd/sp0250.cpp
    ${FBNEO_ROOT}/burn/snd/sp0256.cpp
    ${FBNEO_ROOT}/burn/snd/t6w28.cpp
    ${FBNEO_ROOT}/burn/snd/tms36xx.cpp
    ${FBNEO_ROOT}/burn/snd/tms5110.cpp
    ${FBNEO_ROOT}/burn/snd/tms5220.cpp
    ${FBNEO_ROOT}/burn/snd/vlm5030.cpp
    ${FBNEO_ROOT}/burn/snd/x1010.cpp
    ${FBNEO_ROOT}/burn/snd/ymz770.cpp
    ${FBNEO_ROOT}/burn/snd/hc55516.cpp
    ${FBNEO_ROOT}/burn/snd/i5000.cpp
    ${FBNEO_ROOT}/burn/snd/iremga20.cpp
    ${FBNEO_ROOT}/burn/snd/k005289.cpp
    ${FBNEO_ROOT}/burn/snd/k007232.cpp
    ${FBNEO_ROOT}/burn/snd/llander.cpp
    ${FBNEO_ROOT}/burn/snd/mpeg_audio.cpp
    ${FBNEO_ROOT}/burn/snd/phoenixsound.cpp
    ${FBNEO_ROOT}/burn/snd/pleiadssound.cpp
    ${FBNEO_ROOT}/burn/snd/redbaron.cpp
    ${FBNEO_ROOT}/burn/snd/s14001a.cpp
    ${FBNEO_ROOT}/burn/snd/snk6502_sound.cpp
    ${FBNEO_ROOT}/burn/snd/wiping.cpp
    ${FBNEO_ROOT}/burn/snd/gaelco.cpp
    ${FBNEO_ROOT}/burn/snd/asteroids.cpp
    ${FBNEO_ROOT}/burn/snd/astrocde_snd.cpp
    ${FBNEO_ROOT}/burn/snd/beep.cpp
    ${FBNEO_ROOT}/burn/snd/bzone.cpp
    ${FBNEO_ROOT}/burn/snd/digitalk.cpp
    ${FBNEO_ROOT}/burn/snd/exidy440_snd.cpp
    ${FBNEO_ROOT}/burn/snd/flower.cpp
    ${FBNEO_ROOT}/burn/devices/mathbox.cpp
    ${FBNEO_ROOT}/burn/devices/mb87078.cpp
    ${FBNEO_ROOT}/burn/devices/mermaid.cpp
    ${FBNEO_ROOT}/burn/devices/midcsd.cpp
    ${FBNEO_ROOT}/burn/devices/midcvsd.cpp
    ${FBNEO_ROOT}/burn/devices/midsat.cpp
    ${FBNEO_ROOT}/burn/devices/midsg.cpp
    ${FBNEO_ROOT}/burn/devices/midssio.cpp
    ${FBNEO_ROOT}/burn/devices/midtcs.cpp
    ${FBNEO_ROOT}/burn/devices/nb1414m4.cpp
    ${FBNEO_ROOT}/burn/devices/nb1414m4_8bit.cpp
    ${FBNEO_ROOT}/burn/devices/nmk004.cpp
    ${FBNEO_ROOT}/burn/devices/nmk112.cpp
    ${FBNEO_ROOT}/burn/devices/pandora.cpp
    ${FBNEO_ROOT}/burn/devices/pic8259.cpp
    ${FBNEO_ROOT}/burn/devices/pit8253.cpp
    ${FBNEO_ROOT}/burn/devices/poly.cpp
    ${FBNEO_ROOT}/burn/devices/qs1000.cpp
    ${FBNEO_ROOT}/burn/devices/resnet.cpp
    ${FBNEO_ROOT}/burn/devices/rtc9701.cpp
    ${FBNEO_ROOT}/burn/devices/seibucop.cpp
    ${FBNEO_ROOT}/burn/devices/seibusnd.cpp
    ${FBNEO_ROOT}/burn/devices/serflash.cpp
    ${FBNEO_ROOT}/burn/devices/sknsspr.cpp
    ${FBNEO_ROOT}/burn/devices/slapstic.cpp
    ${FBNEO_ROOT}/burn/devices/st0020.cpp
    ${FBNEO_ROOT}/burn/devices/t5182.cpp
    ${FBNEO_ROOT}/burn/devices/timekpr.cpp
    ${FBNEO_ROOT}/burn/devices/tlc34076.cpp
    ${FBNEO_ROOT}/burn/devices/tms34061.cpp
    ${FBNEO_ROOT}/burn/devices/tms9928a.cpp
    ${FBNEO_ROOT}/burn/devices/v3021.cpp
    ${FBNEO_ROOT}/burn/devices/vdc.cpp
    ${FBNEO_ROOT}/burn/devices/watchdog.cpp
    ${FBNEO_ROOT}/burn/devices/x2212.cpp
    ${FBNEO_ROOT}/burn/devices/ad59mc07.cpp
    ${FBNEO_ROOT}/burn/devices/alpha8201.cpp
    ${FBNEO_ROOT}/burn/devices/atariic.cpp
    ${FBNEO_ROOT}/burn/devices/atarijsa.cpp
    ${FBNEO_ROOT}/burn/devices/atarimo.cpp
    ${FBNEO_ROOT}/burn/devices/atarirle.cpp
    ${FBNEO_ROOT}/burn/devices/atarivad.cpp
    ${FBNEO_ROOT}/burn/devices/avgdvg.cpp
    ${FBNEO_ROOT}/burn/devices/bsmt2000.cpp
    ${FBNEO_ROOT}/burn/devices/c169.cpp
    ${FBNEO_ROOT}/burn/devices/decobsmt.cpp
    ${FBNEO_ROOT}/burn/devices/ds2404.cpp
    ${FBNEO_ROOT}/burn/devices/dtimer.cpp
    ${FBNEO_ROOT}/burn/devices/earom.cpp
    ${FBNEO_ROOT}/burn/devices/eeprom.cpp
    ${FBNEO_ROOT}/burn/devices/epic12.cpp
    ${FBNEO_ROOT}/burn/devices/gaelco_crypt.cpp
    ${FBNEO_ROOT}/burn/devices/i2ceeprom.cpp
    ${FBNEO_ROOT}/burn/devices/i4x00.cpp
    ${FBNEO_ROOT}/burn/devices/i8155.cpp
    ${FBNEO_ROOT}/burn/devices/intelfsh.cpp
    ${FBNEO_ROOT}/burn/devices/joyprocess.cpp
    ${FBNEO_ROOT}/burn/devices/k1ge.cpp
    ${FBNEO_ROOT}/burn/devices/kaneko_hit.cpp
    ${FBNEO_ROOT}/burn/devices/kaneko_tmap.cpp
    ${FBNEO_ROOT}/burn/devices/namco_c45.cpp
    ${FBNEO_ROOT}/burn/devices/namcoio.cpp
    ${FBNEO_ROOT}/burn/devices/6821pia.cpp
    ${FBNEO_ROOT}/burn/devices/6840ptm.cpp
    ${FBNEO_ROOT}/burn/devices/74259.cpp
    ${FBNEO_ROOT}/burn/devices/8255ppi.cpp
    ${FBNEO_ROOT}/burn/devices/8257dma.cpp
)

# ============================================================
# CPU Cores (M68K + Z80 for NeoGeo)
# ============================================================
set(CPU_SOURCES
    # M68K (Motorola 68000 — NeoGeo main CPU)
    ${FBNEO_ROOT}/cpu/m68k/m68kcpu.c
    ${FBNEO_ROOT}/cpu/m68k/m68kdasm.c
    ${FBNEO_ROOT}/cpu/m68k/m68kfpu.c
    ${FBNEO_ROOT}/cpu/m68k/m68k_in.c
    # Note: m68kmake.c is a code generator, not compiled into the target

    # Z80 (sound CPU)
    ${FBNEO_ROOT}/cpu/z80/z80.cpp
    ${FBNEO_ROOT}/cpu/z80/z80ctc.cpp
    ${FBNEO_ROOT}/cpu/z80/z80daisy.cpp
    ${FBNEO_ROOT}/cpu/z80/z80pio.cpp
)

# ============================================================
# Burner (frontend helpers — state, zip, input)
# ============================================================
set(BURNER_SOURCES
    ${FBNEO_ROOT}/burner/state.cpp
    ${FBNEO_ROOT}/burner/statec.cpp
    ${FBNEO_ROOT}/burner/zipfn.cpp
    ${FBNEO_ROOT}/burner/ioapi.c
    ${FBNEO_ROOT}/burner/unzip.c
    ${FBNEO_ROOT}/burner/nvram.cpp
    ${FBNEO_ROOT}/burner/inputbuf.cpp
    ${FBNEO_ROOT}/burner/misc.cpp
    ${FBNEO_ROOT}/burner/dat.cpp
    ${FBNEO_ROOT}/burner/conc.cpp
    ${FBNEO_ROOT}/burner/cong.cpp
    ${FBNEO_ROOT}/burner/gamc.cpp
    ${FBNEO_ROOT}/burner/gami.cpp
    ${FBNEO_ROOT}/burner/image.cpp
    ${FBNEO_ROOT}/burner/sshot.cpp
    ${FBNEO_ROOT}/burner/un7z.cpp
    ${FBNEO_ROOT}/burner/dynhuff.cpp
)

# ============================================================
# Interface layer
# ============================================================
set(INTF_SOURCES
    ${FBNEO_ROOT}/intf/interface.cpp
)

# ============================================================
# Android JNI + platform layer
# ============================================================
set(ANDROID_SOURCES
    ${CMAKE_SOURCE_DIR}/android/jni_bridge.cpp
    ${CMAKE_SOURCE_DIR}/android/android_video.cpp
    ${CMAKE_SOURCE_DIR}/android/android_audio.cpp
    ${CMAKE_SOURCE_DIR}/android/android_input.cpp
    ${CMAKE_SOURCE_DIR}/android/android_romloader.cpp
    ${CMAKE_SOURCE_DIR}/android/android_state.cpp
)

# ============================================================
# Create the native library
# ============================================================
add_library(neogeo-core SHARED
    ${BURN_SOURCES}
    ${NEOGEO_SOURCES}
    ${SND_SOURCES}
    ${CPU_SOURCES}
    ${BURNER_SOURCES}
    ${INTF_SOURCES}
    ${ANDROID_SOURCES}
)

# ============================================================
# Include directories
# ============================================================
target_include_directories(neogeo-core PRIVATE
    ${CMAKE_SOURCE_DIR}/android
    ${FBNEO_ROOT}
    ${FBNEO_ROOT}/burn
    ${FBNEO_ROOT}/burn/drv
    ${FBNEO_ROOT}/burn/drv/neogeo
    ${FBNEO_ROOT}/burn/snd
    ${FBNEO_ROOT}/burn/devices
    ${FBNEO_ROOT}/cpu/m68k
    ${FBNEO_ROOT}/cpu/z80
    ${FBNEO_ROOT}/burner
    ${FBNEO_ROOT}/intf
    ${FBNEO_ROOT}/intf/cd
    ${FBNEO_ROOT}/dep/libs/lua
)

# ============================================================
# Compile definitions
# ============================================================
target_compile_definitions(neogeo-core PRIVATE
    -DANDROID
    -DFB_NEOGEO
    -DFASTCALL=
    -D__cdecl=
    -D__fastcall=
    -DBUILD_YM2610
    -DUSE_SPEEDCAP=0
    -DINCLUDE_RUNAHEAD_SUPPORT
    -DLSB_FIRST=1
    -DBURNER
)

# ============================================================
# Compiler flags
# ============================================================
target_compile_options(neogeo-core PRIVATE
    -O2
    -fexceptions
    -ffunction-sections
    -fdata-sections
    -fvisibility=hidden
    -fno-strict-aliasing
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-sign-compare
    -Wno-parentheses
)

# ============================================================
# Link libraries
# ============================================================
find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(glesv3-lib GLESv3)
find_library(openSLES-lib OpenSLES)
find_library(z-lib z)

target_link_libraries(neogeo-core
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${glesv3-lib}
    ${openSLES-lib}
    ${z-lib}
)

# Strip unused code in release
set_target_properties(neogeo-core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
