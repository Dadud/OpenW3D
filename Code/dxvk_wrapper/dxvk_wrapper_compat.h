#pragma once

#ifdef ZeroMemory
#undef ZeroMemory
#endif

#include <windows_base.h>
#include <windows.h>

// OpenW3D @feature Provide Win32 function implementations (Sleep, GetLastError,
// OutputDebugString, file APIs, mutex/event, etc.) on non-Windows platforms.
// On Windows this is a no-op.
#if !defined(_WIN32)
#include "win32_compat.h"
#endif

#define LF_FACESIZE 32
#define EXTERN_C extern "C"

typedef struct GLYPHMETRICSFLOAT GLYPHMETRICSFLOAT;
typedef GUID *LPGUID;
typedef GUID CLSID;
#define DECLSPEC_UUID(X)
typedef struct IStream IStream;
typedef struct HFONT HFONT;
typedef struct TEXTMETRICA TEXTMETRICA;
typedef struct TEXTMETRICW TEXTMETRICW;

typedef double DOUBLE;

#define STDAPI HRESULT WINAPI

#define HIWORD(W) (((W) & 0xffff) >> 8)
#define LOWORD(W) ((W) & 0xff)
