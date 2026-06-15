/**
 * globals.h — Common definitions for the Android FBNeo port.
 * Resolves FBNeo's platform-specific macros for Android.
 */

#ifndef NEOGEO_ANDROID_GLOBALS_H
#define NEOGEO_ANDROID_GLOBALS_H

#include <android/log.h>
#include <cstdint>
#include <cstring>
#include <cstdlib>

#define LOG_TAG "NeoGeoEmu-Core"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

// ============================================================
// FBNeo type mappings (burn.h expects these)
// ============================================================
typedef unsigned char   UINT8;
typedef signed char     INT8;
typedef unsigned short  UINT16;
typedef signed short    INT16;
typedef unsigned int    UINT32;
typedef signed int      INT32;

// 64-bit types
#if defined(_MSC_VER)
typedef unsigned __int64 UINT64;
typedef signed __int64   INT64;
#else
typedef unsigned long long UINT64;
typedef signed long long   INT64;
#endif

// ============================================================
// FBNeo platform macros
// ============================================================
#ifndef FASTCALL
#define FASTCALL
#endif

#ifndef __cdecl
#define __cdecl
#endif

#ifndef __fastcall
#define __fastcall
#endif

// TCHAR mapping — FBNeo uses TCHAR for text.
// On Android we use char (narrow).
#ifndef _T
#define _T(x) x
#endif

#ifndef _tcsicmp
#define _tcsicmp strcmp
#endif

#ifndef _tcscpy
#define _tcscpy strcpy
#endif

#ifndef _tcslen
#define _tcslen strlen
#endif

#ifndef _stprintf
#define _stprintf sprintf
#endif

#ifndef _tfopen
#define _tfopen fopen
#endif

#ifndef _istspace
#define _istspace isspace
#endif

// MAX_PATH
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// TCHAR
#ifndef TCHAR
#define TCHAR char
#endif

// NeoGeo screen dimensions
#define NEOGEO_WIDTH   320
#define NEOGEO_HEIGHT  224
#define NEOGEO_BPP     2     // 16-bit RGB565

// Audio config
#define NEOGEO_SAMPLE_RATE  44100
#define NEOGEO_AUDIO_BUFFER_SIZE  4096

#endif // NEOGEO_ANDROID_GLOBALS_H
