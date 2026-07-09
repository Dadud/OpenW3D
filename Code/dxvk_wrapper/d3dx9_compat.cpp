#include <d3dx9.h>

D3DXMATRIX *WINAPI D3DXMatrixMultiply(D3DXMATRIX *out, const D3DXMATRIX *a, const D3DXMATRIX *b)
{
    D3DXMATRIX result = {};
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            result.m[row][col] =
                a->m[row][0] * b->m[0][col] +
                a->m[row][1] * b->m[1][col] +
                a->m[row][2] * b->m[2][col] +
                a->m[row][3] * b->m[3][col];
        }
    }
    *out = result;
    return out;
}

UINT WINAPI D3DXGetFVFVertexSize(DWORD fvf)
{
    UINT size = 0;
    switch (fvf & D3DFVF_POSITION_MASK) {
    case D3DFVF_XYZ:
        size += 3 * sizeof(float);
        break;
    case D3DFVF_XYZRHW:
    case D3DFVF_XYZB1:
        size += 4 * sizeof(float);
        break;
    case D3DFVF_XYZB2:
        size += 5 * sizeof(float);
        break;
    case D3DFVF_XYZB3:
        size += 6 * sizeof(float);
        break;
    case D3DFVF_XYZB4:
        size += 7 * sizeof(float);
        break;
    case D3DFVF_XYZB5:
        size += 8 * sizeof(float);
        break;
    default:
        break;
    }

    if (fvf & D3DFVF_NORMAL) {
        size += 3 * sizeof(float);
    }
    if (fvf & D3DFVF_PSIZE) {
        size += sizeof(float);
    }
    if (fvf & D3DFVF_DIFFUSE) {
        size += sizeof(DWORD);
    }
    if (fvf & D3DFVF_SPECULAR) {
        size += sizeof(DWORD);
    }

    const UINT tex_count = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
    size += tex_count * 2 * sizeof(float);
    return size;
}

HRESULT WINAPI D3DXCreateTexture(IDirect3DDevice9 *device, UINT width, UINT height,
        UINT mip_levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture9 **texture)
{
    if (texture == nullptr) {
        return D3DERR_INVALIDCALL;
    }
    *texture = nullptr;
    if (device == nullptr) {
        return D3DERR_INVALIDCALL;
    }
    return device->CreateTexture(width, height, mip_levels, usage, format, pool, texture, nullptr);
}

HRESULT WINAPI D3DXCreateTextureFromFileExA(IDirect3DDevice9 *, const char *, UINT, UINT, UINT, DWORD,
        D3DFORMAT, D3DPOOL, DWORD, DWORD, D3DCOLOR, D3DXIMAGE_INFO *, PALETTEENTRY *, IDirect3DTexture9 **texture)
{
    if (texture != nullptr) {
        *texture = nullptr;
    }
    return D3DERR_INVALIDCALL;
}

HRESULT WINAPI D3DXLoadSurfaceFromSurface(IDirect3DSurface9 *, const PALETTEENTRY *, const RECT *,
        IDirect3DSurface9 *, const PALETTEENTRY *, const RECT *, DWORD, D3DCOLOR)
{
    return S_OK;
}

HRESULT WINAPI D3DXFilterTexture(IDirect3DBaseTexture9 *, const PALETTEENTRY *, UINT, DWORD)
{
    return S_OK;
}
