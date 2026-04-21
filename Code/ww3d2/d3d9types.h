// Stub header for Linux builds
#pragma once

#if !defined(_WIN32)

// D3D color value (ARGB packed)
typedef DWORD D3DCOLOR;
typedef D3DCOLOR* LPD3DCOLOR;

// D3D color value struct
struct D3DCOLORVALUE {
    float r;
    float g;
    float b;
    float a;
};

#endif // !_WIN32
