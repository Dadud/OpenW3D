#pragma once

#include "dxvk_wrapper_compat.h"

#if defined(__GNUC__) || defined(__clang__)
#include_next <d3d9.h>
#else
#include <d3d9.h>
#endif
