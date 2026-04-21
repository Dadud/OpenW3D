// Stub header for Linux builds - D3DX not available on Linux
#pragma once

#include <d3d9.h>

#if defined(_WIN32)
#include <d3dx9.h>
#else

// D3DXLoadSurfaceFromSurface is already defined in d3d9.h
// No additional stub needed here on Linux

#endif // !_WIN32
