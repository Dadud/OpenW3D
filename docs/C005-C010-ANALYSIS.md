# C005–C010 Implementation Analysis

## Overview

C005–C010 form a second modernization pass for the renderer. The first pass (C001–C004) added backend abstraction and BGFX skeleton. These tasks complete the migration from DX8 fixed-function to BGFX programmable pipeline.

---

## C005: ShaderClass Audit — Map bitmask → BGFX shader variants

### Current State

`ShaderClass` encodes 13 rendering parameters as a 32-bit bitmask (`ShaderBits`):
- Depth compare, depth write, color write (3 bits)
- Src/dst blend (2×3 bits)
- Fog, primary gradient, secondary gradient (3+1 bits)
- Alpha test, cull mode, texturing enable (3 bits)
- Post-detail color/alpha functions (4+3 bits)
- NPatch enable (1 bit)

`ShaderClass::Apply()` translates these to DX8 `SetRenderState()` calls.

### BGFX Equivalent

BGFX has no fixed-function pipeline. All state is encoded into a `ShaderKey` struct and used to select or generate a bgfx program. The existing `ShaderVariantCache` in `feature/bgfx-backend-v2` already implements the caching layer.

The existing `ShaderKey_From_DX8()` in `backends/bgfx/ShaderKey.h` maps most ShaderClass fields to ShaderKey bits. **Verification needed for completeness.**

### Implementation

1. **Verify `ShaderKey_From_DX8` mapping** (C005a):
   - All 13 ShaderClass fields must map
   - `POSTDETAIL` functions (DETAILCOLOR_*, DETAILALPHA_*) currently mapped but may need shader program variants
   - `NPatchEnable` — no BGFX equivalent; map to BGFX tessellation or skip
   - `SECONDARY_GRADIENT` — BGFX equivalent: second texture stage or additional shader uniform
   - `PRIGRADIENT` values beyond MODULATE/ADD — BUMPENVMAP, BUMPENVLUMINANCE, DOTPRODUCT3 need dedicated shader variants

2. **Add missing shader program variants** (C005b):
   - `fs_mesh.sc` and `vs_mesh.sc` currently exist
   - `fs_uber.sc` and `vs_uber.sc` exist in `bgfx-backend-combined` but were removed in `pr-3-bgfx-rendering-fixed` (simpler approach)
   - If using uber-shaders: add `#ifdef` branches for each `priGradient` value
   - If using separate programs: add variants for BUMP, BUMPLUM, DOTPRODUCT3

3. **Wire `ShaderVariantCache` into BGFXBackend draw path** (C005c):
   - `BGFXBackend::Apply_Shader_State(shader)` calls `m_shaderCache.GetOrCreate(key)` → sets bgfx program
   - DX8 path: `DX8Wrapper::Apply_Render_State_Changes()` calls `shader.Apply()` → DX8 states
   - Both paths must be coherent

### Files to Change
- `Code/ww3d2/backends/bgfx/ShaderKey.h` — verify/add missing mappings
- `Code/ww3d2/backends/bgfx/ShaderVariantCache.cpp` — implement `GetOrCreate` and `LoadOrCreateProgram`
- `Code/ww3d2/backends/bgfx/shaders/source/fs_uber.sc` — add gradient/detail variants
- `Code/ww3d2/backends/bgfx/shaders/source/vs_uber.sc` — add gradient variants
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp` — wire Apply_Shader_State into draw path

---

## C006: BGFXMaterialMapper Wiring

### Current State

`BGFXMaterialMapper` (in `backends/bgfx/`) provides static helper methods:
- `Shader_Blend_To_BGFX()` — src/dst blend → BGFX blend state
- `Shader_DepthCompare_To_BGFX()` — depth compare → BGFX depth test
- `Shader_Cull_To_BGFX()` — cull mode → BGFX cull state
- `Shader_To_BGFX_State()` — full ShaderClass → BGFX state mask

### What's Missing

The `MaterialMapper` is not called from anywhere in the BGFX backend draw path. The `BGFXBackend` stores a `_renderState` struct and applies changes via `Apply_Render_State_Changes()`. The `BGFXMaterialMapper` helpers need to be invoked when a `ShaderClass` is applied.

### Implementation

1. In `BGFXBackend::Apply_Shader_State(const ShaderClass& shader)`:
   - Call `BGFXMaterialMapper::Shader_To_BGFX_State(shader)` to build a BGFX state mask
   - OR `BGFXBackend` should maintain its own `uint64_t _bgfxState` and update only changed bits

2. When `Apply_Shader_State` is called during a draw:
   - Set blend state via `bgfx::setState()`
   - Set depth test/write via `bgfx::setDepth()` (or include in `setState`)

### Files to Change
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp` — wire MaterialMapper into Apply_Shader_State

---

## C007: Framebuffer/Render-Target Abstraction

### Current State

DX8Wrapper manages the DX8 swap chain (IDirect3DSwapChain8) and backbuffer directly. `Set_Render_Device()` creates the D3D device + swap chain. `Begin_Scene()`/`End_Scene()` manage the render target.

### BGFX Equivalent

BGFX has `bgfx::FrameBuffer` for render targets. The default frame buffer (window/swap chain) is accessed via `BGFX_INVALID_HANDLE`. Additional framebuffers (for shadows, post-processing) would use `bgfx::createFrameBuffer()`.

### Implementation

Add to `WW3DBackend` interface:
```cpp
virtual void* Create_Render_Target(int width, int height, int format) = 0;
virtual void Destroy_Render_Target(void* handle) = 0;
virtual void Set_Render_Target(void* handle) = 0;
virtual void Set_Render_Target_Index(int index) = 0;
```

Implement in `DX8Backend` (wrap DX8 render targets) and `BGFXBackend` (wrap bgfx framebuffers).

**Note**: This is a large architectural change. For a minimal pass, just ensure the default framebuffer (screen) works correctly. Shadow FBOs can be tracked separately as a follow-up.

### Files to Change
- `Code/ww3d2/ww3dbackend.h` — add render target methods
- `Code/ww3d2/dx8backend.cpp` — DX8 framebuffer wrapper
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp` — bgfx framebuffer wrapper

---

## C008: Texture Compression (DXT/BC via bimg)

### Current State

`DX8TextureManager` handles texture creation/loading but does not use DXT/BC compression. Textures are created in RGBA format. No BCn/DXT decoder is present.

### BGFX Equivalent

`bimg` (BGFX image library) supports BC1/BC3/BC5/BC7 decompression. BGFX accepts compressed texture data directly via `bgfx::createTexture2D()` with `BGFX_TEXTURE_BC*` flags.

### Implementation

1. Add `bimg` dependency to `cmake/bgfx.cmake`
2. In `BGFXTexture::Load()` (or `BGFXBackend::Create_Texture`):
   - Check if source texture is BCn compressed ( DDS header or file extension )
   - If compressed: pass raw BCn data to `bgfx::createTexture2D()` with appropriate flags
   - If uncompressed: keep existing path

3. Also consider: add BC compression as an **output** option for runtime texture compression

**Note**: `pr-3-bgfx-rendering-fixed` already has DXT1/3/5 texture support. This task is about adding the same to `feature/bgfx-backend-v2`.

### Files to Change
- `cmake/bgfx.cmake` — add bimg fetch
- `Code/ww3d2/backends/bgfx/BGFXTexture.cpp` — BCn loading path
- `external/bimg/` — add submodule

---

## C009: Vertex Buffer FVF → BGFX Layout Descriptors

### Current State

DX8 uses D3D FVF (Flexible Vertex Format) codes to describe vertex layout:
- `D3DFVF_XYZ`, `D3DFVF_NORMAL`, `D3DFVF_TEX2`, `D3DFVF_DIFFUSE`, etc.
- `VertexBufferClass` stores FVF code + stride

### BGFX Equivalent

BGFX uses `bgfx::VertexLayout` with explicit attribute descriptors:
```cpp
bgfx::VertexLayout layout;
layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true);
layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
```

### Implementation

1. Create a mapping: FVF code → `bgfx::VertexLayout`
2. In `BGFXVertexBuffer::Init()`:
   - Build `bgfx::VertexLayout` from the FVF
   - Create bgfx vertex buffer with the layout
3. FVF→BGFX attribute mapping:
   - D3DFVF_XYZ → Attrib::Position
   - D3DFVF_NORMAL → Attrib::Normal
   - D3DFVF_DIFFUSE → Attrib::Color0
   - D3DFVF_SPECULAR → Attrib::Color1
   - D3DFVF_TEX0..3 → Attrib::TexCoord0..3
   - D3DFVF_TANGENT → Attrib::Tangent (if available)

### Files to Change
- `Code/ww3d2/backends/bgfx/BGFXVertexBuffer.cpp` — FVF→layout conversion

---

## C010: Lighting → Shader Uniforms

### Current State

DX8 lights are set via `DX8Wrapper::Set_Light()` and `DX8Wrapper::Set_Light_Environment()`. These go to the DX8 fixed-function lighting engine. The shader then uses DX8 lights implicitly.

### BGFX Equivalent

BGFX has no built-in lighting. All lighting must be implemented in the shader programs:
- Upload light positions/colors/directions as `bgfx::UniformHandle` (uniform buffers)
- In the vertex/fragment shader: compute lighting per-fragment
- `BGFXBackend::Set_Light_Environment()` must upload light data as uniforms

### Implementation

1. Define BGFX uniform buffer for lights:
   ```cpp
   struct BGFXLightData {
       Vector3 LightPos[8];
       Vector3 LightColor[8];
       float LightIntensity[8];
       int LightType[8]; // 0=point, 1=directional, 2=spot
       int ActiveLightCount;
   };
   ```
2. In `BGFXBackend::Set_Light_Environment()`:
   - Parse the DX8 light environment data
   - Upload to bgfx uniform buffer
3. In `vs_mesh.sc` and `fs_mesh.sc`:
   - Apply lighting calculation using uploaded uniforms

**Note**: `pr-3-bgfx-rendering-fixed` has point light support in the shader and backend. This task is about ensuring it's wired correctly.

### Files to Change
- `Code/ww3d2/backends/bgfx/bgfxbackend.cpp` — upload light uniforms
- `Code/ww3d2/backends/bgfx/shaders/source/vs_mesh.sc` — apply lighting
- `Code/ww3d2/backends/bgfx/shaders/source/fs_mesh.sc` — apply lighting

---

## Dependency Graph

```
C005 (shader audit) ──┬── C006 (BGFXMaterialMapper wiring)
                      │
                      ├── C009 (FVF→layout) ──→ C005
                      │
                      └── C010 (lighting) ──→ C005

C007 (framebuffer) ───→ C005 (needs backend draw path)
C008 (BC textures) ───→ C005 (needs texture pipeline)
```

C005 is the critical path. It unblocks C006, C007, C008, C009, C010.
