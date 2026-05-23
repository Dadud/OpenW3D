# OpenW3D Task Tracker

## Legend

- ✅ Done and verified
- 🔄 Done in source; CI or runtime pending
- ⬜ Not started / follow-up

---

## Track A — Build system

- [x] A001: CMake migration
- [x] A002: Windows DX9 via `cmake/dx9.cmake`
- [x] A003: Linux build
- [x] A004: macOS — **untested**
- [x] A005: CI aligned with w3dhub (MSVC, MinGW x86/x64, **clang64**, SDL3) + fork BGFX jobs
  - `Linux x64` — upstream tools subset (no BGFX)
  - `Linux x64 BGFX` — submodules + `make linux-gcc-release64` + `ENABLE_BGFX_BACKEND=ON`
  - `MSVC x64 BGFX` — GENie + MSBuild bgfx before configure
- [x] A006: `cmake/bgfx.cmake` — **FATAL_ERROR** if BGFX enabled without real libs (no stub)
- [x] A007: Upstream merge on `dev` (SDL3/LLVM/warnings + fork BGFX)

---

## Track B — Audio

- [x] B001–B006: OpenAL via `find_package` (upstream pattern post-merge)

---

## Track C — Renderer / BGFX

### Infrastructure (source-complete)

- [x] C001–C004: WW3DBackend, DX8, Null, BGFX skeleton
- [x] C005–C010: Shaders, materials, textures, lights (see prior checklist)
- [x] C011: Texture/VB/IB cache invalidation hooks on `BGFXBackend`
- [x] C012: Upstream platform layer merged (`dev` @ merge commit)

### Phase 2 — Windows proof-of-life 🔄

Validation ladder (manual; requires retail `.mix` in `Run/`):

1. `.\scripts\verify-bgfx-build.ps1` — libs + shader `.bin` present
2. Configure MSVC + `-DENABLE_BGFX_BACKEND=ON`, build `renegade`
3. Launch — BGFX init logs, no stub link
4. Main menu or empty level shows 3D
5. Clean exit

- [ ] C020: Interactive Windows session **verified** (document commit + date when done)
- [ ] C021: Audit #116-class issues (device list, VB/IB upload sizes, program handles)

### Phase 3 — Render parity ⬜

- [ ] C030: Sorting / translucency paths via `dx8wrapper` deferred dispatch
- [ ] C031: Ubershader variant coverage in gameplay scenes
- [ ] C032: DXT / managed texture reload
- [ ] C033: Fog, alpha test, skin/HLOD
- [ ] C034: Optional #131 staging/null guards (cherry-pick only if gaps found)

### Phase 4 — Linux BGFX + headless 🔄

- [x] C040: Linux CI BGFX job configured
- [ ] C041: Manual Linux smoke with Vulkan (if hardware available)
- [x] C042: FDS / headless via `NullBackend` + `W3D_ALLOW_MISSING_SDL3` (documented in BUILD.md)

### Phase 4b — SDL runtime window

- [x] C050: `pr/128` delta ported — `SDL_CreateWindow`, HWND from SDL props, `SDL3_Pump_Events`
- [ ] C051: Verify on **MinGW x64 SDL3** CI row with `-DW3D_BUILD_OPTION_SDL3=ON`

---

## Track D — Media

- [x] D001–D006: FFmpeg / Bink options per upstream defaults (FFmpeg OFF on Windows by default)

---

## Upstream engagement

Fork-only until BGFX playable on Windows (C020). Then consider upstream PRs for non-renderer fixes only.

---

## What's needed to land (fork)

### Must

1. Green CI on `dev` (all matrix jobs including BGFX extensions)
2. C020 Windows interactive proof-of-life
3. Shader `.bin` artifacts in CI Linux/Windows BGFX jobs

### Should

4. C030+ render parity for skirmish-quality visuals
5. Networking (Track E) — separate milestone
