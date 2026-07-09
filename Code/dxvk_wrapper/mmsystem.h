#pragma once

#include "dxvk_wrapper_compat.h"
#include <cstdint>

#ifndef TIMERR_NOERROR
#define TIMERR_NOERROR 0
#endif

#ifndef TIME_PERIODIC
#define TIME_PERIODIC 1
#endif

#ifndef TIME_CALLBACK_FUNCTION
#define TIME_CALLBACK_FUNCTION 0x0000
#endif

typedef UINT MMRESULT;
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef DWORD_PTR
typedef uintptr_t DWORD_PTR;
#endif
typedef void (CALLBACK *LPTIMECALLBACK)(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);

#ifndef OPENW3D_DXVK_WRAPPER_HAS_WINMM_TIME
inline DWORD timeGetTime()
{
    return GetTickCount();
}

inline MMRESULT timeBeginPeriod(UINT)
{
    return TIMERR_NOERROR;
}

inline MMRESULT timeEndPeriod(UINT)
{
    return TIMERR_NOERROR;
}
#endif
