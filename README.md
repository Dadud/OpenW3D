# OpenW3D

OpenW3D is a cross-platform modernization of the [Command & Conquer: Renegade](https://www.ea.com/games/command-and-conquer/command-and-conquer-the-ultimate-collection) game engine. It replaces the original DirectX 8 fixed-function rendering pipeline with a modern [BGFX](https://github.com/bkaradzic/bgfx) backend while preserving the original game's logic, assets, and multiplayer infrastructure.

This is a **modder's and engine-hacker's foundation** — not a drop-in replacement for the retail game. You need a legal copy of Command & Conquer: Renegade to use it.

## Build Status

| Platform | Backend | Status |
|----------|---------|--------|
| Windows (MSVC x64) | DX9 (native) | ✅ Builds |
| Windows (MinGW x64) | DX9 (native) | ✅ Builds |
| Windows (MinGW x64) | BGFX (Vulkan) | ✅ Builds |
| Linux (GCC/Clang) | NullBackend | ✅ Builds |
| Linux (GCC/Clang) | BGFX (Vulkan) | ✅ Builds |
| macOS | Any | ⚠️ Untested |

The original `electronicarts/CnC_Renegade` source release compiled only on Windows with Visual Studio 6. OpenW3D adds Linux and macOS support.

## Quick Start

### Prerequisites

- CMake 3.25+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 17.5+)
- A legal copy of Command & Conquer: Renegade (retail `.mix` data files)

### Clone

```bash
git clone https://github.com/w3dhub/OpenW3D.git
cd OpenW3D
git submodule update --init --recursive
```

### Build (Linux, BGFX/Vulkan)

```bash
cmake -S . -B build -DENABLE_BGFX_BACKEND=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Build (Windows, MSVC, DX9 native)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -Ax64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Build (Linux, NullBackend — no GPU required)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Run

Copy retail game data files (`.mix` files from the Renegade install directory) into the `Run/` directory, then run the `renegade` or `combat` binary from the build directory.

## Project Structure

### Libraries

| Directory | Purpose |
|-----------|---------|
| `Code/WWMath/` | Math: vectors, matrices, quaternions, planes, collision (AABB/OBB/sphere/ray), culling, splines |
| `Code/wwutil/` | Utilities: string manipulation, argument parsing, memory management, base64, heap, critsections |
| `Code/wwdebug/` | Debug: profiling (`wwprofile`), memory logging (`wwmemlog`), debug utils |
| `Code/wwbitpack/` | Bitstream packing/unpacking for network and file I/O |
| `Code/wwlib/` | Core I/O: file classes, INI parsing, MIX archive handling, CRC, chunk I/O, threading |
| `Code/wwsaveload/` | Save/load system for game state serialization |
| `Code/wwtranslatedb/` | INI-to-binary translation cache |
| `Code/wwui/` | UI framework: dialog system, controls (button, list, tree, edit, slider, etc.), input handling |
| `Code/WWAudio/` | Audio: sound system abstraction with OpenAL and Miles backend support |
| `Code/ww3d2/` | 3D engine: W3D file format parser (RenderWare R3), renderer, shader system, terrain, materials, animations |
| `Code/wwnet/` | Networking: packet management, socket wrappers (Win32/posix), LAN discovery, connection state machine |
| `Code/wwphys/` | Physics: collision detection, pathfinding (A*), vehicle physics (tracked, wheeled, motorcycle), projectile simulation |
| `Code/Combat/` | Game logic: weapons, combat, gameplay rules |
| `Code/Commando/` | Main game client |
| `Code/WWOnline/` | Matchmaking and online services |
| `Code/BandTest/` | Bink video decoder integration |
| `Code/BinkMovie/` | Movie playback subsystem |
| `Code/Scripts/` | Game scripts (INI-based logic) |

### Renderer Backends (`Code/ww3d2/backends/`)

| Backend | Description |
|---------|-------------|
| `bgfx/` | Modern cross-platform renderer using BGFX (Vulkan, D3D11/12, OpenGL, Metal). Primary target for new development. |
| `null/` | No-op renderer for headless servers and build verification |

The `DX8` backend (DirectX 8/9 wrapper) is Windows-only and present in `dx8caps.cpp`, `dx8renderer.cpp`, etc. It is not actively maintained.

### External Dependencies

Managed via CMake FetchContent or git submodules:

| Dependency | Purpose |
|------------|---------|
| `external/bgfx/` | Renderer abstraction library |
| `external/bx/` | BGFX utility library (required by bgfx) |
| `external/bimg/` | BGFX image processing (required by bgfx) |
| FFmpeg | Audio/video decoding via `W3D_BUILD_OPTION_FFMPEG=ON` |
| OpenAL | Spatial audio via `W3D_BUILD_OPTION_OPENAL=ON` (default on non-Windows) |
| SDL3 | Input and window management on Windows (`W3D_BUILD_OPTION_SDL3=ON`) |
| ICU4C | Unicode string handling via vcpkg or system ICU |

## Documentation

- [`docs/TASKS.md`](docs/TASKS.md) — Internal task tracking for the modernization effort
- [`docs/C005-C010-ANALYSIS.md`](docs/C005-C010-ANALYSIS.md) — Renderer backend implementation analysis
- [`docs/history/`](docs/history/) — Historical artifacts from the original EA source release

## Contributing

This project is a community fork. Before opening PRs, please read [`docs/CONTRIBUTING.md`](docs/CONTRIBUTING.md) (coming soon). Key expectations:

- **Small, focused PRs.** One logical change per PR. Large refactors or backend rewrites need discussion in an Issue first.
- **Playtesting before merge.** CI must be green and the change must be tested in-game where applicable.
- **Draft PRs for work-in-progress.** Use GitHub Draft PRs for incomplete features.
- **Community norms.** The team coordinates on Discord — see the repo description for the invite link.

## License

GPL v3 with additional terms. See [`LICENSE.md`](LICENSE.md) and [`docs/history/LICENSE.original.md`](docs/history/LICENSE.original.md).
