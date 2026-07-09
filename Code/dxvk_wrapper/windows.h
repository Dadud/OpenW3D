#pragma once

#include "dxvk_wrapper_compat.h"

#if defined(__GNUC__) || defined(__clang__)
#include_next <windows.h>
#else
#include <windows.h>
#endif
