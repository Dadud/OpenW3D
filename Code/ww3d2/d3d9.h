#pragma once

// Minimal DirectX 9 stub header for Unix/Linux builds
// This provides basic type definitions without full COM interface implementations

#if !defined(_WIN32)
#ifndef _D3D9_H_
#define _D3D9_H_

#include <cstdint>
#include <cstddef>

// Base types - use bittype.h's definitions if available
// On Unix/Linux, bittype.h (which provides DWORD, ULONG, BYTE, etc.) should be
// included before this header via the include chain.
// If DWORD/ULONG/etc aren't defined yet, define them here.
#ifndef DWORD
typedef uint32_t DWORD;
#endif
#ifndef BYTE
typedef uint8_t BYTE;
#endif
#ifndef WORD
typedef uint16_t WORD;
#endif
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef UINT
typedef uint32_t UINT;
typedef int32_t INT;
#endif
#ifndef LONG
typedef int32_t LONG;
#endif
#ifndef ULONG
typedef uint32_t ULONG;
#endif
#ifndef HRESULT
typedef int32_t HRESULT;
#endif
#ifndef PLONG
typedef int32_t* PLONG;
#endif
#ifndef PULONG
typedef uint32_t* PULONG;
#endif
#ifndef PDWORD
typedef uint32_t* PDWORD;
#endif
#ifndef PUINT
typedef uint32_t* PUINT;
#endif
#ifndef LPVOID
typedef void* LPVOID;
#endif
#ifndef LPCVOID
typedef const void* LPCVOID;
#endif
#ifndef PVOID
typedef void* PVOID;
#endif
#ifndef HANDLE
typedef void* HANDLE;
#endif
#ifndef LONG_PTR
typedef intptr_t LONG_PTR;
#endif
#ifndef ULONG_PTR
typedef uintptr_t ULONG_PTR;
#endif

// D3D-specific types
typedef DWORD D3DCOLOR;
typedef DWORD D3DFORMAT;
typedef float FLOAT;

// STDMETHODCALLTYPE
#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif

// CONST
#ifndef CONST
#define CONST const
#endif

// Windows handles
typedef void* HBITMAP;
typedef void* HDC;
typedef void* HWND;
typedef void* HMONITOR;
typedef void* HMODULE;
typedef void* HINSTANCE;
typedef void* HANDLE;
typedef void* HKEY;
typedef void* HACCEL;
typedef void* HGLOBAL;
typedef void* HRSRC;
typedef void* HGDIOBJ;
typedef void* HFONT;

// GUID
struct GUID {
    DWORD Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
};
typedef GUID IID;
typedef GUID CLSID;
typedef const IID* REFIID;
typedef const CLSID* REFCLSID;

// D3D enums - must be defined before interface methods use them
enum D3DPRIMITIVETYPE { D3DPT_POINTLIST = 1, D3DPT_LINELIST = 2, D3DPT_LINESTRIP = 3, D3DPT_TRIANGLELIST = 4, D3DPT_TRIANGLESTRIP = 5, D3DPT_TRIANGLEFAN = 6 };
enum D3DSAMPLERSTATETYPE { D3DSAMP_ADDRESSU = 0, D3DSAMP_ADDRESSV = 1, D3DSAMP_ADDRESSW = 2, D3DSAMP_BORDERCOLOR = 3, D3DSAMP_MAGFILTER = 4, D3DSAMP_MINFILTER = 5, D3DSAMP_MIPFILTER = 6, D3DSAMP_MIPMAPLODBIAS = 7, D3DSAMP_MAXMIPLEVEL = 8, D3DSAMP_MAXANISOTROPY = 9, D3DSAMP_SRGBTEXTURE = 10, D3DSAMP_ELEMENTINDEX = 11, D3DSAMP_DMAPOFFSET = 12 };
enum D3DTEXTURESTAGESTATETYPE { D3DTSS_COLOROP = 1, D3DTSS_COLORARG1 = 2, D3DTSS_COLORARG2 = 3, D3DTSS_ALPHAOP = 4, D3DTSS_ALPHAARG1 = 5, D3DTSS_ALPHAARG2 = 6, D3DTSS_BUMPENVMAT00 = 7, D3DTSS_BUMPENVMAT01 = 8, D3DTSS_BUMPENVMAT10 = 9, D3DTSS_BUMPENVMAT11 = 10, D3DTSS_BUMPENVLSCALE = 11, D3DTSS_BUMPENVLOFFSET = 12, D3DTSS_MIPMAPLODBIAS = 13, D3DTSS_TEXTURETRANSFORMFLAGS = 24, D3DTSS_COLORARG0 = 32, D3DTSS_ALPHAARG0 = 33, D3DTSS_RESULTARG = 38, D3DTSS_TCI_CAMERASPACENORMAL = 0x00010000, D3DTSS_TCI_CAMERASPACEPOSITION = 0x00020000, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR = 0x00030000, D3DTSS_TCI_SPHEREMAP = 0x00040000, D3DTSS_TCI_PASSTHRU = 0, D3DTSS_TEXCOORDINDEX = 26, D3DTSS_CONSTANT = 34 };

// Missing stencil ops
#define D3DSTENCILOP_KEEP 1
#define D3DSTENCILOP_ZERO 2
#define D3DSTENCILOP_REPLACE 3
#define D3DSTENCILOP_INCRSAT 4
#define D3DSTENCILOP_DECRSAT 5
#define D3DSTENCILOP_INVERT 6
#define D3DSTENCILOP_INCR 7
#define D3DSTENCILOP_DECR 8

// Material color sources
#define D3DMCS_MATERIAL 0
#define D3DMCS_COLOR1 1
#define D3DMCS_COLOR2 2

// Vertex blend flags
#define D3DVBF_DISABLE 0
#define D3DVBF_0WEIGHTS 1
#define D3DVBF_1WEIGHTS 2
#define D3DVBF_2WEIGHTS 3
#define D3DVBF_3WEIGHTS 4
#define D3DVBF_TWEENING 7

// Patch edge styles
#define D3DPATCHEDGE_DISCRETE 0
#define D3DPATCHEDGE_CONTINUOUS 1

// Debug monitor tokens
#define D3DDMT_ENABLE 0
#define D3DDMT_DISABLE 1

// Blend operations
#define D3DBLENDOP_ADD 1
#define D3DBLENDOP_SUBTRACT 2
#define D3DBLENDOP_REVSUBTRACT 3
#define D3DBLENDOP_MIN 4
#define D3DBLENDOP_MAX 5

// Texture address modes
#define D3DTADDRESS_WRAP 1
#define D3DTADDRESS_MIRROR 2
#define D3DTADDRESS_CLAMP 3
#define D3DTADDRESS_BORDER 4
#define D3DTADDRESS_MIRRORONCE 5

// Texture transform flags
#define D3DTTFF_DISABLE 0
#define D3DTTFF_COUNT1 1
#define D3DTTFF_COUNT2 2
#define D3DTTFF_COUNT3 3
#define D3DTTFF_COUNT4 4
#define D3DTTFF_PROJECTED 256

// Z buffer types
#define D3DZB_FALSE 0
#define D3DZB_TRUE 1
#define D3DZB_USEW 2

// Shade modes
#define D3DSHADE_FLAT 1
#define D3DSHADE_GOURAUD 2
#define D3DSHADE_PHONG 3

// Additional blend modes (now defined via enum D3DBLEND below)

// Fog modes
#define D3DFOG_NONE 0
#define D3DFOG_EXP 1
#define D3DFOG_EXP2 2
#define D3DFOG_LINEAR 3
enum D3DRENDERSTATETYPE { D3DRS_ZENABLE = 1, D3DRS_FILLMODE = 2, D3DRS_SHADEMODE = 3, D3DRS_ZWRITEENABLE = 4, D3DRS_ALPHATESTENABLE = 5, D3DRS_SRCBLEND = 6, D3DRS_DESTBLEND = 7, D3DRS_ZFUNC = 8, D3DRS_ALPHAREF = 9, D3DRS_ALPHAFUNC = 10, D3DRS_DITHERENABLE = 11, D3DRS_STENCILZFAIL = 12, D3DRS_STENCILPASS = 13, D3DRS_STENCILFUNC = 14, D3DRS_STENCILREF = 15, D3DRS_STENCILMASK = 16, D3DRS_STENCILWRITEMASK = 17, D3DRS_TEXTUREFACTOR = 18, D3DRS_SCISSORTESTENABLE = 19, D3DRS_CULLMODE = 22, D3DRS_ZBIAS = 60, D3DRS_FOGSTART = 39, D3DRS_FOGEND = 40, D3DRS_FOGENABLE = 43, D3DRS_FOGCOLOR = 44, D3DRS_COLORVERTEX = 45, D3DRS_BLENDOP = 47, D3DRS_ALPHABLENDENABLE = 65, D3DRS_DEBUGMONITORTOKEN = 46, D3DRS_COLORWRITEENABLE = 200, D3DRS_POINTSCALE_A = 204, D3DRS_POINTSCALE_B = 205, D3DRS_POINTSCALE_C = 206, D3DRS_POINTSIZE_MAX = 207, D3DRS_TWEENFACTOR = 208, D3DRS_STENCILFAIL = 213, D3DRS_WRAP0 = 128, D3DRS_WRAP1 = 129, D3DRS_WRAP2 = 130, D3DRS_WRAP3 = 131, D3DRS_WRAP4 = 132, D3DRS_WRAP5 = 133, D3DRS_WRAP6 = 134, D3DRS_WRAP7 = 135, D3DRS_WRAP8 = 136, D3DRS_WRAP9 = 137, D3DRS_WRAP10 = 138, D3DRS_WRAP11 = 139, D3DRS_WRAP12 = 252, D3DRS_WRAP13 = 253, D3DRS_WRAP14 = 254, D3DRS_WRAP15 = 255, D3DRS_DIFFUSEMATERIALSOURCE = 145, D3DRS_SPECULARMATERIALSOURCE = 146, D3DRS_AMBIENTMATERIALSOURCE = 147, D3DRS_EMISSIVEMATERIALSOURCE = 148, D3DRS_VERTEXBLEND = 151, D3DRS_PATCHEDGESTYLE = 162, D3DRS_SLOPESCALEDEPTHBIAS = 195, D3DRS_DEPTHBIAS = 196, D3DRS_AMBIENT = 139, D3DRS_CLIPPLANEENABLE = 152, D3DRS_MULTISAMPLEMASK = 255, D3DRS_LASTPIXEL = 41, D3DRS_SPECULARENABLE = 42, D3DRS_STENCILENABLE = 52, D3DRS_RANGEFOGENABLE = 48, D3DRS_CLIPPING = 34, D3DRS_LIGHTING = 37, D3DRS_LOCALVIEWER = 51, D3DRS_NORMALIZENORMALS = 54, D3DRS_POINTSPRITEENABLE = 182, D3DRS_POINTSCALEENABLE = 183, D3DRS_MULTISAMPLEANTIALIAS = 161, D3DRS_INDEXEDVERTEXBLENDENABLE = 153, D3DRS_FOGTABLEMODE = 53, D3DRS_FOGVERTEXMODE = 55, D3DRS_FOGDENSITY = 56, D3DRS_POINTSIZE = 154, D3DRS_POINTSIZE_MIN = 165, D3DRS_SEPARATEALPHABLENDENABLE = 270, D3DRS_SRCBLENDALPHA = 271, D3DRS_DESTBLENDALPHA = 272, D3DRS_BLENDOPALPHA = 273, D3DRS_CCW_STENCILPASS = 259, D3DRS_CCW_STENCILFAIL = 257, D3DRS_CCW_STENCILZFAIL = 258, D3DRS_CCW_STENCILFUNC = 263, D3DRS_COLORWRITEENABLE1 = 201, D3DRS_COLORWRITEENABLE2 = 202, D3DRS_COLORWRITEENABLE3 = 203, D3DRS_BLENDFACTOR = 302, D3DRS_SRGBWRITEENABLE = 284, D3DRS_NORMALDEGREE = 180, D3DRS_ANTIALIASEDLINEENABLE = 283, D3DRS_MINTESSELLATIONLEVEL = 266, D3DRS_MAXTESSELLATIONLEVEL = 267, D3DRS_ADAPTIVETESS_X = 268, D3DRS_ADAPTIVETESS_Y = 269, D3DRS_ADAPTIVETESS_Z = 280, D3DRS_ADAPTIVETESS_W = 281, D3DRS_ENABLEADAPTIVETESSELLATION = 282, D3DRS_TWOSIDEDSTENCILMODE = 265 };

// Wrap flags
#define D3DWRAP_U 0x00000001L
#define D3DWRAP_V 0x00000002L
#define D3DWRAP_W 0x00000004L

// PALETTEENTRY struct
struct PALETTEENTRY {
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
};

// D3DXIMAGE_INFO struct
struct D3DXIMAGE_INFO {
    UINT Width;
    UINT Height;
    UINT Depth;
    UINT MipLevels;
    D3DFORMAT Format;
    DWORD ResourceType;
    DWORD ImageFileFormat;
};

// Color write enable flags
#define D3DCOLORWRITEENABLE_RED 1
#define D3DCOLORWRITEENABLE_GREEN 2
#define D3DCOLORWRITEENABLE_BLUE 4
#define D3DCOLORWRITEENABLE_ALPHA 8

// Back buffer types
#define D3DBACKBUFFER_TYPE_MONO 0

// Present parameters
#define D3DSWAPEFFECT_FLIP 3
#define D3DPRESENT_INTERVAL_DEFAULT 0x00000000L

// Texture address type
typedef int D3DTEXTUREADDRESS;

// Texture filter type
typedef int D3DTEXTUREFILTERTYPE;
enum D3DTRANSFORMSTATETYPE { D3DTS_WORLD = 256, D3DTS_VIEW = 257, D3DTS_PROJECTION = 258, D3DTS_TEXTURE0 = 272, D3DTS_TEXTURE1 = 273, D3DTS_TEXTURE2 = 274, D3DTS_TEXTURE3 = 275 };
enum D3DQUERYTYPE { D3DQUERYTYPE_EVENT = 0, D3DQUERYTYPE_OCCLUSION = 1, D3DQUERYTYPE_TIMESTAMP = 2, D3DQUERYTYPE_TIMESTAMPDISJOINT = 3, D3DQUERYTYPE_PREDICATION = 5 };
enum D3DTEXTUREOP { D3DTOP_DISABLE = 1, D3DTOP_SELECTARG1 = 2, D3DTOP_SELECTARG2 = 3, D3DTOP_MODULATE = 4, D3DTOP_MODULATE2X = 5, D3DTOP_MODULATE4X = 6, D3DTOP_ADD = 7, D3DTOP_ADDSIGNED = 8, D3DTOP_ADDSIGNED2X = 9, D3DTOP_SUBTRACT = 10, D3DTOP_ADDSMOOTH = 11, D3DTOP_BLENDDIFFUSEALPHA = 12, D3DTOP_BLENDTEXTUREALPHA = 13, D3DTOP_BLENDFACTORALPHA = 14, D3DTOP_BLENDTEXTUREALPHAPM = 15, D3DTOP_BLENDCURRENTALPHA = 16, D3DTOP_PREMODULATE = 17, D3DTOP_MODULATEALPHA_ADDCOLOR = 18, D3DTOP_MODULATECOLOR_ADDALPHA = 19, D3DTOP_MODULATEINVALPHA_ADDCOLOR = 20, D3DTOP_MODULATEINVCOLOR_ADDALPHA = 21, D3DTOP_BUMPENVMAP = 22, D3DTOP_BUMPENVMAPLUMINANCE = 23, D3DTOP_DOTPRODUCT3 = 25, D3DTOP_MULTIPLYADD = 26, D3DTOP_LERP = 27 };

// Texture arg constants
#define D3DTA_SELECTMASK 0
#define D3DTA_DIFFUSE 1
#define D3DTA_CURRENT 2
#define D3DTA_TEXTURE 3
#define D3DTA_TFACTOR 4
#define D3DTA_SPECULAR 5
#define D3DTA_TEMP 6
#define D3DTA_COMPLEMENT 0x00000010
#define D3DTA_ALPHAREPLICATE 0x00000020

// Texture filter types
#define D3DTEXF_NONE 0
#define D3DTEXF_POINT 1
#define D3DTEXF_LINEAR 2
#define D3DTEXF_ANISOTROPIC 3
#define D3DTEXF_PYRAMIDALQUAD 4
#define D3DTEXF_GAUSSIANQUAD 5

enum D3DMULTISAMPLE_TYPE { D3DMULTISAMPLE_NONE = 0, D3DMULTISAMPLE_2_SAMPLES = 2, D3DMULTISAMPLE_3_SAMPLES = 3, D3DMULTISAMPLE_4_SAMPLES = 4, D3DMULTISAMPLE_5_SAMPLES = 5, D3DMULTISAMPLE_6_SAMPLES = 6, D3DMULTISAMPLE_7_SAMPLES = 7, D3DMULTISAMPLE_8_SAMPLES = 8 };
enum D3DLIGHTTYPE { D3DLIGHT_POINT = 1, D3DLIGHT_SPOT = 2, D3DLIGHT_DIRECTIONAL = 3, D3DLIGHT_FORCE_DWORD = 0x7fffffff };
enum D3DPOOL { D3DPOOL_DEFAULT = 0, D3DPOOL_MANAGED = 1, D3DPOOL_SYSTEMMEM = 2, D3DPOOL_SCRATCH = 3 };

// Simple structs
struct POINT { int x; int y; };
struct SIZE { int cx; int cy; };

// String types
#ifndef LPCSTR
typedef const char* LPCSTR;
#endif

// Forward declarations for interface types
struct IUnknown;
struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DBaseTexture9;
struct IDirect3DVertexBuffer9;
struct IDirect3DIndexBuffer9;
struct IDirect3DTexture9;
struct IDirect3DCubeTexture9;
struct IDirect3DVolumeTexture9;
struct IDirect3DSurface9;
struct IDirect3DSwapChain9;
struct IDirect3DQuery9;
struct IDirect3DStateBlock9;
struct IDirect3DVertexShader9;
struct IDirect3DPixelShader9;
struct IDirect3DVertexDeclaration9;
struct ID3DXFont;

// Forward declarations for struct types
struct D3DADAPTER_IDENTIFIER9;
struct D3DDISPLAYMODE;
struct D3DCAPS9;
struct D3DPRESENT_PARAMETERS_;
struct D3DDEVICE_CREATION_PARAMETERS;
struct D3DRASTER_STATUS;
struct GAMMARAMP;
struct D3DRECT;
struct D3DMATRIX;
struct D3DVIEWPORT9;
struct D3DMATERIAL9;
struct D3DLIGHT9;
struct D3DVECTOR;
struct RECT;
struct D3DRECTPATCH_INFO;
struct D3DTRIPATCH_INFO;
struct D3DCLIPSTATUS9;
struct D3DSURFACE_DESC;
struct D3DVERTEXELEMENT9;
struct RGNDATA;

// Include d3d9types.h for D3DCOLORVALUE
#include "d3d9types.h"

// IUnknown
struct IUnknown {
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) = 0;
    virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
    virtual ULONG STDMETHODCALLTYPE Release() = 0;
};

// IDirect3D9
struct IDirect3D9 : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void* pInitializeFunction) = 0;
    virtual UINT STDMETHODCALLTYPE GetAdapterCount() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(UINT Adapter, DWORD Flags, struct D3DADAPTER_IDENTIFIER9* pIdentifier) = 0;
    virtual UINT STDMETHODCALLTYPE GetAdapterModeCount(UINT Adapter, DWORD Format) = 0;
    virtual HRESULT STDMETHODCALLTYPE EnumAdapterModes(UINT Adapter, DWORD Format, UINT Mode, struct D3DDISPLAYMODE* pMode) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode(UINT Adapter, struct D3DDISPLAYMODE* pMode) = 0;
    virtual HRESULT STDMETHODCALLTYPE CheckDeviceType(UINT Adapter, DWORD DevType, DWORD DisplayFormat, DWORD BackBufferFormat, BOOL Windowed) = 0;
    virtual HRESULT STDMETHODCALLTYPE CheckDeviceFormat(UINT Adapter, DWORD DeviceType, DWORD AdapterFormat, DWORD Usage, DWORD RType, DWORD CheckFormat) = 0;
    virtual HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(UINT Adapter, DWORD DeviceType, DWORD SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType) = 0;
    virtual HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(UINT Adapter, DWORD DeviceType, DWORD AdapterFormat, DWORD TargetFormat, DWORD DepthFormat) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDeviceCaps(UINT Adapter, DWORD DeviceType, struct D3DCAPS9* pCaps) = 0;
    virtual HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(UINT Adapter) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateDevice(UINT Adapter, DWORD DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, struct D3DPRESENT_PARAMETERS_* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) = 0;
};

// IDirect3DDevice9
struct IDirect3DDevice9 : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE TestCooperativeLevel() = 0;
    virtual UINT STDMETHODCALLTYPE GetAvailableTextureMem() = 0;
    virtual HRESULT STDMETHODCALLTYPE EvictManagedResources() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDirect3D(IDirect3D9** ppD3D9) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDeviceCaps(struct D3DCAPS9* pCaps) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDisplayMode(struct D3DDISPLAYMODE* pMode) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDisplayMode(UINT iSwapChain, struct D3DDISPLAYMODE* pMode) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCreationParameters(struct D3DDEVICE_CREATION_PARAMETERS *pParameters) = 0;
    virtual void STDMETHODCALLTYPE SetCursorPosition(int X, int Y, DWORD Flags) = 0;
    virtual BOOL STDMETHODCALLTYPE SetCursorProperties(int X, int Y, IDirect3DSurface9* pCursorBitmap) = 0;
    virtual void STDMETHODCALLTYPE GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, DWORD Type, IDirect3DSurface9** ppBackBuffer) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetRasterStatus(struct D3DRASTER_STATUS* pRasterStatus) = 0;
    virtual void STDMETHODCALLTYPE SetGammaRamp(int iSwapChain, DWORD Flags, CONST struct GAMMARAMP* pRamp) = 0;
    virtual void STDMETHODCALLTYPE GetGammaRamp(int iSwapChain, struct GAMMARAMP* pRamp) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateTexture(int Width, int Height, int Levels, DWORD Usage, D3DFORMAT Format, DWORD Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateVolumeTexture(int Width, int Height, int Depth, int Levels, DWORD Usage, D3DFORMAT Format, DWORD Pool, struct IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateCubeTexture(int EdgeLength, int Levels, DWORD Usage, D3DFORMAT Format, DWORD Pool, struct IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateVertexBuffer(int Length, DWORD Usage, DWORD FVF, DWORD Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateIndexBuffer(int Length, DWORD Usage, D3DFORMAT Format, DWORD Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateRenderTarget(int Width, int Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateDepthStencilSurface(int Width, int Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) = 0;
    virtual HRESULT STDMETHODCALLTYPE UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) = 0;
    virtual HRESULT STDMETHODCALLTYPE UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST struct RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetRenderTargetData(IDirect3DSurface9* pSourceSurface, IDirect3DSurface9* pDestSurface) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDepthStencilSurface(IDirect3DSurface9* pNewZStencilSurface) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) = 0;
    virtual HRESULT STDMETHODCALLTYPE BeginScene() = 0;
    virtual HRESULT STDMETHODCALLTYPE EndScene() = 0;
    virtual HRESULT STDMETHODCALLTYPE Clear(DWORD Count, CONST struct D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) = 0;
    virtual HRESULT STDMETHODCALLTYPE Present(CONST struct RECT* pSourceRect, CONST struct RECT* pDestRect, HWND hDestWindowOverride, CONST struct RGNDATA* pDirtyRegion) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetTransform(DWORD State, CONST struct D3DMATRIX* pMatrix) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTransform(DWORD State, struct D3DMATRIX* pMatrix) = 0;
    virtual HRESULT STDMETHODCALLTYPE MultiplyTransform(DWORD State, CONST struct D3DMATRIX* pMatrix) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetViewport(CONST struct D3DVIEWPORT9* pViewport) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetViewport(struct D3DVIEWPORT9* pViewport) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMaterial(CONST struct D3DMATERIAL9* pMaterial) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMaterial(struct D3DMATERIAL9* pMaterial) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetLight(DWORD Index, CONST struct D3DLIGHT9* pLight) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLight(DWORD Index, struct D3DLIGHT9* pLight) = 0;
    virtual HRESULT STDMETHODCALLTYPE LightEnable(DWORD Index, BOOL Enable) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLightEnable(DWORD Index, BOOL* pEnable) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetClipPlane(DWORD Index, CONST float* pPlane) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetClipPlane(DWORD Index, float* pPlane) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetRenderState(DWORD State, DWORD Value) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetRenderState(DWORD State, DWORD* pValue) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateStateBlock(DWORD Type, IDirect3DStateBlock9** ppSB) = 0;
    virtual HRESULT STDMETHODCALLTYPE BeginStateBlock() = 0;
    virtual HRESULT STDMETHODCALLTYPE EndStateBlock(IDirect3DStateBlock9** ppSB) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetClipStatus(CONST struct D3DCLIPSTATUS9* pClipStatus) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetClipStatus(struct D3DCLIPSTATUS9* pClipStatus) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTextureStageState(DWORD Stage, DWORD Type, DWORD* pValue) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD Stage, DWORD Type, DWORD Value) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSamplerState(DWORD Sampler, DWORD Type, DWORD* pValue) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSamplerState(DWORD Sampler, DWORD Type, DWORD Value) = 0;
    virtual HRESULT STDMETHODCALLTYPE ValidateDevice(DWORD* pNumPasses) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetNPatchMode(float nSegments) = 0;
    virtual float STDMETHODCALLTYPE GetNPatchMode() = 0;
    virtual HRESULT STDMETHODCALLTYPE DrawPrimitive(DWORD PrimitiveType, UINT StartVertex, UINT PrimitiveCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE DrawIndexedPrimitive(DWORD Type, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE DrawPrimitiveUP(DWORD PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) = 0;
    virtual HRESULT STDMETHODCALLTYPE DrawIndexedPrimitiveUP(DWORD PrimitiveType, UINT MinIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, DWORD IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) = 0;
    virtual HRESULT STDMETHODCALLTYPE ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateVertexDeclaration(CONST struct D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetFVF(DWORD FVF) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFVF(DWORD* pFVF) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetVertexShader(IDirect3DVertexShader9* pShader) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVertexShader(IDirect3DVertexShader9** ppShader) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetVertexShaderConstantF(BYTE Register, CONST float* pConstantData, UINT Vector4fCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVertexShaderConstantF(BYTE Register, float* pConstantData, UINT Vector4fCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetStreamSourceFreq(UINT StreamNumber, UINT Divider) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetStreamSourceFreq(UINT StreamNumber, UINT* pDivider) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetIndices(IDirect3DIndexBuffer9* pIndexData) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetIndices(IDirect3DIndexBuffer9** ppIndexData) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateQuery(DWORD Type, IDirect3DQuery9** ppQuery) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSoftwareVertexProcessing(BOOL bSoftware) = 0;
};

// Stub interface definitions (empty implementations)
struct IDirect3DVertexBuffer9 : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unlock() = 0;
};
struct IDirect3DIndexBuffer9 : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unlock() = 0;
};
struct IDirect3DBaseTexture9 : public IUnknown {};
struct IDirect3DTexture9 : public IDirect3DBaseTexture9 {
    virtual HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT Level, struct D3DSURFACE_DESC* pDesc) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSurfaceLevel(UINT Level, struct IDirect3DSurface9** ppSurfaceLevel) = 0;
    virtual HRESULT STDMETHODCALLTYPE LockRect(UINT Level, struct D3DLOCKED_RECT* pLockedRect, CONST struct RECT* pRect, DWORD Flags) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnlockRect(UINT Level) = 0;
    virtual UINT STDMETHODCALLTYPE GetLevelCount() = 0;
    virtual DWORD STDMETHODCALLTYPE GetPriority() = 0;
    virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD Priority) = 0;
};
struct IDirect3DCubeTexture9 : public IUnknown {};
struct IDirect3DVolumeTexture9 : public IUnknown {};
struct IDirect3DSurface9 : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetDesc(struct D3DSURFACE_DESC* pDesc) = 0;
    virtual HRESULT STDMETHODCALLTYPE LockRect(struct D3DLOCKED_RECT* pLockedRect, CONST struct RECT* pRect, DWORD Flags) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnlockRect() = 0;
};
struct IDirect3DSwapChain9 : public IUnknown {};
struct IDirect3DQuery9 : public IUnknown {};
struct IDirect3DStateBlock9 : public IUnknown {};
struct IDirect3DVertexShader9 : public IUnknown {};
struct IDirect3DPixelShader9 : public IUnknown {};
struct ID3DXFont : public IUnknown {};
struct IDirect3DVertexDeclaration9 : public IUnknown {};

// Typedef pointers
typedef IDirect3D9* LPDIRECT3D9;
typedef IDirect3DDevice9* LPDIRECT3DDEVICE9;
typedef IDirect3DVertexBuffer9* LPDIRECT3DVERTEXBUFFER9;
typedef IDirect3DIndexBuffer9* LPDIRECT3DINDEXBUFFER9;
typedef IDirect3DTexture9* LPDIRECT3DTEXTURE9;
typedef IDirect3DCubeTexture9* LPDIRECT3DCUBETEXTURE9;
typedef IDirect3DSurface9* LPDIRECT3DSURFACE9;
typedef IDirect3DStateBlock9* LPDIRECT3DSTATEBLOCK9;
typedef IDirect3DVertexShader9* LPDIRECT3DVERTEXSHADER9;
typedef IDirect3DPixelShader9* LPDIRECT3DPIXELSHADER9;
typedef ID3DXFont* LPD3DXFONT;

// D3D return codes
#define D3D_OK 0
#define D3DERR_WRONGTYPE ((HRESULT)0x88760868L)
#define D3DERR_INVALIDCALL ((HRESULT)0x8876086CL)
#define D3DERR_NOTAVAILABLE ((HRESULT)0x88760869L)
#define D3DERR_DEVICELOST ((HRESULT)0x88760869L)
#define D3DERR_DEVICENOTRESET ((HRESULT)0x8876087AL)
#define D3DERR_CONFLICTINGTEXTUREFILTER ((HRESULT)0x8876087CL)
#define D3DERR_CONFLICTINGTEXTUREPALETTE ((HRESULT)0x8876087DL)
#define D3DERR_TOOMANYOPERATIONS ((HRESULT)0x8876087EL)
#define D3DERR_UNSUPPORTEDALPHAARG ((HRESULT)0x8876087FL)
#define D3DERR_UNSUPPORTEDALPHAOPERATION ((HRESULT)0x88760880L)
#define D3DERR_UNSUPPORTEDCOLORARG ((HRESULT)0x88760881L)
#define D3DERR_UNSUPPORTEDCOLOROPERATION ((HRESULT)0x88760882L)
#define D3DERR_UNSUPPORTEDFACTORVALUE ((HRESULT)0x88760883L)
#define D3DERR_UNSUPPORTEDTEXTUREFILTER ((HRESULT)0x88760884L)
#define D3DERR_WRONGTEXTUREFORMAT ((HRESULT)0x88760885L)
#define D3DERR_OUTOFVIDEOMEMORY ((HRESULT)0x88760886L)

// D3DX constants
#define D3DX_DEFAULT ((UINT)-1)
#define E_FAIL 0x80000005
#define E_NOTIMPL 0x80004001
#define S_OK 0

// Helper macros
#ifndef HIWORD
#define HIWORD(l) ((unsigned short)((((unsigned long)(l)) >> 16) & 0xFFFF))
#endif
#ifndef LOWORD
#define LOWORD(l) ((unsigned short)(((unsigned long)(l)) & 0xFFFF))
#endif
#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif
#ifndef FAILED
#define FAILED(hr) ((( HRESULT)(hr)) < 0)
#endif

// D3D formats
#define D3DFMT_UNKNOWN 0
#define D3DFMT_A8R8G8B8 21
#define D3DFMT_X8R8G8B8 22
#define D3DFMT_R5G6B5 23
#define D3DFMT_X1R5G5B5 24
#define D3DFMT_A1R5G5B5 25
#define D3DFMT_A4R4G4B4 26
#define D3DFMT_R3G3B2 27
#define D3DFMT_A8 28
#define D3DFMT_A8R3G3B2 29
#define D3DFMT_X4R4G4B4 30
#define D3DFMT_A2B10G10R10 31
#define D3DFMT_A8B8G8R8 32
#define D3DFMT_X8B8G8R8 33
#define D3DFMT_R8G8B8 34
#define D3DFMT_DXT1 827611204
#define D3DFMT_DXT2 844715028
#define D3DFMT_DXT3 861165876
#define D3DFMT_DXT4 877942244
#define D3DFMT_DXT5 894720068
#define D3DFMT_D16_LOCKABLE 70
#define D3DFMT_D32 71
#define D3DFMT_L8 50
#define D3DFMT_P8 41
#define D3DFMT_INDEX16 101
#define D3DFMT_INDEX32 0x00000036L
#define D3DFMT_L16 0x54
#define D3DFMT_D32F_LOCKABLE 0x445F6978L
#define D3DFMT_D32_LOCKABLE 0x445F6976L
#define D3DFMT_A2R10G10B10 37
#define D3DFMT_A16B16G16R16 36
#define D3DFMT_A8P8 40
#define D3DFMT_A8L8 51
#define D3DFMT_A4L4 52
#define D3DFMT_V8U8 60
#define D3DFMT_L6V5U5 61
#define D3DFMT_X8L8V8U8 62
#define D3DFMT_Q8W8V8U8 63
#define D3DFMT_V16U16 64
#define D3DFMT_W11V11U10 65
#define D3DFMT_UYVY 0x59555955
#define D3DFMT_YUY2 0x32595559
#define D3DFMT_G16R16 115
#define D3DFMT_A2W10V10U10 119
#define D3DFMT_R8G8_B8G8 350
#define D3DFMT_G8R8_G8B8 351
#define D3DFMT_D15S1 67
#define D3DFMT_D24S8 75
#define D3DFMT_D16 80
#define D3DFMT_D24X8 73
#define D3DFMT_D24X4S4 77
#define D3DFMT_D24FS8 79

// D3D device types
#define D3DDEVTYPE_HAL 1
#define D3DDEVTYPE_REF 2
#define D3DDEVTYPE_SW 5
#define D3DDEVTYPE_NULLREF 4

// D3D device capabilities
#define D3DDEVCAPS_HWTRANSFORMANDLIGHT 0x00010000
#define D3DDEVCAPS_NPATCHES 0x00040000

// D3D raster capabilities
#define D3DPRASTERCAPS_DEPTHBIAS 0x04000000

// D3D caps2
#define D3DCAPS2_FULLSCREENGAMMA 0x00000001

// D3D texture filter caps
#define D3DPTFILTERCAPS_MAGFANISOTROPIC 0x00400000
#define D3DPTFILTERCAPS_MINFANISOTROPIC 0x00800000
#define D3DPTFILTERCAPS_MAGFLINEAR 0x00000040L
#define D3DPTFILTERCAPS_MINFLINEAR 0x00000020L
#define D3DPTFILTERCAPS_MIPFLINEAR 0x00000010L

// D3D texture op caps
#define D3DTEXOPCAPS_BUMPENVMAP 0x00000001
#define D3DTEXOPCAPS_BUMPENVMAPLUMINANCE 0x00000002
#define D3DTEXOPCAPS_ADD 0x00000001L
#define D3DTEXOPCAPS_DOTPRODUCT3 0x00008000L
#define D3DTEXOPCAPS_SELECTARG1 0x00020000L
#define D3DTEXOPCAPS_MODULATE 0x00040000L
#define D3DTEXOPCAPS_ADDSMOOTH 0x00100000L
#define D3DTEXOPCAPS_SUBTRACT 0x00010000L
#define D3DTEXOPCAPS_BLENDTEXTUREALPHA 0x04000000L
#define D3DTEXOPCAPS_BLENDCURRENTALPHA 0x08000000L

// D3D usage flags
#define D3DUSAGE_RENDERTARGET 0x00000001L
#define D3DUSAGE_DEPTHSTENCIL 0x00000002L
#define D3DUSAGE_WRITEONLY 0x00000008L
#define D3DUSAGE_SOFTWAREPROCESSING 0x00000020L
#define D3DUSAGE_DYNAMIC 0x00000200L
#define D3DUSAGE_NPATCHES 0x00040000L

// D3D lock flags
#define D3DLOCK_NOSYSLOCK 0x00000008L
#define D3DLOCK_DISCARD 0x00001000L
#define D3DLOCK_NOOVERWRITE 0x00002000L

// D3D lock flags
#define D3DLOCK_READONLY 0x00000010L

// D3DLOCKED_RECT struct
struct D3DLOCKED_RECT {
    int Pitch;
    void* pBits;
};

// D3D present flags
#define D3DPRESENT_DONOTWAIT 0x00000001L
#define D3DPRESENT_RATE_DEFAULT 0x00000000

// D3D resource types
#define D3DRTYPE_SURFACE 1
#define D3DRTYPE_VOLUME 2
#define D3DRTYPE_TEXTURE 3
#define D3DRTYPE_VOLUMETEXTURE 4
#define D3DRTYPE_CUBETEXTURE 5
#define D3DRTYPE_VERTEXBUFFER 6
#define D3DRTYPE_INDEXBUFFER 7

// D3D pool

// D3DFVF - Vertex format flags
#define D3DFVF_XYZ 0x002
#define D3DFVF_XYZRHW 0x004
#define D3DFVF_NORMAL 0x010
#define D3DFVF_PSIZE 0x020
#define D3DFVF_DIFFUSE 0x040
#define D3DFVF_SPECULAR 0x080
#define D3DFVF_TEX0 0x000
#define D3DFVF_TEX1 0x100
#define D3DFVF_TEX2 0x200
#define D3DFVF_TEX3 0x300
#define D3DFVF_TEX4 0x400
#define D3DFVF_TEX5 0x500
#define D3DFVF_TEX6 0x600
#define D3DFVF_TEX7 0x700
#define D3DFVF_TEX8 0x800

// Vertex blend weights
#define D3DFVF_XYZB1 0x0200
#define D3DFVF_XYZB2 0x0300
#define D3DFVF_XYZB3 0x0400
#define D3DFVF_XYZB4 0x0500
#define D3DFVF_LASTBETA_UBYTE4 0x1000
#define D3DFVF_LASTBETA 0x0fff

// Texture coord size macros (for texture stage 0-7)
#define D3DFVF_TEXCOORDSIZE1(i) (((i) << 8) | 0x100)
#define D3DFVF_TEXCOORDSIZE2(i) (((i) << 8) | 0x200)
#define D3DFVF_TEXCOORDSIZE3(i) (((i) << 8) | 0x300)
#define D3DFVF_TEXCOORDSIZE4(i) (((i) << 8) | 0x400)

// Blending
#undef D3DBLEND_ZERO
#undef D3DBLEND_ONE
#undef D3DBLEND_SRCCOLOR
#undef D3DBLEND_INVSRCCOLOR
#undef D3DBLEND_SRCALPHA
#undef D3DBLEND_INVSRCALPHA
#undef D3DBLEND_DESTALPHA
#undef D3DBLEND_INVDESTALPHA
#undef D3DBLEND_DESTCOLOR
#undef D3DBLEND_INVDESTCOLOR
#undef D3DBLEND_SRCALPHASAT
#undef D3DBLEND_BOTHSRCALPHA
#undef D3DBLEND_BOTHINVSRCALPHA

// Blend modes (enum)
enum D3DBLEND {
    D3DBLEND_ZERO = 1,
    D3DBLEND_ONE = 2,
    D3DBLEND_SRCCOLOR = 3,
    D3DBLEND_INVSRCCOLOR = 4,
    D3DBLEND_SRCALPHA = 5,
    D3DBLEND_INVSRCALPHA = 6,
    D3DBLEND_DESTALPHA = 9,
    D3DBLEND_INVDSTALPHA = 10,
    D3DBLEND_DESTCOLOR = 13,
    D3DBLEND_INVDESTCOLOR = 14,
    D3DBLEND_SRCALPHASAT = 15,
    D3DBLEND_BOTHSRCALPHA = 7,
    D3DBLEND_BOTHINVSRCALPHA = 8
};

#define D3DBLEND_ZERO D3DBLEND_ZERO
#define D3DBLEND_ONE D3DBLEND_ONE
#define D3DBLEND_SRCCOLOR D3DBLEND_SRCCOLOR
#define D3DBLEND_INVSRCCOLOR D3DBLEND_INVSRCCOLOR
#define D3DBLEND_SRCALPHA D3DBLEND_SRCALPHA
#define D3DBLEND_INVSRCALPHA D3DBLEND_INVSRCALPHA
#define D3DBLEND_DESTALPHA D3DBLEND_DESTALPHA
#define D3DBLEND_INVDESTALPHA D3DBLEND_INVDESTALPHA
#define D3DBLEND_DESTCOLOR D3DBLEND_DESTCOLOR
#define D3DBLEND_INVDESTCOLOR D3DBLEND_INVDESTCOLOR
#define D3DBLEND_SRCALPHASAT D3DBLEND_SRCALPHASAT
#define D3DBLEND_BOTHSRCALPHA D3DBLEND_BOTHSRCALPHA
#define D3DBLEND_BOTHINVSRCALPHA D3DBLEND_BOTHINVSRCALPHA

// Blend operation
#define D3DBLENDOP_ADD 1

// Depth function
#undef D3DCMP_NEVER
#undef D3DCMP_LESS
#undef D3DCMP_EQUAL
#undef D3DCMP_LESSEQUAL
#undef D3DCMP_GREATER
#undef D3DCMP_NOTEQUAL
#undef D3DCMP_GREATEREQUAL
#undef D3DCMP_ALWAYS

// Comparison functions (enum)
enum D3DCMPFUNC {
    D3DCMPFUNC_NEVER = 1,
    D3DCMPFUNC_LESS = 2,
    D3DCMPFUNC_EQUAL = 3,
    D3DCMPFUNC_LESSEQUAL = 4,
    D3DCMPFUNC_GREATER = 5,
    D3DCMPFUNC_NOTEQUAL = 6,
    D3DCMPFUNC_GREATEREQUAL = 7,
    D3DCMPFUNC_ALWAYS = 8
};

#define D3DCMP_NEVER D3DCMPFUNC_NEVER
#define D3DCMP_LESS D3DCMPFUNC_LESS
#define D3DCMP_EQUAL D3DCMPFUNC_EQUAL
#define D3DCMP_LESSEQUAL D3DCMPFUNC_LESSEQUAL
#define D3DCMP_GREATER D3DCMPFUNC_GREATER
#define D3DCMP_NOTEQUAL D3DCMPFUNC_NOTEQUAL
#define D3DCMP_GREATEREQUAL D3DCMPFUNC_GREATEREQUAL
#define D3DCMP_ALWAYS D3DCMPFUNC_ALWAYS

// Fill mode
#define D3DFILL_POINT 1
#define D3DFILL_WIREFRAME 2
#define D3DFILL_SOLID 3

// Cull mode
#define D3DCULL_NONE 1
#define D3DCULL_CW 2
#define D3DCULL_CCW 3

// Clear flags
#define D3DCLEAR_STENCIL 0x00000004L
#define D3DCLEAR_ZBUFFER 0x00000002L
#define D3DCLEAR_TARGET 0x00000001L

// D3DDP_MAXTEXCOORD
#define D3DDP_MAXTEXCOORD 8

// Struct definitions
struct D3DRECT {
    LONG x1;
    LONG y1;
    LONG x2;
    LONG y2;
};

struct D3DVECTOR {
    float x;
    float y;
    float z;
};

struct D3DMATRIX {
    union {
        struct { float m[4][4]; };
        float m16[16];
    };
};

struct D3DVIEWPORT9 {
    DWORD X;
    DWORD Y;
    DWORD Width;
    DWORD Height;
    float MinZ;
    float MaxZ;
};

struct D3DMATERIAL9 {
    D3DCOLORVALUE Diffuse;
    D3DCOLORVALUE Ambient;
    D3DCOLORVALUE Specular;
    D3DCOLORVALUE Emissive;
    float Power;
};

struct D3DLIGHT9 {
    D3DLIGHTTYPE Type;
    D3DCOLORVALUE Diffuse;
    D3DCOLORVALUE Specular;
    D3DCOLORVALUE Ambient;
    D3DVECTOR Position;
    D3DVECTOR Direction;
    float Range;
    float Falloff;
    float Attenuation0;
    float Attenuation1;
    float Attenuation2;
    float Theta;
    float Phi;
};

struct RECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

struct D3DVERTEXELEMENT9 {
    unsigned short Stream;
    unsigned short Offset;
    unsigned char Type;
    unsigned char Method;
    unsigned char Usage;
    unsigned char UsageIndex;
};

struct GAMMARAMP {
    unsigned short red[256];
    unsigned short green[256];
    unsigned short blue[256];
};

struct D3DRASTER_STATUS {
    BOOL InVBlank;
    UINT ScanLine;
};

struct D3DDEVICE_CREATION_PARAMETERS {
    UINT AdapterOrdinal;
    DWORD DeviceType;
    HWND hFocusWindow;
    DWORD BehaviorFlags;
};

struct D3DSURFACE_DESC {
    D3DFORMAT Format;
    DWORD Type;
    DWORD Usage;
    DWORD Pool;
    UINT Size;
    D3DMULTISAMPLE_TYPE MultiSampleType;
    DWORD MultiSampleQuality;
    UINT Width;
    UINT Height;
};

struct D3DRECTPATCH_INFO {
    UINT SegmentsX;
    UINT SegmentsY;
    float Point[4];
    float Curve[4];
    BOOL Normals;
};

struct D3DTRIPATCH_INFO {
    UINT Segments;
    float Point[3];
    float Curve[3];
    BOOL Normals;
};

struct D3DCLIPSTATUS9 {
    DWORD ClipUnion;
    DWORD ClipIntersection;
};

struct D3DADAPTER_IDENTIFIER9 {
    char Driver[512];
    char Description[512];
    char DeviceName[32];
    DWORD Flags;
    DWORD VendorId;
    DWORD DeviceId;
    // Driver version as LARGE_INTEGER-like structure
    struct {
        DWORD HighPart;
        DWORD LowPart;
    } DriverVersion;
    DWORD SubSysId;
    DWORD Revision;
    GUID DeviceIdentifier;
    DWORD WHQLLevel;
};

struct D3DCAPS9 {
    DWORD Caps;
    DWORD Caps2;
    DWORD Caps3;
    DWORD Caps4;
    DWORD Caps5;
    DWORD Caps6;
    DWORD Caps7;
    DWORD Caps8;
    DWORD PresentationIntervals;
    DWORD CursorCaps;
    DWORD DevCaps;
    DWORD MiscCaps;
    DWORD RawStampWidth;
    DWORD StampWidth;
    DWORD StampHeight;
    DWORD MaxTextureWidth;
    DWORD MaxTextureHeight;
    DWORD MaxVolumeExtent;
    DWORD MaxTextureRepeat;
    DWORD MaxTextureAspectRatio;
    DWORD MaxAnisotropy;
    DWORD MaxVertexW;
    float GuardBandLeft;
    float GuardBandTop;
    float GuardBandRight;
    float GuardBandBottom;
    float ExtentsAdjust;
    DWORD StencilCaps;
    DWORD FVFCaps;
    DWORD TextureOpCaps;
    DWORD TextureFilterCaps;
    DWORD VertexProcessingCaps;
    DWORD MiscCaps2;
    DWORD VertexShaderCaps;
    DWORD PixelShaderCaps;
    DWORD DeclTypes;
    DWORD NumSimultaneousRTs;
    DWORD StretchRectFilterCaps;
    DWORD RasterCaps;
    DWORD MaxSimultaneousTextures;
    UINT AdapterOrdinal;
    DWORD DeviceType;
    DWORD VSPortCaps;
    DWORD PSPortCaps;
    DWORD VertexShaderVersion;
    DWORD MaxVertexShaderConst;
    DWORD PixelShaderVersion;
    float PixelShader1xMaxValue;
    DWORD NumSimultaneousTS;
    DWORD TextureCaps;
    DWORD TextureAddressCaps;
    float TextureBumpOffset;
    DWORD VolumeTextureAddressCaps;
    DWORD WrapAndMipCaps;
    DWORD TransformedIndexCaps;
    DWORD TextureCaps2;
    DWORD Reserved;
};

struct D3DDISPLAYMODE {
    UINT Width;
    UINT Height;
    UINT RefreshRate;
    UINT Format;
};

struct D3DPRESENT_PARAMETERS_ {
    UINT BackBufferWidth;
    UINT BackBufferHeight;
    UINT BackBufferFormat;
    UINT BackBufferCount;
    UINT MultiSampleType;
    DWORD MultiSampleQuality;
    UINT SwapEffect;
    HWND hDeviceWindow;
    BOOL Windowed;
    BOOL EnableAutoDepthStencil;
    UINT AutoDepthStencilFormat;
    DWORD Flags;
    UINT FullScreen_RefreshRateInHz;
    UINT PresentationInterval;
};

struct RGNDATA {
    DWORD dwSize;
    DWORD iType;
    RECT rcBound;
};

// Stub creation function
inline LPDIRECT3D9 Direct3DCreate9(unsigned int SDKVersion) { (void)SDKVersion; return nullptr; }

// D3DX stub functions
inline unsigned int D3DXGetFVFVertexSize(unsigned int FVF) { (void)FVF; return 12; } // basic vertex size

// D3DX filter types
#define D3DX_FILTER_NONE 0x00000001L
#define D3DX_FILTER_POINT 0x00000002L
#define D3DX_FILTER_LINEAR 0x00000003L
#define D3DX_FILTER_TRIANGLE 0x00000004L
#define D3DX_FILTER_BOX 0x00000005L

// D3DX stub functions - texture
inline HRESULT D3DXCreateTextureFromFileExA(
    IDirect3DDevice9*, LPCSTR, UINT, UINT, UINT, DWORD,
    D3DFORMAT, D3DPOOL, DWORD, DWORD, D3DCOLOR,
    D3DXIMAGE_INFO*, PALETTEENTRY*, IDirect3DTexture9**)
{ return E_NOTIMPL; }

inline HRESULT D3DXLoadSurfaceFromSurface(
    IDirect3DSurface9* pDestSurface, CONST PALETTEENTRY* pDestPalette, CONST RECT* pDestRect,
    IDirect3DSurface9* pSrcSurface, CONST PALETTEENTRY* pSrcPalette, CONST RECT* pSrcRect,
    DWORD Filter, D3DCOLOR ColorKey)
{ (void)pDestSurface; (void)pDestPalette; (void)pDestRect; (void)pSrcSurface; (void)pSrcPalette; (void)pSrcRect; (void)Filter; (void)ColorKey; return E_NOTIMPL; }

inline HRESULT D3DXFilterTexture(IDirect3DTexture9* pTexture, CONST PALETTEENTRY* pPalette, UINT SubSet, DWORD Filter)
{ (void)pTexture; (void)pPalette; (void)SubSet; (void)Filter; return E_NOTIMPL; }

inline HRESULT D3DXCreateTexture(
    IDirect3DDevice9* pDevice, UINT Width, UINT Height, UINT MipLevels, DWORD Usage,
    D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture)
{ (void)pDevice; (void)Width; (void)Height; (void)MipLevels; (void)Usage; (void)Format; (void)Pool; *ppTexture = nullptr; return E_NOTIMPL; }

// CreateOffscreenPlainSurface stub
inline HRESULT IDirect3DDevice9_CreateOffscreenPlainSurface(
    IDirect3DDevice9* device, UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool,
    IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{ (void)device; (void)Width; (void)Height; (void)Format; (void)Pool; *ppSurface = nullptr; (void)pSharedHandle; return E_NOTIMPL; }

#endif // _D3D9_H_
#endif // !_WIN32
