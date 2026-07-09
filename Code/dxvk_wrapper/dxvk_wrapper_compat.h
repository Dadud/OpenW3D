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

#ifndef DECLARE_INTERFACE_IID_
#define DECLARE_INTERFACE_IID_(x, y, iid) DECLARE_INTERFACE_(x, y)
#endif

#define HIWORD(W) (((W) & 0xffff) >> 8)
#define LOWORD(W) ((W) & 0xff)

#ifndef GWL_STYLE
#define GWL_STYLE (-16)
#endif
#ifndef WS_CHILD
#define WS_CHILD 0x40000000L
#endif
#ifndef WS_SYSMENU
#define WS_SYSMENU 0x00080000L
#endif
#ifndef WS_CAPTION
#define WS_CAPTION 0x00C00000L
#endif
#ifndef WS_MINIMIZEBOX
#define WS_MINIMIZEBOX 0x00020000L
#endif
#ifndef WS_CLIPCHILDREN
#define WS_CLIPCHILDREN 0x02000000L
#endif
#ifndef WS_POPUP
#define WS_POPUP 0x80000000L
#endif
#ifndef HWND_TOPMOST
#define HWND_TOPMOST ((HWND)-1)
#endif
#ifndef SM_CXSCREEN
#define SM_CXSCREEN 0
#endif
#ifndef SM_CYSCREEN
#define SM_CYSCREEN 1
#endif
#ifndef SWP_SHOWWINDOW
#define SWP_SHOWWINDOW 0x0040
#endif
#ifndef SWP_NOCOPYBITS
#define SWP_NOCOPYBITS 0x0100
#endif

inline LONG GetWindowLong(HWND, int) { return 0; }
inline LONG SetWindowLong(HWND, int, LONG) { return 0; }
inline BOOL SetWindowPos(HWND, HWND, int, int, int, int, UINT) { return TRUE; }
inline int GetSystemMetrics(int metric) { return metric == SM_CXSCREEN ? 640 : 480; }
inline BOOL GetWindowRect(HWND, RECT *rect) { if (rect) { rect->left = 0; rect->top = 0; rect->right = 640; rect->bottom = 480; } return TRUE; }
inline HWND GetDesktopWindow() { return nullptr; }
inline HDC GetDC(HWND) { return nullptr; }
inline int ReleaseDC(HWND, HDC) { return 1; }
inline BOOL SetDeviceGammaRamp(HDC, void *) { return TRUE; }

#ifndef TIMERR_NOERROR
#define TIMERR_NOERROR 0
#endif
typedef UINT MMRESULT;
inline DWORD timeGetTime() { return GetTickCount(); }
inline MMRESULT timeBeginPeriod(UINT) { return TIMERR_NOERROR; }
inline MMRESULT timeEndPeriod(UINT) { return TIMERR_NOERROR; }
#define OPENW3D_DXVK_WRAPPER_HAS_WINMM_TIME 1

#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif

inline BOOL SetCurrentDirectoryA(const char *) { return TRUE; }
