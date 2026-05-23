# Building OpenW3D

## Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| CMake | 3.25 | 3.28+ |
| Compiler | C++20 | GCC 13, Clang 18, MSVC 17.10 |
| Git | 2.40+ | with submodules for BGFX |
| Game data | C&C Renegade retail `.mix` files | Latest patch (v1.037) |

### Linux

```bash
sudo apt install build-essential cmake ninja-build pkg-config
sudo apt install libasound2-dev libpulse-dev libopenal-dev
sudo apt install libavcodec-dev libavformat-dev libswscale-dev libavutil-dev libswresample-dev
sudo apt install libsdl3-dev libvulkan-dev libgl-dev libx11-dev libxrandr-dev libxcursor-dev
```

### macOS

```bash
brew install cmake ninja pkg-config ffmpeg sdl3 vulkan-loader
```

### Windows

- **MSVC**: Visual Studio 2022 17.x with "Desktop development with C++"
- **MSYS2**: `pacman -S mingw-w64-x86_64-{toolchain,cmake,ninja,ffmpeg,pkg-config}`  
  Also install **clang64** env for the LLVM matrix job: `pacman -S mingw-w64-clang-x86_64-toolchain`
- **SDL3 on Windows**: use [setup-sdl](https://github.com/libsdl-org/setup-sdl) in CI, or `-DW3D_BUILD_OPTION_SDL3=ON` with SDL3 installed

---

## Submodules

Required for BGFX builds only:

```bash
git submodule update --init --recursive
```

---

## Standard builds (matches CI)

These mirror [`.github/workflows/openw3d.yml`](../.github/workflows/openw3d.yml).

### MSVC x86 / x64

```powershell
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### MSYS2 MinGW x64 / x86 / clang64

```bash
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release -DW3D_BUILD_OPTION_FFMPEG=ON
cmake --build build --parallel --target combat combate renegade renegadeserver ww3d2e wwaudioe wwphyse
```

### MinGW x64 + SDL3 (tools/input; runtime SDL window on fork)

```bash
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release -DW3D_BUILD_OPTION_FFMPEG=ON -DW3D_BUILD_OPTION_SDL3=ON
cmake --build build --parallel --target renegade
```

### Linux x64 (SDL3, no BGFX)

```bash
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel --target scripts bandtest wwsaveload wwmath wwutil wwdebug wwbitpack wwtranslatedb
```

### Qt tools (WWConfigQt, x64 Windows)

```powershell
cmake --preset windows-qt -DW3D_BUILD_QT_TOOLS=ON
cmake --build --preset windows-qt
```

---

## BGFX builds (fork renderer track)

`ENABLE_BGFX_BACKEND=ON` requires **prebuilt** `external/bgfx` libraries. CMake will **fail** if they are missing (no stub link).

### Linux — BGFX + Vulkan (CI: `Linux x64 BGFX`)

```bash
git submodule update --init --recursive
cd external/bgfx && make linux-gcc-release64 && cd ../..
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release -DENABLE_BGFX_BACKEND=ON
cmake --build build --parallel --target renegade ww3d2
```

### Windows — MSVC + BGFX (CI: `MSVC x64 BGFX`)

```powershell
git submodule update --init --recursive
cd external\bgfx
..\bx\tools\bin\windows\genie.exe vs2022
msbuild .build\projects\vs2022\bgfx.sln /p:Configuration=Release /p:Platform=x64 /m
cd ..\..
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release -DENABLE_BGFX_BACKEND=ON
cmake --build build --parallel --target renegade ww3d2
```

Verify before running:

```powershell
.\scripts\verify-bgfx-build.ps1 -BuildDir build
```

### Windows — MinGW + BGFX

Same as MSVC after building bgfx with the VS toolchain (GENie output is shared under `external/bgfx/.build/`).

### Linux — headless / FDS (NullBackend)

```bash
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release -DW3D_ALLOW_MISSING_SDL3=ON
cmake --build build --parallel --target renegadeserver
```

No GPU required; WW3D uses `NullBackend` when BGFX and DX9 are off.

---

## SDL3: tools vs runtime window

| Layer | Status |
|-------|--------|
| CMake + `find_package(SDL3)` | Upstream-aligned; `W3D_ALLOW_MISSING_SDL3` for local headless only |
| Tools / Linux client input | `W3D_BUILD_OPTION_SDL3=ON` (default on non-Windows) |
| Windows runtime `SDL_CreateWindow` | Fork: `OPENW3D_SDL3` path in `WINMAIN.CPP` + `SDL3_Pump_Events` in `msgloop.cpp` |

---

## CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_BGFX_BACKEND` | OFF (Win), ON (Linux dep.) | BGFX renderer; requires built `external/bgfx` |
| `WANT_DX9` | ON when BGFX off (Win) | Native DX9 backend |
| `W3D_BUILD_OPTION_SDL3` | OFF (Win), ON (else) | SDL3 window/input |
| `W3D_ALLOW_MISSING_SDL3` | OFF | Headless configure without SDL3 (not for CI) |
| `W3D_BUILD_OPTION_FFMPEG` | OFF (Win), ON (else) | FFmpeg decode |
| `W3D_BUILD_OPTION_OPENAL` | ON when FFMPEG on | OpenAL audio |
| `W3D_BUILD_QT_TOOLS` | OFF | WWConfigQt |

---

## Output

```
build/             # Binaries and `build/shaders/*.bin` when BGFX enabled
Run/               # Deploy dir for game data + executables
```

---

## Game data

Copy retail Renegade `.mix` files into `Run/`. Without them the client starts but assets are missing.

### Windows: one-command deploy folder

If you already have a v1.037 Renegade install:

```powershell
.\scripts\make-run-package.ps1 -InstallPath "C:\path\to\Renegade"
cd Run
.\Play-Renegade.bat
```

`-InstallPath` can be omitted when the game is under Steam’s `common\Renegade` (or similar). See [Run/README.md](../Run/README.md).

---

## Shader compilation (BGFX)

Shaders compile at OpenW3D build time via `shaderc` from the bgfx submodule. Rebuild bgfx after editing `.sc` files under `Code/ww3d2/backends/bgfx/shaders/source/`.

---

## Troubleshooting

### `BGFX library not found`

Build bgfx first (see BGFX sections above). Stub libraries are no longer created.

### `bx.h not found`

`git submodule update --init --recursive`

### SDL3 not found (local dev)

Install SDL3 or pass `-DW3D_ALLOW_MISSING_SDL3=ON` for headless-only work — never in CI.

### MSVC `_lrotl` linker errors

Use CMake 3.28+.
