#pragma once

// Android first-pass D3D9 compile shim. This is intentionally only a type/constant
// surface so non-rendering engine code can cross-compile before the real Android
// renderer (DXVK/Vulkan) is wired in.

#include <stdint.h>

#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

using BOOL = int;
using BYTE = uint8_t;
using WORD = uint16_t;
using DWORD = uint32_t;
using UINT = uint32_t;
using ULONG = uint32_t;
using FLOAT = float;
using HRESULT = long;
using HWND = void *;
using HANDLE = void *;
using HMONITOR = void *;
using LPCSTR = const char *;
using LPVOID = void *;
using REFIID = const void *;
using D3DCOLOR = uint32_t;
#ifndef CONST
#define CONST const
#endif

#ifndef S_OK
#define S_OK 0
#endif
#ifndef E_FAIL
#define E_FAIL ((HRESULT)0x80004005L)
#endif
#ifndef D3D_OK
#define D3D_OK S_OK
#endif
#ifndef D3DERR_INVALIDCALL
#define D3DERR_INVALIDCALL ((HRESULT)0x8876086CL)
#endif
#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif
#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif

struct RECT { long left, top, right, bottom; };
struct POINT { long x, y; };
struct D3DMATRIX { float m[4][4]; };
struct GUID { uint32_t Data1; uint16_t Data2; uint16_t Data3; uint8_t Data4[8]; };

struct IUnknown {
    virtual HRESULT QueryInterface(REFIID, void **) { return E_FAIL; }
    virtual ULONG AddRef() { return 1; }
    virtual ULONG Release() { return 1; }
};

struct IDirect3D9 : public IUnknown {};
struct IDirect3DSwapChain9 : public IUnknown {};
struct IDirect3DDevice9 : public IUnknown {
    template <typename... Args> HRESULT SetTransform(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT GetTransform(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT SetRenderState(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT GetRenderState(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT SetTextureStageState(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT SetSamplerState(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT SetTexture(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT DrawPrimitive(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT DrawIndexedPrimitive(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT SetMaterial(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT SetLight(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT LightEnable(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT SetNPatchMode(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT UpdateSurface(Args...) { return D3D_OK; }
    template <typename... Args> HRESULT GetRenderTargetData(Args...) { return D3D_OK; }
};
struct IDirect3DResource9 : public IUnknown {};
struct IDirect3DBaseTexture9 : public IDirect3DResource9 {};
struct IDirect3DTexture9 : public IDirect3DBaseTexture9 {};
struct IDirect3DCubeTexture9 : public IDirect3DBaseTexture9 {};
struct IDirect3DVolumeTexture9 : public IDirect3DBaseTexture9 {};
struct IDirect3DVertexBuffer9 : public IDirect3DResource9 {};
struct IDirect3DIndexBuffer9 : public IDirect3DResource9 {};
struct IDirect3DSurface9 : public IDirect3DResource9 {};
struct IDirect3DVolume9 : public IDirect3DResource9 {};
struct IDirect3DStateBlock9 : public IUnknown {};
struct IDirect3DVertexDeclaration9 : public IUnknown {};
struct IDirect3DVertexShader9 : public IUnknown {};
struct IDirect3DPixelShader9 : public IUnknown {};
struct IDirect3DQuery9 : public IUnknown {};

using LPDIRECT3D9 = IDirect3D9 *;
using LPDIRECT3DDEVICE9 = IDirect3DDevice9 *;
using LPDIRECT3DSWAPCHAIN9 = IDirect3DSwapChain9 *;
using LPDIRECT3DRESOURCE9 = IDirect3DResource9 *;
using LPDIRECT3DBASETEXTURE9 = IDirect3DBaseTexture9 *;
using LPDIRECT3DTEXTURE9 = IDirect3DTexture9 *;
using LPDIRECT3DCUBETEXTURE9 = IDirect3DCubeTexture9 *;
using LPDIRECT3DVOLUMETEXTURE9 = IDirect3DVolumeTexture9 *;
using LPDIRECT3DVERTEXBUFFER9 = IDirect3DVertexBuffer9 *;
using LPDIRECT3DINDEXBUFFER9 = IDirect3DIndexBuffer9 *;
using LPDIRECT3DSURFACE9 = IDirect3DSurface9 *;
using LPDIRECT3DVOLUME9 = IDirect3DVolume9 *;
using LPDIRECT3DSTATEBLOCK9 = IDirect3DStateBlock9 *;
using LPDIRECT3DVERTEXDECLARATION9 = IDirect3DVertexDeclaration9 *;
using LPDIRECT3DVERTEXSHADER9 = IDirect3DVertexShader9 *;
using LPDIRECT3DPIXELSHADER9 = IDirect3DPixelShader9 *;
using LPDIRECT3DQUERY9 = IDirect3DQuery9 *;

enum D3DFORMAT {
    D3DFMT_UNKNOWN = 0,
    D3DFMT_R8G8B8 = 20,
    D3DFMT_A8R8G8B8 = 21,
    D3DFMT_X8R8G8B8 = 22,
    D3DFMT_R5G6B5 = 23,
    D3DFMT_X1R5G5B5 = 24,
    D3DFMT_A1R5G5B5 = 25,
    D3DFMT_A4R4G4B4 = 26,
    D3DFMT_R3G3B2 = 27,
    D3DFMT_A8 = 28,
    D3DFMT_A8P8 = 40,
    D3DFMT_P8 = 41,
    D3DFMT_L8 = 50,
    D3DFMT_A8L8 = 51,
    D3DFMT_A4L4 = 52,
    D3DFMT_V8U8 = 60,
    D3DFMT_L6V5U5 = 61,
    D3DFMT_X8L8V8U8 = 62,
    D3DFMT_Q8W8V8U8 = 63,
    D3DFMT_V16U16 = 64,
    D3DFMT_A2W10V10U10 = 67,
    D3DFMT_R8G8_B8G8 = 68,
    D3DFMT_G8R8_G8B8 = 69,
    D3DFMT_UYVY = 0x59565955,
    D3DFMT_YUY2 = 0x32595559,
    D3DFMT_A8R3G3B2 = 29,
    D3DFMT_X4R4G4B4 = 30,
    D3DFMT_A2B10G10R10 = 31,
    D3DFMT_A8B8G8R8 = 32,
    D3DFMT_X8B8G8R8 = 33,
    D3DFMT_G16R16 = 34,
    D3DFMT_A2R10G10B10 = 35,
    D3DFMT_A16B16G16R16 = 36,
    D3DFMT_DXT1 = 0x31545844,
    D3DFMT_DXT2 = 0x32545844,
    D3DFMT_DXT3 = 0x33545844,
    D3DFMT_DXT4 = 0x34545844,
    D3DFMT_DXT5 = 0x35545844,
    D3DFMT_D16 = 80,
    D3DFMT_D16_LOCKABLE = 70,
    D3DFMT_D32 = 71,
    D3DFMT_D15S1 = 73,
    D3DFMT_D24S8 = 75,
};

enum D3DDEVTYPE { D3DDEVTYPE_HAL = 1, D3DDEVTYPE_REF = 2, D3DDEVTYPE_SW = 3 };
enum D3DRESOURCETYPE { D3DRTYPE_SURFACE = 1, D3DRTYPE_VOLUME = 2, D3DRTYPE_TEXTURE = 3, D3DRTYPE_VOLUMETEXTURE = 4, D3DRTYPE_CUBETEXTURE = 5, D3DRTYPE_VERTEXBUFFER = 6, D3DRTYPE_INDEXBUFFER = 7 };
enum D3DPOOL { D3DPOOL_DEFAULT = 0, D3DPOOL_MANAGED = 1, D3DPOOL_SYSTEMMEM = 2, D3DPOOL_SCRATCH = 3 };
enum D3DMULTISAMPLE_TYPE { D3DMULTISAMPLE_NONE = 0 };
enum D3DSWAPEFFECT { D3DSWAPEFFECT_DISCARD = 1, D3DSWAPEFFECT_FLIP = 2, D3DSWAPEFFECT_COPY = 3 };
enum D3DPRIMITIVETYPE { D3DPT_POINTLIST = 1, D3DPT_LINELIST = 2, D3DPT_LINESTRIP = 3, D3DPT_TRIANGLELIST = 4, D3DPT_TRIANGLESTRIP = 5, D3DPT_TRIANGLEFAN = 6 };
enum D3DTRANSFORMSTATETYPE { D3DTS_VIEW = 2, D3DTS_PROJECTION = 3, D3DTS_TEXTURE0 = 16, D3DTS_WORLD = 256 };
enum D3DRENDERSTATETYPE { D3DRS_ZENABLE = 7, D3DRS_FILLMODE = 8, D3DRS_SHADEMODE = 9, D3DRS_ZWRITEENABLE = 14, D3DRS_ALPHATESTENABLE = 15, D3DRS_SRCBLEND = 19, D3DRS_DESTBLEND = 20, D3DRS_CULLMODE = 22, D3DRS_ZFUNC = 23, D3DRS_ALPHAREF = 24, D3DRS_ALPHAFUNC = 25, D3DRS_DITHERENABLE = 26, D3DRS_ALPHABLENDENABLE = 27, D3DRS_FOGENABLE = 28, D3DRS_SPECULARENABLE = 29, D3DRS_FOGCOLOR = 34, D3DRS_FOGTABLEMODE = 35, D3DRS_FOGSTART = 36, D3DRS_FOGEND = 37, D3DRS_FOGDENSITY = 38, D3DRS_RANGEFOGENABLE = 48, D3DRS_LIGHTING = 137, D3DRS_AMBIENT = 139, D3DRS_COLORVERTEX = 141, D3DRS_LOCALVIEWER = 142, D3DRS_NORMALIZENORMALS = 143, D3DRS_DIFFUSEMATERIALSOURCE = 145, D3DRS_SPECULARMATERIALSOURCE = 146, D3DRS_AMBIENTMATERIALSOURCE = 147, D3DRS_EMISSIVEMATERIALSOURCE = 148, D3DRS_TEXTUREFACTOR = 60, D3DRS_MULTISAMPLEANTIALIAS = 161 };

enum D3DCMPFUNC { D3DCMP_NEVER = 1, D3DCMP_LESS = 2, D3DCMP_EQUAL = 3, D3DCMP_LESSEQUAL = 4, D3DCMP_GREATER = 5, D3DCMP_NOTEQUAL = 6, D3DCMP_GREATEREQUAL = 7, D3DCMP_ALWAYS = 8 };
enum D3DBLEND { D3DBLEND_ZERO = 1, D3DBLEND_ONE = 2, D3DBLEND_SRCCOLOR = 3, D3DBLEND_INVSRCCOLOR = 4, D3DBLEND_SRCALPHA = 5, D3DBLEND_INVSRCALPHA = 6, D3DBLEND_DESTALPHA = 7, D3DBLEND_INVDESTALPHA = 8, D3DBLEND_DESTCOLOR = 9, D3DBLEND_INVDESTCOLOR = 10 };
enum D3DCULL { D3DCULL_NONE = 1, D3DCULL_CW = 2, D3DCULL_CCW = 3 };
enum D3DFILLMODE { D3DFILL_POINT = 1, D3DFILL_WIREFRAME = 2, D3DFILL_SOLID = 3 };
enum D3DSHADEMODE { D3DSHADE_FLAT = 1, D3DSHADE_GOURAUD = 2 };
enum D3DFOGMODE { D3DFOG_NONE = 0, D3DFOG_EXP = 1, D3DFOG_EXP2 = 2, D3DFOG_LINEAR = 3 };
enum D3DTEXTURESTAGESTATETYPE { D3DTSS_COLOROP = 1, D3DTSS_COLORARG1 = 2, D3DTSS_COLORARG2 = 3, D3DTSS_ALPHAOP = 4, D3DTSS_ALPHAARG1 = 5, D3DTSS_ALPHAARG2 = 6, D3DTSS_TEXCOORDINDEX = 11, D3DTSS_TEXTURETRANSFORMFLAGS = 24 };
enum D3DTEXTUREOP { D3DTOP_DISABLE = 1, D3DTOP_SELECTARG1 = 2, D3DTOP_SELECTARG2 = 3, D3DTOP_MODULATE = 4, D3DTOP_ADD = 7, D3DTOP_SUBTRACT = 10, D3DTOP_BLENDTEXTUREALPHA = 15, D3DTOP_DOTPRODUCT3 = 24 };
enum D3DTA { D3DTA_DIFFUSE = 0, D3DTA_CURRENT = 1, D3DTA_TEXTURE = 2, D3DTA_TFACTOR = 3, D3DTA_SPECULAR = 4, D3DTA_COMPLEMENT = 16 };
enum D3DSAMPLERSTATETYPE { D3DSAMP_ADDRESSU = 1, D3DSAMP_ADDRESSV = 2, D3DSAMP_MAGFILTER = 5, D3DSAMP_MINFILTER = 6, D3DSAMP_MIPFILTER = 7, D3DSAMP_MAXMIPLEVEL = 9, D3DSAMP_MAXANISOTROPY = 10 };
enum D3DTEXTUREADDRESS { D3DTADDRESS_WRAP = 1, D3DTADDRESS_MIRROR = 2, D3DTADDRESS_CLAMP = 3 };
enum D3DTEXTUREFILTERTYPE { D3DTEXF_NONE = 0, D3DTEXF_POINT = 1, D3DTEXF_LINEAR = 2, D3DTEXF_ANISOTROPIC = 3 };
enum D3DDECLTYPE { D3DDECLTYPE_FLOAT1 = 0, D3DDECLTYPE_FLOAT2 = 1, D3DDECLTYPE_FLOAT3 = 2, D3DDECLTYPE_FLOAT4 = 3, D3DDECLTYPE_D3DCOLOR = 4, D3DDECLTYPE_UNUSED = 17 };
enum D3DDECLMETHOD { D3DDECLMETHOD_DEFAULT = 0 };
enum D3DDECLUSAGE { D3DDECLUSAGE_POSITION = 0, D3DDECLUSAGE_BLENDWEIGHT = 1, D3DDECLUSAGE_BLENDINDICES = 2, D3DDECLUSAGE_NORMAL = 3, D3DDECLUSAGE_PSIZE = 4, D3DDECLUSAGE_TEXCOORD = 5, D3DDECLUSAGE_TANGENT = 6, D3DDECLUSAGE_BINORMAL = 7, D3DDECLUSAGE_COLOR = 10 };

struct D3DADAPTER_IDENTIFIER9 { char Driver[512]; char Description[512]; char DeviceName[32]; uint64_t DriverVersion; DWORD VendorId, DeviceId, SubSysId, Revision; GUID DeviceIdentifier; DWORD WHQLLevel; };
struct D3DCAPS9 { DWORD DeviceType; UINT AdapterOrdinal; DWORD Caps, Caps2, Caps3, PresentationIntervals; DWORD CursorCaps, DevCaps, PrimitiveMiscCaps, RasterCaps, ZCmpCaps, SrcBlendCaps, DestBlendCaps, AlphaCmpCaps, ShadeCaps, TextureCaps, TextureFilterCaps, CubeTextureFilterCaps, VolumeTextureFilterCaps, TextureAddressCaps, VolumeTextureAddressCaps, LineCaps; DWORD MaxTextureWidth, MaxTextureHeight, MaxVolumeExtent, MaxTextureRepeat, MaxTextureAspectRatio, MaxAnisotropy; float MaxVertexW; float GuardBandLeft, GuardBandTop, GuardBandRight, GuardBandBottom, ExtentsAdjust; DWORD StencilCaps, FVFCaps, TextureOpCaps, MaxTextureBlendStages, MaxSimultaneousTextures, VertexProcessingCaps, MaxActiveLights, MaxUserClipPlanes, MaxVertexBlendMatrices, MaxVertexBlendMatrixIndex; float MaxPointSize; DWORD MaxPrimitiveCount, MaxVertexIndex, MaxStreams, MaxStreamStride, VertexShaderVersion, MaxVertexShaderConst, PixelShaderVersion, PixelShader1xMaxValue; };
struct D3DPRESENT_PARAMETERS { UINT BackBufferWidth, BackBufferHeight; D3DFORMAT BackBufferFormat; UINT BackBufferCount; D3DMULTISAMPLE_TYPE MultiSampleType; DWORD MultiSampleQuality; D3DSWAPEFFECT SwapEffect; HWND hDeviceWindow; BOOL Windowed; BOOL EnableAutoDepthStencil; D3DFORMAT AutoDepthStencilFormat; DWORD Flags; UINT FullScreen_RefreshRateInHz; UINT PresentationInterval; };
struct D3DLOCKED_RECT { int Pitch; void *pBits; };
struct D3DLOCKED_BOX { int RowPitch; int SlicePitch; void *pBits; };
struct D3DVIEWPORT9 { DWORD X, Y, Width, Height; float MinZ, MaxZ; };
struct D3DVERTEXELEMENT9 { WORD Stream; WORD Offset; BYTE Type; BYTE Method; BYTE Usage; BYTE UsageIndex; };
struct D3DCOLORVALUE { float r, g, b, a; };
struct D3DVECTOR { float x, y, z; };
struct D3DMATERIAL9 { D3DCOLORVALUE Diffuse, Ambient, Specular, Emissive; float Power; };
struct D3DLIGHT9 { int Type; D3DCOLORVALUE Diffuse, Specular, Ambient; D3DVECTOR Position, Direction; float Range, Falloff, Attenuation0, Attenuation1, Attenuation2, Theta, Phi; };
struct D3DSURFACE_DESC { D3DFORMAT Format; D3DRESOURCETYPE Type; DWORD Usage; D3DPOOL Pool; UINT Size, Width, Height; D3DMULTISAMPLE_TYPE MultiSampleType; DWORD MultiSampleQuality; };
struct D3DVOLUME_DESC { D3DFORMAT Format; D3DRESOURCETYPE Type; DWORD Usage; D3DPOOL Pool; UINT Width, Height, Depth; };
struct D3DBOX { UINT Left, Top, Right, Bottom, Front, Back; };
struct D3DRECT { long x1, y1, x2, y2; };

#define D3DCOLOR_ARGB(a,r,g,b) ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_XRGB(r,g,b) D3DCOLOR_ARGB(0xff,r,g,b)
#define D3DDECL_END() {0xFF,0,D3DDECLTYPE_UNUSED,0,0,0}
#define D3DFVF_XYZ 0x002
#define D3DFVF_XYZRHW 0x004
#define D3DFVF_NORMAL 0x010
#define D3DFVF_DIFFUSE 0x040
#define D3DFVF_SPECULAR 0x080
#define D3DFVF_TEX1 0x100
#define D3DFVF_TEX2 0x200
#define D3DDP_MAXTEXCOORD 8

inline IDirect3D9 *WINAPI Direct3DCreate9(UINT) { return nullptr; }
