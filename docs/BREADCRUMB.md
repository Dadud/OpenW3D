# OpenW3D Session Breadcrumb — 2026-05-04

## Session Summary

Reviewed `w3dhub/OpenW3D` against comparable projects (Spring RTS, Warzone 2100, OpenRA). Identified critical gaps: zero unit tests, no documentation, no modding API, broken Linux CI, no static analysis, no format enforcement. Did documentation-first approach since it's prerequisite for contributor onboarding.

## Actions Taken

### Commits on `dadud/main`

| Commit | What |
|--------|-------|
| `75ed2c0` | Archived original EA README, GPL license, DX8 internal docs → `docs/history/` |
| `a5dc26a` | Linux/BGFX cmake split: DX8/Win32 source file isolation, cross-platform guards, `ww3d_platform.h` (D3D stubs), `d3d9.h` redirect, cmake/dx9.cmake cleanup |
| `68f6e85` | New docs: README.md, BUILD.md, ARCHITECTURE.md, CONTRIBUTING.md |

## What's Still Outstanding

1. **Unit tests** — zero test coverage. `Code/Tests/` has old `.dsp` files, not wired into CMake.
2. **Linux CI incomplete** — only builds libs, not full targets (`ww3d2`, `renegade`, `ww3d2e`).
3. **`-Werror` on Linux** — not enforced, many warnings to fix.
4. **clang-format + clang-tidy** — not set up.
5. **Networking** — `select()` limits 64-player scaling.
6. **Asset pipeline** — no extraction tooling docs beyond BUILD.md brief mention.
7. **Shader compilation** — shaderc integration needs verification in CI.
8. **Modding API / AI** — not started.

Full gap analysis: search "OpenW3D Gap Analysis" in conversation.

## Files Changed This Session

```
README.md          (rewritten)
docs/BUILD.md      (new)
docs/ARCHITECTURE.md (new)
docs/CONTRIBUTING.md (new)
docs/history/README.original.md (new)
docs/history/LICENSE.original.md (new)
docs/history/dx8-*.txt (new)
docs/history/state-management.txt (new)
docs/history/render-object-guide.txt (new)
Code/ww3d2/CMakeLists.txt
Code/ww3d2/dx8wrapper.cpp
Code/ww3d2/dx8wrapper.h
Code/ww3d2/ww3d_platform.h (new)
Code/ww3d2/d3d9.h (new)
Code/wwlib/CMakeLists.txt
Code/wwlib/crc.cpp
Code/wwlib/registry.cpp
Code/wwlib/registry.h
Code/wwlib/ini.cpp
Code/wwlib/mixfile.cpp
Code/wwlib/mpu.cpp
cmake/dx9.cmake
```

## Key Context for OpenClaw

- **Repo:** `https://github.com/Dadud/OpenW3D`
- **Branch:** `dadud/main` (pushed)
- **Build:** `cmake -S . -B build -DENABLE_BGFX_BACKEND=ON && cmake --build build --parallel`
- **vcpkg.json** present but inactive
- **BGFX submodules** need `git submodule update --init --recursive`
- **Community:** small, skeptical of agent PRs, coordinates on Discord
- **PR norm:** small focused PRs, playtested, CI green before review request
