# OpenW3D Task Tracker

## Legend
- ✅ Done and verified in branch
- ⚠️ Blocked / needs decision
- ⬜ Not started

---

## Track A — Build system (complete)

- [x] A001: CMake migration
- [x] A002: MSYS2/Windows build
- [x] A003: Linux build
- [x] A004: macOS build (not tested)
- [x] A005: CI/CD (workflows disabled from auto-run — see below)

**CI Note:** `openw3d.yml` and `docs-lint.yml` currently set to `on: [workflow_dispatch]` in the working clone. To re-enable, change back to `on: [push, pull_request]`.

---

## Track B — Audio engine (complete)

- [x] B001: Null audio (no-op backend)
- [x] B002: Audio manager abstraction
- [x] B003: OpenAL backend
- [x] B004: Miles fallback
- [x] B005: Audio format registration
- [x] B006: macOS audio configure path (not tested)

---

## Track C — Renderer backend abstraction

### Phase 1: Core backend infrastructure (complete)

- [x] C001: WW3DBackend interface — pure virtual ~50 methods in `Code/ww3d2/ww3dbackend.h`
- [x] C002: DX8Backend implementation — wraps existing DX8 code behind interface
- [x] C003: NullBackend — no-op renderer (`Null3DObjClass` from `nullrobj.h`)
- [x] C004: BGFXBackend skeleton — 2261-line stub in `backends/bgfx/bgfxbackend.h/cpp`

### Phase 2: Follow-up modernization (all verified implemented in `feature/bgfx-backend-v2`)

> Verified by reading actual source files in `origin/feature/bgfx-backend-v2`.

- [x] C005: ✅ `ShaderKey_From_DX8()` maps ShaderClass bitmask → ShaderKey struct. `ShaderVariantCache::GetOrCreateProgram()` loads compiled uber-shaders. `vs_uber.sc` / `fs_uber.sc` handle gradient/texturing/lighting via runtime uniforms.
- [x] C006: ✅ `BGFXMaterialMapper::Shader_To_BGFX_State()` wired into `BGFXBackend::Apply_Shader_State()`. Converts ShaderClass blend/depth/cull to BGFX state mask.
- [x] C007: ✅ `BGFXBackend::Create_FrameBuffer()`, `m_frameBuffer`, `m_activeFrameBuffer`. Default framebuffer initialized in `Init()`.
- [x] C008: ✅ `WW3D_FORMAT_DXT1/2/3/4/5` → `bgfx::TextureFormat::BC1/2/3`. `BGFXTextureFormat::BC1/2/3` mapped to BGFX equivalents.
- [x] C009: ✅ `BGFXVertexBuffer::Make_Layout()` converts WW3D FVF enum to `bgfx::VertexLayout` with Position/Normal/Color0/TexCoord0..3 attributes.
- [x] C010: ✅ `BGFXBackend::Apply_Light_Environment_State()` uploads ambient light, directional/point lights (up to 4) as `m_ambientLightUniform`, `m_lightDirUniform`, `m_lightDiffuseUniform`, `m_lightPosUniform`.

---

## Track D — Audio/Video decoupling

- [x] D001: FFmpegFile extraction — yes, already in the codebase
- [x] D002: ✅ **Fixed** — `fix/d002-broken-linkage` branch = `upstream/main` with `openw3d.cpp` restored. PR #103 deletes `openw3d.cpp` but leaves 24 call sites to `OpenW3D::Save_Config` / `Get_Config_File_Path` / `Has_Config_File_Path_Override` / `Set_Config_File_Path_From_Command_Line`. Restore openw3d.cpp to fix.
- [x] D003: ✅ **Verified clean** — PR #103 FFmpegFile changes (`Set_Frame_Callback`, `Set_User_Data`, audio decode callback API) are non-breaking. `FFMpegBuffer` coupling with OpenAL is intentional and correct.
- [x] D004: Bink disabled by default — `OPENW3D_BUILD_BINK=OFF`
- [x] D005: FFmpeg enabled by default — `W3D_BUILD_OPTION_FFMPEG=ON` in `Code/BinkMovie/CMakeLists.txt`
- [x] D006: ✅ **Done** — `feature/d006-media-test` pushed. `tests/media/README.md` with manual verification procedure. `tests/test_media.cpp` smoke test for `FFMpegPlayer` linkage.

---

## Branches Summary

| Branch | Description | Status |
|--------|-------------|--------|
| `upstream/main` | W3DHub main (contains broken PR #103) | Base |
| `origin/feature/backend-abstraction` | WW3DBackend interface + DX8/Null impl | C001-C003 done |
| `origin/feature/bgfx-backend-v2` | BGFX layered on backend abstraction | C005-C010 done |
| `origin/fix/d002-broken-linkage` | upstream/main with openw3d.cpp restored | D002 done |
| `origin/feature/d006-media-test` | Media test + smoke test | D006 done |
| `pr-103-openal` | OmniBlade's OpenAL PR (D002 broken, D003 verified) | Needs D002 fix |

---

## What's Needed to Land

1. **Merge `fix/d002-broken-linkage` into PR #103** — Apply PR #103 on top of our fix branch, or re-add `openw3d.cpp` to the PR directly.
2. **Compile test** — No build environment available to verify C005-C010 actually compile together.
3. **CI re-enable** — After landing, change `openw3d.yml` and `docs-lint.yml` from `workflow_dispatch` back to `on: [push, pull_request]`.
