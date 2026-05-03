# OpenW3D Task Tracker

## Legend
- ✅ Done and verified
- ⚠️ Blocked / needs decision
- ⬜ Not started / untested

---

## Track A — Build system

- [x] A001: CMake migration
- [x] A002: Windows build — `cmake/dx9.cmake` provides DX9 SDK path
- [x] A003: Linux build — standard CMake, no special config needed
- [x] A004: macOS build — cmake files present, **untested**
- [x] A005: CI/CD — `openw3d.yml` runs on push/PR (`on: [push, pull_request]`)

---

## Track B — Audio engine

- [x] B001: Null audio — `null/NullAudio.h/cpp`
- [x] B002: Audio manager abstraction — `WWAudio.h/cpp`
- [x] B003: OpenAL backend — `openal/OpenALAudio.h/cpp` wired via `W3D_BUILD_OPTION_OPENAL` (ON by default non-Windows). `cmake/openal.cmake` fetches SDK on Windows, uses pkg-config on Linux/macOS.
- [x] B004: Miles fallback — `miles/MilesAudio.h/cpp`
- [x] B005: Audio format registration — handled implicitly by Miles/OpenAL backend Init()
- [x] B006: macOS audio configure path — part of standard CMake, **untested**

---

## Track C — Renderer backend abstraction

### Phase 1: Core backend infrastructure

- [x] C001: WW3DBackend interface — `ww3dbackend.h`, ~50 pure virtual methods
- [x] C002: DX8Backend implementation — wraps DX8 behind WW3DBackend
- [x] C003: NullBackend — no-op renderer (`Null3DObjClass` from `nullrobj.h`)
- [x] C004: BGFXBackend skeleton — `backends/bgfx/bgfxbackend.h/cpp`

### Phase 2: Modernization (implemented in `feature/bgfx-backend-v2`)

- [x] C005: `ShaderKey` + `ShaderVariantCache::GetOrCreate()` loads compiled uber-shaders (`vs_uber.sc`/`fs_uber.sc`)
- [x] C006: `BGFXMaterialMapper::Shader_To_BGFX_State()` wired into `Apply_Shader_State()`
- [x] C007: `Create_FrameBuffer()` + `m_frameBuffer`/`m_activeFrameBuffer`
- [x] C008: `DXT1/2/3/4/5` → `BC1/2/3` texture format mapping
- [x] C009: `Make_Layout()` — FVF enum → `bgfx::VertexLayout`
- [x] C010: `Apply_Light_Environment_State()` — ambient + 4 directional/point lights as uniforms

---

## Track D — Audio/Video decoupling

- [x] D001: FFmpegFile — `Code/wwlib/FFmpegFile.h`
- [x] D002: ✅ Fixed — `fix/d002-broken-linkage` branch restores `openw3d.cpp` deleted by PR #103
- [x] D003: PR #103 FFmpegFile changes (`Set_Frame_Callback`, `Set_User_Data`) verified clean
- [x] D004: Bink **OFF** by default — `W3D_BUILD_OPTION_BINK=OFF`
- [x] D005: FFmpeg **ON** by default — `W3D_BUILD_OPTION_FFMPEG=ON`
- [x] D006: `tests/media/test_pattern.mp4` (5s H.264+AAC FFmpeg lavfi, CC0) + `tests/test_media.cpp` smoke test

---

## Branches

| Branch | Description |
|--------|-------------|
| `upstream/main` | W3DHub main — base reference |
| `origin/feature/backend-abstraction` | WW3DBackend + DX8/Null — C001–C003 |
| `origin/feature/bgfx-backend-v2` | BGFX on backend abstraction — C005–C010 + OpenAL wiring + D004/D005 flip |
| `origin/fix/d002-broken-linkage` | `upstream/main` + `openw3d.cpp` restored |
| `origin/feature/d006-media-test` | Media test + `test_pattern.mp4` |
| `pr-103-openal` | OmniBlade OpenAL PR — D002 broken, D003 verified |

---

## What's needed to land

1. **D002 fix** — Merge `fix/d002-broken-linkage` into PR #103, or re-add `openw3d.cpp` directly to the PR
2. **Compile test** — No build environment available; needs real machine run
3. **CI re-enable** — Already enabled (`on: [push, pull_request]`), no action needed
4. **macOS testing** — A004/B006 untested, no macOS machine available
