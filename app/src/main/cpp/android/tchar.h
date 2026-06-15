/**
 * tchar.h — TCHAR compatibility layer for Android/Non-Windows platforms.
 * FBNeo expects this header for TCHAR, _T(), _tcscpy, etc.
 */

#ifndef NEOGEO_TCHAR_H
#define NEOGEO_TCHAR_H

#include <cstring>
#include <cstdio>

// TCHAR is char on non-Windows
#ifndef _TCHAR
typedef char TCHAR;
#endif

// _T() macro for string literals
#ifndef _T
#define _T(x) x
#endif

// _tcs functions map to standard C functions
#ifndef _tcscpy
#define _tcscpy strcpy
#endif

#ifndef _tcslen
#define _tcslen strlen
#endif

#ifndef _tcsicmp
#define _tcscmp strcmp
#endif

#ifndef _tcsnicmp
#define _tcsncpy strncpy
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

#ifndef _istdigit
#define _istdigit isdigit
#endif

#ifndef _totupper
#define _totupper toupper
#endif

#ifndef _totlower
#define _totlower tolower
#endif

#ifndef _ttoi
#define _ttoi atoi
#endif

#ifndef _tcstol
#define _tcstol strtol
#endif

#ifndef _tcstoul
#define _tcstoul strtoul
#endif

#ifndef _tcsstr
#define _tcsstr strstr
#endif

#ifndef _tcschr
#define _tcschr strchr
#endif

#ifndef _tcsrchr
#define _tcsrchr strrchr
#endif

#ifndef _tcsncpy
#define _tcsncpy strncpy
#endif

#ifndef _tcscat
#define _tcscat strcat
#endif

#ifndef _tcsncmp
#define _tcsncmp strncmp
#endif

#ifndef _tcspbrk
#define _tcspbrk strpbrk
#endif

#ifndef _tcsspn
#define _tcsspn strspn
#endif

#ifndef _tcstok
#define _tcstok strtok
#endif

#ifndef _vsntprintf
#define _vsntprintf vsnprintf
#endif

#ifndef _sntprintf
#define _sntprintf snprintf
#endif

// UNICODE/_UNICODE not defined on Android
#ifndef UNICODE
// Already using char-based functions above
#endif

#endif // NEOGEO_TCHAR_H
