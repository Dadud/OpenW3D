// ww3d_platform.h — Platform compatibility for OpenW3D
//
// On Windows, includes the real D3D9 / Windows headers.
// On other platforms, provides minimal type/enum stubs so shared ww3d2
// headers compile without the Direct3D SDK.
//
// This file uses #ifndef guards for every type/constant so it is safe
// to include after dxvk headers (which use #pragma once and define
// some of the same types as int-typedefs rather than enums).

#ifndef WW3D_PLATFORM_H
#define WW3D_PLATFORM_H

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <d3d9.h>
  #include <d3d9types.h>
  #include <d3d9caps.h>
#else
  #include "bittype.h"

  // If dxvk already defined these types, don't redefine them
  // (dxvk uses #pragma once, so we detect by existence, not include guards)

  #ifndef TRUE
    #define TRUE 1
  #endif
  #ifndef FALSE
    #define FALSE 0
  #endif
  #ifndef MAX_PATH
    #define MAX_PATH 260
  #endif
  #ifndef CALLBACK
    #define CALLBACK
  #endif
  #ifndef WINAPI
    #define WINAPI
  #endif
  #ifndef VOID
    #define VOID void
  #endif
  #ifndef CONST
    #define CONST const
  #endif

  // Win32 handles
  #ifndef HWND
    typedef void* HWND;
  #endif
  #ifndef HINSTANCE
    typedef void* HINSTANCE;
  #endif
  #ifndef HANDLE
    typedef void* HANDLE;
  #endif
  #ifndef LPARAM
    typedef long LPARAM;
  #endif
  #ifndef WPARAM
    typedef unsigned long WPARAM;
  #endif

  #ifndef HRESULT
    typedef long HRESULT;
  #endif
  #ifndef S_OK
    #define S_OK ((HRESULT)0L)
  #endif
  #ifndef E_FAIL
    #define E_FAIL ((HRESULT)0x80004005L)
  #endif

  #ifndef GUID
    typedef struct _GUID {
      unsigned long  Data1;
      unsigned short Data2;
      unsigned short Data3;
      unsigned char  Data4[8];
    } GUID;
  #endif

  #ifndef RECT
    typedef struct tagRECT {
      long left;
      long top;
      long right;
      long bottom;
    } RECT;
  #endif

  #ifndef POINT
    typedef struct tagPOINT {
      long x;
      long y;
    } POINT;
  #endif

  // Forward-declare D3D interface types (pointer-only usage on non-Windows)
  #ifndef IDirect3D9
    struct IDirect3D9;
  #endif
  #ifndef IDirect3DDevice9
    struct IDirect3DDevice9;
  #endif
  #ifndef IDirect3DTexture9
    struct IDirect3DTexture9;
  #endif
  #ifndef IDirect3DSurface9
    struct IDirect3DSurface9;
  #endif
  #ifndef IDirect3DVertexBuffer9
    struct IDirect3DVertexBuffer9;
  #endif
  #ifndef IDirect3DIndexBuffer9
    struct IDirect3DIndexBuffer9;
  #endif
  #ifndef IDirect3DBaseTexture9
    struct IDirect3DBaseTexture9;
  #endif
  #ifndef IDirect3DSwapChain9
    struct IDirect3DSwapChain9;
  #endif

  #ifndef D3DCOLOR
    typedef unsigned int D3DCOLOR;
  #endif

  // ---- D3DRENDERSTATETYPE ----
  #ifndef D3DRS_FILLMODE
    typedef enum _D3DRENDERSTATETYPE {
      D3DRS_FILLMODE = 8,
      D3DRS_SHADEMODE = 9,
      D3DRS_ZENABLE = 7,
      D3DRS_CULLMODE = 22,
      D3DRS_ALPHABLENDENABLE = 27,
      D3DRS_SRCBLEND = 19,
      D3DRS_DESTBLEND = 20,
      D3DRS_FOGENABLE = 28,
      D3DRS_FOGCOLOR = 34,
      D3DRS_FOGSTART = 36,
      D3DRS_FOGEND = 37,
      D3DRS_FOGDENSITY = 38,
      D3DRS_ZFUNC = 23,
      D3DRS_ZWRITEENABLE = 14,
      D3DRS_LIGHTING = 137,
      D3DRS_SPECULARENABLE = 29,
      D3DRS_AMBIENT = 123,
      D3DRS_AMBIENTMATERIALSOURCE = 142,
      D3DRS_DIFFUSEMATERIALSOURCE = 143,
      D3DRS_SPECULARMATERIALSOURCE = 144,
      D3DRS_EMISSIVEMATERIALSOURCE = 145,
      D3DRS_NORMALIZENORMALS = 232,
      D3DRS_COLORVERTEX = 141,
      D3DRS_CLIPPLANEENABLE = 152,
      D3DRS_STENCILENABLE = 155,
      D3DRS_STENCILFAIL = 156,
      D3DRS_STENCILZFAIL = 157,
      D3DRS_STENCILPASS = 158,
      D3DRS_STENCILFUNC = 151,
      D3DRS_STENCILREF = 154,
      D3DRS_STENCILMASK = 153,
      D3DRS_STENCILWRITEMASK = 159,
      D3DRS_ALPHATESTENABLE = 15,
      D3DRS_ALPHAREF = 24,
      D3DRS_ALPHAFUNC = 25,
      D3DRS_DITHERENABLE = 26,
      D3DRS_TEXTUREFACTOR = 60,
      D3DRS_MULTISAMPLEANTIALIAS = 161,
      D3DRS_MULTISAMPLEMASK = 162,
      D3DRS_SCISSORTESTENABLE = 174,
      D3DRS_SLOPESCALEDEPTHBIAS = 182,
      D3DRS_DEPTHBIAS = 183,
      D3DRS_SEPARATEALPHABLENDENABLE = 206,
      D3DRS_SRCBLENDALPHA = 207,
      D3DRS_DESTBLENDALPHA = 208,
      D3DRS_BLENDOPALPHA = 209,
      D3DRS_BLENDOP = 171,
      D3DRS_COLORWRITEENABLE = 168,
      D3DRS_WRAP0 = 128,
      D3DRS_WRAP1 = 129,
      D3DRS_WRAP2 = 130,
      D3DRS_WRAP3 = 131,
      D3DRS_WRAP4 = 132,
      D3DRS_WRAP5 = 133,
      D3DRS_WRAP6 = 134,
      D3DRS_WRAP7 = 135,
    } D3DRENDERSTATETYPE;
  #endif

  #ifndef D3DCULL_NONE
    typedef enum _D3DCULL {
      D3DCULL_NONE = 1,
      D3DCULL_CW = 2,
      D3DCULL_CCW = 3,
    } D3DCULL;
  #endif

  #ifndef D3DFILL_SOLID
    typedef enum _D3DFILLMODE {
      D3DFILL_POINT = 1,
      D3DFILL_WIREFRAME = 2,
      D3DFILL_SOLID = 3,
    } D3DFILLMODE;
  #endif

  #ifndef D3DBLEND_ZERO
    typedef enum _D3DBLEND {
      D3DBLEND_ZERO = 1,
      D3DBLEND_ONE = 2,
      D3DBLEND_SRCCOLOR = 3,
      D3DBLEND_INVSRCCOLOR = 4,
      D3DBLEND_SRCALPHA = 5,
      D3DBLEND_INVSRCALPHA = 6,
      D3DBLEND_DESTALPHA = 7,
      D3DBLEND_INVDESTALPHA = 8,
      D3DBLEND_DESTCOLOR = 9,
      D3DBLEND_INVDESTCOLOR = 10,
      D3DBLEND_SRCALPHASAT = 11,
      D3DBLEND_BOTHSRCALPHA = 12,
      D3DBLEND_BOTHINVSRCALPHA = 13,
      D3DBLEND_BLENDFACTOR = 14,
      D3DBLEND_INVBLENDFACTOR = 15,
    } D3DBLEND;
  #endif

  #ifndef D3DCMP_NEVER
    typedef enum _D3DCMPFUNC {
      D3DCMP_NEVER = 1,
      D3DCMP_LESS = 2,
      D3DCMP_EQUAL = 3,
      D3DCMP_GREATEREQUAL = 7,
      D3DCMP_ALWAYS = 8,
    } D3DCMPFUNC;
  #endif

  #ifndef D3DFOG_NONE
    typedef enum _D3DFOGMODE {
      D3DFOG_NONE = 0,
      D3DFOG_EXP = 1,
      D3DFOG_EXP2 = 2,
      D3DFOG_LINEAR = 3,
    } D3DFOGMODE;
  #endif

  #ifndef D3DZB_FALSE
    typedef enum _D3DZBUFFERTYPE {
      D3DZB_FALSE = 0,
      D3DZB_TRUE = 1,
      D3DZB_USEW = 2,
    } D3DZBUFFERTYPE;
  #endif

  #ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3) \
      ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) | \
       ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24))
  #endif

  #ifndef D3DFMT_UNKNOWN
    typedef enum _D3DFORMAT {
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
      D3DFMT_A4L4 = 52,
      D3DFMT_A8L8 = 51,
      D3DFMT_V8U8 = 60,
      D3DFMT_L6V5U5 = 61,
      D3DFMT_X8L8V8U8 = 62,
      D3DFMT_A8R3G3B2 = 29,
      D3DFMT_X4R4G4B4 = 30,
      D3DFMT_A2B10G10R10 = 31,
      D3DFMT_A8B8G8R8 = 32,
      D3DFMT_X8B8G8R8 = 33,
      D3DFMT_G16R16 = 34,
      D3DFMT_A2R10G10B10 = 35,
      D3DFMT_A16B16G16R16 = 36,
      D3DFMT_D16 = 80,
      D3DFMT_D32 = 71,
      D3DFMT_D24S8 = 75,
      D3DFMT_INDEX16 = 101,
      D3DFMT_INDEX32 = 102,
      D3DFMT_L8 = 50,
      D3DFMT_DXT1 = 827611204, // MAKEFOURCC('D','X','T','1')
      D3DFMT_DXT2 = 844420420,
      D3DFMT_DXT3 = 861081636,
      D3DFMT_DXT4 = 877742852,
      D3DFMT_DXT5 = 894404068,
    } D3DFORMAT;
  #endif

  #ifndef D3DPT_TRIANGLELIST
    typedef enum _D3DPRIMITIVETYPE {
      D3DPT_POINTLIST = 1,
      D3DPT_LINELIST = 2,
      D3DPT_LINESTRIP = 3,
      D3DPT_TRIANGLELIST = 4,
      D3DPT_TRIANGLESTRIP = 5,
      D3DPT_TRIANGLEFAN = 6,
    } D3DPRIMITIVETYPE;
  #endif

  #ifndef D3DFVF_XYZ
    #define D3DFVF_XYZ 0x002
    #define D3DFVF_XYZRHW 0x004
    #define D3DFVF_NORMAL 0x010
    #define D3DFVF_DIFFUSE 0x040
    #define D3DFVF_SPECULAR 0x080
    #define D3DFVF_TEX0 0x000
    #define D3DFVF_TEX1 0x100
    #define D3DFVF_TEX2 0x200
    #define D3DFVF_TEX3 0x300
    #define D3DFVF_TEXCOORDSIZE1(a) (0x00100000 | (a << 20))
    #define D3DFVF_TEXCOORDSIZE2(a) (0x00200000 | (a << 20))
    #define D3DFVF_TEXCOORDSIZE3(a) (0x00300000 | (a << 20))
    #define D3DFVF_TEXCOORDSIZE4(a) (0x00400000 | (a << 20))
    #define D3DFVF_POSITION_MASK 0x00E
  #endif

  #ifndef D3DDP_MAXTEXCOORD
    #define D3DDP_MAXTEXCOORD 8
  #endif

  #ifndef D3DDEVTYPE_HAL
    typedef enum _D3DDEVTYPE {
      D3DDEVTYPE_HAL = 1,
      D3DDEVTYPE_REF = 2,
      D3DDEVTYPE_SW = 3,
    } D3DDEVTYPE;
  #endif

  #ifndef D3DTA_SELECTMASK
    #define D3DTA_SELECTMASK 0x0000000f
    #define D3DTA_DIFFUSE 0x00000000
    #define D3DTA_CURRENT 0x00000001
    #define D3DTA_TEXTURE 0x00000002
    #define D3DTA_TFACTOR 0x00000003
    #define D3DTA_SPECULAR 0x00000004
  #endif

  #ifndef D3DTSS_COLOROP
    typedef enum _D3DTEXTURESTAGESTATETYPE {
      D3DTSS_COLOROP = 1,
      D3DTSS_COLORARG1 = 2,
      D3DTSS_COLORARG2 = 3,
      D3DTSS_ALPHAOP = 4,
      D3DTSS_ALPHAARG1 = 5,
      D3DTSS_ALPHAARG2 = 6,
      D3DTSS_BUMPENVMAT00 = 7,
      D3DTSS_BUMPENVMAT01 = 8,
      D3DTSS_BUMPENVMAT10 = 9,
      D3DTSS_BUMPENVMAT11 = 10,
      D3DTSS_ADDRESSU = 11,
      D3DTSS_ADDRESSV = 12,
      D3DTSS_ADDRESSW = 13,
      D3DTSS_BORDERCOLOR = 14,
      D3DTSS_MAGFILTER = 16,
      D3DTSS_MINFILTER = 17,
      D3DTSS_MIPFILTER = 18,
      D3DTSS_MIPMAPLODBIAS = 19,
      D3DTSS_MAXMIPLEVEL = 20,
      D3DTSS_MAXANISOTROPY = 21,
      D3DTSS_BUMPENVLSCALE = 22,
      D3DTSS_BUMPENVLOFFSET = 23,
      D3DTSS_COLORARG0 = 24,
      D3DTSS_ALPHAARG0 = 25,
      D3DTSS_RESULTARG = 26,
      D3DTSS_TEXTURETRANSFORMFLAGS = 27,
    } D3DTEXTURESTAGESTATETYPE;
  #endif

  // D3DTSS_TEXTURETRANSFORMFLAGS also exists as a standalone macro for compatibility
  // On Windows: it is part of D3DTEXTURESTAGESTATETYPE enum (value 20 in d3d9.h, but 27 in d3d9types.h)
  // On non-Windows: we define it as macro 20 for compatibility with code that uses it as a constant
  #ifndef D3DTSS_TEXTURETRANSFORMFLAGS
    #define D3DTSS_TEXTURETRANSFORMFLAGS ((D3DTEXTURESTAGESTATETYPE)20)
  #endif

  #ifndef D3DTSS_TCI_CAMERASPACEPOSITION
    #define D3DTSS_TCI_CAMERASPACEPOSITION ((D3DTEXTURESTAGESTATETYPE)0x00010000)
  #endif
  #ifndef D3DTSS_TCI_SPACEPOSITION
    #define D3DTSS_TCI_SPACEPOSITION ((D3DTEXTURESTAGESTATETYPE)0x00020000)
  #endif
  #ifndef D3DTSS_TCI_CAMERASPACENORMAL
    #define D3DTSS_TCI_CAMERASPACENORMAL ((D3DTEXTURESTAGESTATETYPE)0x00030000)
  #endif
  #ifndef D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR
    #define D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR ((D3DTEXTURESTAGESTATETYPE)0x00040000)
  #endif
  #ifndef D3DTSS_TCI_PASSTHRU
    #define D3DTSS_TCI_PASSTHRU ((D3DTEXTURESTAGESTATETYPE)0x00000000)
  #endif

  // D3DTSS_TEXCOORDINDEX is the actual indexed texture coordinate stage state
  #ifndef D3DTSS_TEXCOORDINDEX
    #define D3DTSS_TEXCOORDINDEX ((D3DTEXTURESTAGESTATETYPE)16)
  #endif

  // TEXCOORDINDEX is a legacy alias for D3DTSS_TEXTURETRANSFORMFLAGS (they share value 16)
  #ifndef D3DTEXCOORDINDEX
    #define D3DTEXCOORDINDEX 16
    #define TEXCOORDINDEX 16
  #endif

  #ifndef FLOAT
    #define FLOAT float
  #endif

  #ifndef F2DW
    static inline unsigned int _F2DW_impl(float f) { union { float f; unsigned int u; } x; x.f = f; return x.u; }
    #define F2DW(f) _F2DW_impl(f)
  #endif

  #ifndef D3DTEXF_NONE
    typedef enum _D3DTEXTUREFILTERTYPE {
      D3DTEXF_NONE = 0,
      D3DTEXF_POINT = 1,
      D3DTEXF_LINEAR = 2,
      D3DTEXF_ANISOTROPIC = 3,
    } D3DTEXTUREFILTERTYPE;
  #endif

  #ifndef D3DTADDRESS_WRAP
    typedef enum _D3DTEXTUREADDRESS {
      D3DTADDRESS_WRAP = 1,
      D3DTADDRESS_MIRROR = 2,
      D3DTADDRESS_CLAMP = 3,
      D3DTADDRESS_BORDER = 4,
      D3DTADDRESS_MIRRORONCE = 5,
    } D3DTEXTUREADDRESS;
  #endif

  #ifndef D3DTS_VIEW
    typedef enum _D3DTRANSFORMSTATETYPE {
      D3DTS_VIEW = 2,
      D3DTS_PROJECTION = 3,
      D3DTS_TEXTURE0 = 16,
      D3DTS_TEXTURE1 = 17,
      D3DTS_WORLD = 256,
    } D3DTRANSFORMSTATETYPE;
  #endif

  #ifndef D3DSAMP_ADDRESSU
    typedef enum _D3DSAMPLERSTATETYPE {
      D3DSAMP_ADDRESSU = 1,
      D3DSAMP_ADDRESSV = 2,
      D3DSAMP_ADDRESSW = 3,
      D3DSAMP_MAGFILTER = 5,
      D3DSAMP_MINFILTER = 6,
      D3DSAMP_MIPFILTER = 7,
      D3DSAMP_MAXANISOTROPY = 10,
    } D3DSAMPLERSTATETYPE;
  #endif

  #ifndef D3DLIGHT_POINT
    typedef enum _D3DLIGHTTYPE {
      D3DLIGHT_POINT = 1,
      D3DLIGHT_SPOT = 2,
      D3DLIGHT_DIRECTIONAL = 3,
    } D3DLIGHTTYPE;
  #endif

  #ifndef D3DPOOL_DEFAULT
    typedef enum _D3DPOOL {
      D3DPOOL_DEFAULT = 0,
      D3DPOOL_MANAGED = 1,
      D3DPOOL_SYSTEMMEM = 2,
    } D3DPOOL;
  #endif

  #ifndef D3DLOCK_READONLY
    #define D3DLOCK_READONLY 0x00000010
    #define D3DLOCK_DISCARD 0x00002000
    #define D3DLOCK_NOOVERWRITE 0x00001000
  #endif

  #ifndef D3D_OK
    #define D3D_OK 0
  #endif

  #ifndef _MAX_FNAME
    #define _MAX_FNAME 256
  #endif
  #ifndef _MAX_EXT
    #define _MAX_EXT 256
  #endif
  #ifndef _MAX_DRIVE
    #define _MAX_DRIVE 3
  #endif
  #ifndef _MAX_DIR
    #define _MAX_DIR 256
  #endif
  #ifndef _MAX_PATH
    #define _MAX_PATH 260
  #endif

  // D3D structures needed by dx8stubs.cpp and other non-Windows stubs
  #ifndef D3DVECTOR
    typedef struct _D3DVECTOR {
      float x, y, z;
    } D3DVECTOR;
  #endif

  #ifndef D3DMATRIX_DEFINED
    typedef struct _D3DMATRIX {
      float m[4][4];
    } D3DMATRIX;
    #define D3DMATRIX_DEFINED
  #endif

  #ifndef D3DCOLORVALUE
    typedef struct _D3DCOLORVALUE {
      float r, g, b, a;
    } D3DCOLORVALUE;
  #endif

  #ifndef D3DVIEWPORT9
    typedef struct _D3DVIEWPORT9 {
      unsigned int X, Y;
      unsigned int Width, Height;
      float MinZ, MaxZ;
    } D3DVIEWPORT9;
  #endif

  #ifndef D3DMATERIAL9
    typedef struct _D3DMATERIAL9 {
      D3DCOLORVALUE Diffuse;
      D3DCOLORVALUE Ambient;
      D3DCOLORVALUE Specular;
      D3DCOLORVALUE Emissive;
      float Power;
    } D3DMATERIAL9;
  #endif

  #ifndef D3DLIGHT9
    typedef struct _D3DLIGHT9 {
      D3DLIGHTTYPE Type;
      D3DCOLORVALUE Diffuse;
      D3DCOLORVALUE Specular;
      D3DCOLORVALUE Ambient;
      D3DVECTOR Position;
      D3DVECTOR Direction;
      float Range;
      float Falloff;
      float Attenuation0, Attenuation1, Attenuation2;
      float Theta, Phi;
    } D3DLIGHT9;
  #endif

  #ifndef D3DADAPTER_IDENTIFIER9
    typedef struct _D3DADAPTER_IDENTIFIER9 {
      char Driver[512];
      char Description[512];
      char DeviceName[32];
      unsigned int DriverVersionLowPart;
      unsigned int DriverVersionHighPart;
      unsigned int VendorId;
      unsigned int DeviceId;
      unsigned int SubSysId;
      unsigned int Revision;
      void* DeviceIdentifier;
      unsigned int WHQLLevel;
    } D3DADAPTER_IDENTIFIER9;
  #endif

  #ifndef D3DCAPS9
    typedef struct _D3DCAPS9 {
      unsigned int DeviceType;
      unsigned int AdapterOrdinal;
      int Caps;
      int Caps2;
      int Caps3;
      int PresentationIntervals;
      int CursorCaps;
      int DevCaps;
      int RhoadsResolution;
      int CommandQueuePipelineDepth;
      int DedicatedVideoMemory;
      int DedicatedSystemMemory;
      int SharedSystemMemory;
      int TextureOpCaps;
    } D3DCAPS9;
  #endif

  #ifndef D3D_CLEAR_TARGET
    #define D3D_CLEAR_TARGET 0x00000001
  #endif
  #ifndef D3D_CLEAR_ZBUFFER
    #define D3D_CLEAR_ZBUFFER 0x00000002
  #endif
  #ifndef D3D_CLEAR_STENCIL
    #define D3D_CLEAR_STENCIL 0x00000004
  #endif

  #ifndef D3DCLIPPLANE0
    #define D3DCLIPPLANE0 0x00000001
    #define D3DCLIPPLANE1 0x00000002
    #define D3DCLIPPLANE2 0x00000004
    #define D3DCLIPPLANE3 0x00000008
    #define D3DCLIPPLANE4 0x00000010
    #define D3DCLIPPLANE5 0x00000020
  #endif

  // ---- D3DTEXTUREOP (for shader.cpp) ----
  #ifndef D3DTOP_DISABLE
    typedef enum _D3DTEXTUREOP {
      D3DTOP_DISABLE = 1,
      D3DTOP_SELECTARG1 = 2,
      D3DTOP_SELECTARG2 = 3,
      D3DTOP_MODULATE = 4,
      D3DTOP_MODULATE2X = 5,
      D3DTOP_MODULATE4X = 6,
      D3DTOP_ADD = 7,
      D3DTOP_ADDSIGNED = 8,
      D3DTOP_ADDSMOOTH = 9,
      D3DTOP_SUBTRACT = 10,
      D3DTOP_ADDSMOOTH2 = 11,
      D3DTOP_BLENDTEXTUREALPHA = 14,
      D3DTOP_BLENDCURRENTALPHA = 18,
      D3DTOP_BUMPENVMAP = 19,
      D3DTOP_BUMPENVMAPLUMINANCE = 20,
      D3DTOP_DOTPRODUCT3 = 21,
    } D3DTEXTUREOP;
  #endif

  // ---- D3DTEXOPCAPS (for shader.cpp) ----
  #ifndef D3DTEXOPCAPS_SELECTARG1
    #define D3DTEXOPCAPS_SELECTARG1 0x00000001
    #define D3DTEXOPCAPS_SELECTARG2 0x00000002
    #define D3DTEXOPCAPS_MODULATE 0x00000004
    #define D3DTEXOPCAPS_MODULATE2X 0x00000008
    #define D3DTEXOPCAPS_MODULATE4X 0x00000010
    #define D3DTEXOPCAPS_ADD 0x00000020
    #define D3DTEXOPCAPS_ADDSIGNED 0x00000040
    #define D3DTEXOPCAPS_ADDSMOOTH 0x00000080
    #define D3DTEXOPCAPS_SUBTRACT 0x00000100
    #define D3DTEXOPCAPS_ADDSMOOTH2 0x00000200
    #define D3DTEXOPCAPS_BLENDTEXTUREALPHA 0x00000400
    #define D3DTEXOPCAPS_BLENDCURRENTALPHA 0x00000800
    #define D3DTEXOPCAPS_BUMPENVMAP 0x00001000
    #define D3DTEXOPCAPS_BUMPENVMAPLUMINANCE 0x00002000
    #define D3DTEXOPCAPS_DOTPRODUCT3 0x00004000
  #endif

  // ---- D3DTTFF_* texture transform flags ----
  #ifndef D3DTTFF_DISABLE
    #define D3DTTFF_DISABLE 0x00000000
    #define D3DTTFF_COUNT1 0x00010000
    #define D3DTTFF_COUNT2 0x00020000
    #define D3DTTFF_COUNT3 0x00030000
    #define D3DTTFF_PROJECTED 0x00010000
  #endif

  // ---- D3DMCS_* MaterialColorSource (for vertmaterial.cpp) ----
  #ifndef D3DMCS_COLOR1
    #define D3DMCS_COLOR1 1
    #define D3DMCS_COLOR2 2
    #define D3DMCS_MATERIAL 7
  #endif

  // ---- D3DCMP_LESSEQUAL (for shader.cpp) ----
  #ifndef D3DCMP_LESSEQUAL
    #define D3DCMP_LESSEQUAL 4
  #endif

  // ---- D3DLOCKED_RECT (for surfaceclass.cpp, textureloader.cpp) ----
  #ifndef D3DLOCKED_RECT
    typedef struct _D3DLOCKED_RECT {
      int Pitch;
      void* pBits;
    } D3DLOCKED_RECT;
  #endif

  // ---- D3DRESOURCETYPE ----
  #ifndef D3DRESOURCETYPE
    typedef enum _D3DRESOURCETYPE {
      D3DRTYPE_SURFACE = 1,
      D3DRTYPE_VOLUME = 2,
      D3DRTYPE_TEXTURE = 3,
    } D3DRESOURCETYPE;
  #endif

  // ---- D3DSURFACE_DESC (for surfaceclass.cpp) ----
  #ifndef D3DSURFACE_DESC
    typedef struct _D3DSURFACE_DESC {
      D3DFORMAT Format;
      D3DRESOURCETYPE Type;
      unsigned int Usage;
      D3DPOOL Pool;
      unsigned int Size;
      unsigned int Width;
      unsigned int Height;
    } D3DSURFACE_DESC;
  #endif

  // ---- D3DX constants (for surfaceclass.cpp) ----
  #ifndef D3DX_FILTER_NONE
    #define D3DX_FILTER_NONE 0x00000001
  #endif
  #ifndef D3DX_FILTER_TRIANGLE
    #define D3DX_FILTER_TRIANGLE 0x00000002
  #endif

  // ---- Additional D3DFVF flags (for bgfxbackend.cpp) ----
  #ifndef D3DFVF_XYZB1
    #define D3DFVF_XYZB1 0x00000014
  #endif
  #ifndef D3DFVF_XYZB5
    #define D3DFVF_XYZB5 0x00000034
  #endif
  #ifndef D3DFVF_LASTBETA_UBYTE4
    #define D3DFVF_LASTBETA_UBYTE4 0x04000000
  #endif
  #ifndef D3DFVF_TEXCOUNT_MASK
    #define D3DFVF_TEXCOUNT_MASK 0x0F000000
  #endif
  #ifndef D3DFVF_TEXCOUNT_SHIFT
    #define D3DFVF_TEXCOUNT_SHIFT 24
  #endif

  // ---- timeGetTime stub (for textureloader.cpp) ----
  #ifndef timeGetTime
    #include <time.h>
    #define timeGetTime() ((unsigned int)((clock() * 1000) / CLOCKS_PER_SEC))
  #endif

#endif  // !_WIN32

#endif  // WW3D_PLATFORM_H
