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
  - 7 matrix entries: MSVC x86/x64, MinGW x86/x64, MinGW x64 SDL3, Linux x64, Linux x64 BGFX
  - Linux jobs: expanded to build ww3d2, ww3d2e, renegade + submodules recursive
  - Linux BGFX variant: builds bgfx library then compiles with `-DENABLE_BGFX_BACKEND=ON`
  - Submodule init: `actions/checkout` with `submodules: recursive` for all jobs

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

### Phase 2: Modernization (source-complete)

- [x] C005: `ShaderKey` + `ShaderVariantCache::GetOrCreate()` — ⚠️ **runtime needs compiled `.bin` shaders** (see Shader Pipeline below)
- [x] C006: `BGFXMaterialMapper::Shader_To_BGFX_State()` wired into `Apply_Shader_State()`
- [x] C007: `Create_FrameBuffer()` + `m_frameBuffer`/`m_activeFrameBuffer`
- [x] C008: `DXT1/2/3/4/5` → `BC1/2/3` texture format mapping
- [x] C009: `Make_Layout()` — FVF enum → `bgfx::VertexLayout`
- [x] C010: `Apply_Light_Environment_State()` — ambient + 4 directional/point lights as uniforms

### Shader Pipeline ⚠️

The `.sc` shader source files exist in `Code/ww3d2/backends/bgfx/shaders/source/`
(vs_uber.sc, fs_uber.sc, vs_mesh.sc, fs_mesh.sc) and the CMake rules in
`Code/ww3d2/backends/bgfx/CMakeLists.txt` compile them via shaderc.

**Prerequisites for compilation:**
1. `git submodule update --init --recursive` — fetches bgfx/bx/bimg
2. `make linux-gcc-release64` in `external/bgfx/` — builds bgfx library + shaderc tool
3. The compiled `.bin` files land in `build/shaders/` at build time

**Runtime:** `ShaderVariantCache::GetShaderPath()` currently hardcodes `shaders/d3d11/vs_uber.bin`.
This path must match where CMake copies the compiled binaries — verified automatically
when the build runs successfully.

---

## Track D — Audio/Video decoupling

- [x] D001: FFmpegFile — `Code/wwlib/FFmpegFile.h`
- [x] D002: `fix/d002-broken-linkage` branch restores `openw3d.cpp` deleted by PR #103
- [x] D003: PR #103 FFmpegFile changes (`Set_Frame_Callback`, `Set_User_Data`) verified clean
- [x] D004: Bink **OFF** by default — `W3D_BUILD_OPTION_BINK=OFF`
- [x] D005: FFmpeg **ON** by default — `W3D_BUILD_OPTION_FFMPEG=ON`
- [x] D006: `tests/media/test_pattern.mp4` (5s H.264+AAC FFmpeg lavfi, CC0) + `tests/test_media.cpp` smoke test

---

## Track E — Networking (not started)

Research completed. Key findings:
- Raw `select()`-based I/O — breaks at ~50 connections; needs epoll/IOCP/kqueue
- Server-authoritative model already in `cnetwork.cpp` — correct for 64+ player scale
- No NAT traversal — needs coturn/ICE for symmetric NAT
- No modern lobby — WOL/GameSpy protocols defunct
- Bandwidth math: 64 players × 20 Hz × ~200 bytes ≈ 1–2 MB/s server uplink

Needed:
- E001: Replace `select()` I/O with epoll (Linux) / IOCP (Win32) / kqueue (macOS)
- E002: Integrate ENet or GameNetworkingSockets for UDP transport
- E003: coturn relay for NAT traversal
- E004: REST/WebSocket lobby service (even minimal)

---

## Track F — Physics (not started)

- `Code/wwphys/` is a custom implementation. No known issues but unaudited.
- Consider migrating to Bullet Physics (LGPL, widely used in open-source games)

---

## Track G — AI (not started)

- No behavior tree / GOAP system identified
- A* pathfinding not confirmed present
- Needed for single-player depth

---

## Track H — Mod/Asset System (not started)

- No formal mod package format
- No asset override/precedence system
- No workshop integration path

---

## Track I — Documentation (not started)

- README is the original C&C Renegade README
- No architecture docs, no build guide for Linux, no contributing guide

---

## Branches

| Branch | Description |
|--------|-------------|
| `upstream/main` | W3DHub main — base reference |
| `origin/feature/bgfx-backend-v2` | BGFX on backend abstraction — C001–C010 + OpenAL wiring |
| `origin/fix/d002-broken-linkage` | `upstream/main` + `openw3d.cpp` restored |
| `origin/feature/d006-media-test` | Media test + `test_pattern.mp4` |
| `pr-103-openal` | OmniBlade OpenAL PR — D002 broken, D003 verified |

---

## What's needed to land

### Must (blocks playability)

1. **Shader compilation** — `shaders/d3d11/vs_uber.bin` / `fs_uber.bin` must be compiled at build time.
   CI now pre-builds bgfx on the Linux BGFX job, which runs shaderc automatically.
   **Verify**: run the Linux BGFX CI job and confirm `build/shaders/` contains `.bin` files.
2. **D002** — Merge `fix/d002-broken-linkage` into the PR or re-add `openw3d.cpp` to PR #103
3. **Compile test** — No build machine available; CI on `dadud/main` is the proxy for correctness

### Should (blocks multiplayer)

4. **E001 select() → epoll/IOCP** — 64+ players won't work with current I/O model
5. **Lobby rewrite** — WOL/GameSpy defunct; users cannot connect to servers without a replacement

### Could (quality of life)

6. **macOS testing** — A004/B006 untested
7. **SDL3 Linux input** — `W3D_BUILD_OPTION_SDL3` is Windows-only; Linux client has no input abstraction
8. **Physics audit** — verify `wwphys` correctness or migrate to Bullet
9. **AI** — behavior tree, A* pathfinding
10. **Mod system** — mod packages, asset override, workshop integration
