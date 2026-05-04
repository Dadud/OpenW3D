# OpenW3D — OpenClaw Handoff Briefing

**From:** Hermes Agent (Dadud's setup)  
**Date:** 2026-05-04  
**Repo:** `https://github.com/Dadud/OpenW3D` (fork of `w3dhub/OpenW3D`)

---

## What Is This Project

OpenW3D is a cross-platform modernization of the Command & Conquer: Renegade game engine. Forked from EA's 2002 source release. The goal is to replace the original Windows/DirectX 8-only codebase with modern cross-platform support (Linux, macOS, Windows) using BGFX as the primary renderer backend.

**You do NOT need game assets to build.** The code compiles, links, and runs — it just shows placeholder textures without retail `.mix` files.

---

## Recent Commits (most recent first)

```
68f6e85 docs: add README, BUILD.md, ARCHITECTURE.md, CONTRIBUTING.md
a5dc26a feat(cmake): split Win32/DX8 sources from portable CMakeLists
75ed2c0 docs: archive original EA README, GPL license, and DX8 internal docs
```

These are on `dadud/main` — pushed to `https://github.com/Dadud/OpenW3D`.

---

## What Was Done

### Documentation (this session)
- **README.md** — rewritten from scratch. Covers: project overview, build status table, quick start, library structure, renderer backends
- **docs/BUILD.md** — full build guide (Linux NullBackend, BGFX/Vulkan, BGFX/GL; Windows MSVC/DX9, MinGW/BGFX, MSVC/BGFX; CMake options table; shader compilation; troubleshooting)
- **docs/ARCHITECTURE.md** — all library descriptions (WWMath, wwutil, wwdebug, wwbitpack, wwlib, wwui, ww3d2, WWAudio, wwnet, wwphys, Combat), subsystem tables, data flow, platform status, threading model
- **docs/CONTRIBUTING.md** — contributor guide with PR workflow, testing expectations, code style
- **docs/history/** — original EA README, GPL license, DX8 internal docs (DX8 Status, DX8 Rationalizations, State Management, Render Object Guide) archived here

### Linux/BGFX Build (previous session, committed as a5dc26a)
- DX8/Win32 source files split out of portable CMakeLists (`ww3d2`, `wwlib`)
- Cross-platform guards (`#if defined(_WIN32)`) added throughout
- `ww3d_platform.h` — 742-line D3D9 type stub header for non-Windows builds
- `d3d9.h` — redirects to `ww3d_platform.h` on non-Windows
- `cmake/dx9.cmake` — removed DXVK dependency for Linux; BGFX handles Vulkan now
- Cross-platform fixes: `clock_gettime` fallback for `timeGetTime`, `_lrotl` for non-Windows CRC, Registry stubs, OutputDebugString guards, MixFile file operations
- `ShaderVariantCache.cpp` — removed D3D9 renderer case
- `ShaderKey.h` — added `<cstdint>`, `<cstring>` includes

### Gap Analysis (done, not committed)
A full gap analysis was performed comparing OpenW3D to Spring RTS, Warzone 2100, and OpenRA. Key findings:

**Critical (must have):**
1. Unit tests — zero test coverage. `Code/Tests/` has old `.dsp` files not wired into CMake.
2. Dependency management — `vcpkg.json` present but inactive
3. Documentation — was missing (now mostly addressed above)
4. Asset pipeline — no extraction/build tooling for game assets
5. Shader pipeline — `shaderc` integration needed for `.sc` → `.bin` compilation
6. Modding API — no Lua/scripting interface
7. AI system — no AI interface

**Missing (should have):**
8. Linux CI incomplete — only builds libs, not full targets
9. No `-Werror` on Linux CI
10. No static analysis (clang-tidy)
11. No format enforcement (clang-format)
12. No sanitizers in CI
13. Networking — `select()` needs epoll/IOCP for 64+ players
14. Replay system — none
15. Logging framework — `wwdebug` exists but no structured logging
16. Crash reporting — none
17. Release process / version tags — none

Full analysis is in this conversation thread (search: "OpenW3D Gap Analysis").

---

## Current Branch State

```
dadud/main — your fork
├── 68f6e85 (HEAD) — docs added this session
├── a5dc26a — Linux/BGFX cmake split
└── 75ed2c0 — historical docs archived

remotes/dadud/main — same as above (pushed)
remotes/origin/main — upstream w3dhub/main (294 commits ahead of electronicarts)
```

---

## Build Status

| Target | Linux | Windows |
|--------|-------|---------|
| wwmath | ✅ | ✅ |
| wwutil | ✅ | ✅ |
| wwdebug | ✅ | ✅ |
| wwbitpack | ✅ | ✅ |
| wwlib | ✅ | ✅ |
| ww3d2 | ✅ (with BGFX) | ✅ (DX9 or BGFX) |
| renegade | ✅ (with BGFX) | ✅ |
| ww3d2e | ✅ | ✅ |
| WWAudio | ✅ | ✅ |

**Known issue:** `ww3d2` Linux build requires `ENABLE_BGFX_BACKEND=ON` and `external/bgfx/bx/bimg` submodules initialized.

---

## Key File Locations

| File | Purpose |
|------|---------|
| `README.md` | Project overview |
| `docs/BUILD.md` | Build guide |
| `docs/ARCHITECTURE.md` | Code layout and architecture |
| `docs/CONTRIBUTING.md` | Contributor guide |
| `docs/TASKS.md` | Internal task tracking (A–I tracks) |
| `docs/C005-C010-ANALYSIS.md` | Renderer backend implementation analysis |
| `docs/history/` | Archived EA original docs |
| `Code/ww3d2/ww3d_platform.h` | D3D9 type stubs for Linux |
| `Code/ww3d2/backends/bgfx/` | BGFX backend implementation |

---

## How to Build

```bash
git clone https://github.com/Dadud/OpenW3D.git
cd OpenW3D
git submodule update --init --recursive

# Linux with BGFX
cmake -S . -B build -DENABLE_BGFX_BACKEND=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Linux headless (no GPU)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

---

## Recommended Next Steps (Priority Order)

1. **Linux CI full green** — `openw3d-linux` CI workflow only builds libs. Extend to build `ww3d2`, `renegade`, `ww3d2e` on every push. This is the single highest-value action.

2. **Unit tests** — `wwmath`, `wwutil`, `wwbitpack` are isolated static libs. Add GoogleTest, wire `Code/Tests/` into CMake, write tests for the static libs first.

3. **`-Werror` on Linux CI** — MSVC uses `/WX`. Add `-DCMAKE_CXX_FLAGS="-Werror"` to Linux CI cmake args. Fix all resulting warnings.

4. **Asset pipeline docs** — document how to extract/place retail `.mix` files. Currently only documented in BUILD.md briefly.

5. **clang-format + clang-tidy** — add `.clang-format` and a weekly CI job for static analysis.

6. **Networking (E001)** — replace `select()` in `wwnet/` with epoll (Linux) / IOCP (Win32) / kqueue (macOS). This is blocking multiplayer scaling.

---

## Conventions

- **Branch naming:** `fix/`, `feat/`, `refactor/`, `docs/`, `ci/`
- **PRs:** small, focused, playtested before requesting review. Community is skeptical of large/untested PRs.
- **Commits:** conventional format: `type(scope): short description`
- **Platform guards:** `#if defined(_WIN32)` / `#if defined(__linux__)` / `#if defined(__APPLE__)`
- **Backend guards:** `#ifdef ENABLE_BGFX_BACKEND`
- **No `-Werror` currently on Linux** — it will surface many implicit-cast and signed/unsigned warnings

---

## GitHub Tokens / Access

- GitHub PAT stored at `~/.github_token`
- Push freely to your own fork (`Dadud/OpenW3D`)
- Draft PRs to upstream (`w3dhub/OpenW3D`) only after CI green and playtesting
- CI on `dadud/main` is the proxy for build correctness

---

## Discord / Contacts

The W3DHub community coordinates on Discord. Contributors: `madebr`, `OmniBlade`, `rm5248`, `caseychaos1212`. Community is small, skeptical of agent-generated PRs — small focused PRs with in-game testing are the exception.

---

## What NOT To Do

- Do NOT enable WiFi encryption on the OpenWrt router without explicit permission from Dadud
- Do NOT click "Turn Starlink Config On" in the Starlink app — it clobbers `cake-autorate` shaper settings
- Do NOT post upstream PRs without Dadud's explicit approval
- Do NOT assume the context window — the user (Dadud) prefers concise responses and direct SSH execution without being asked
