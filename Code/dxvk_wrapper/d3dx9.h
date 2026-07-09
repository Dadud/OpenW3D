#pragma once

#include "dxvk_wrapper_compat.h"

#if defined(__GNUC__) || defined(__clang__)
#include_next <d3dx9.h>
#else
#include <d3dx9.h>
#endif
