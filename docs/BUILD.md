# Building OpenW3D

## Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| CMake | 3.25 | 3.28+ |
| Compiler | C++20 | GCC 13, Clang 18, MSVC 17.10 |
| Git | 2.40+ | with `git lfs` for large files |
| Game data | C&C Renegade retail `.mix` files | Latest patch (v1.037) |

### Linux

```bash
# Install build tools and dependencies
sudo apt install build-essential cmake ninja-build pkg-config
sudo apt install libasound2-dev libpulse-dev    # OpenAL
sudo apt install libavcodec-dev libavformat-dev libswscale-dev libavutil-dev  # FFmpeg
sudo apt install libvulkan-dev                  # BGFX/Vulkan
```

### macOS

```bash
brew install cmake ninja pkg-config
brew install alsa-lib portaudio  # audio
brew install ffmpeg               # video
brew install vulkan-loader        # BGFX/Vulkan
```

### Windows

- Visual Studio 2022 17.x with "Desktop development with C++"
- OR MinGW-w64 (via MSYS2) with `pacman -S mingw-w64-x86_64-cmake ninja`

---

## Submodules

Fetch all submodules after cloning:

```bash
git submodule update --init --recursive
```

This pulls: `bgfx`, `bx`, `bimg` (the BGFX library stack).

---

## Build Variants

### 1. Linux — NullBackend (no GPU, headless server)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

No GPU required. Produces headless server binaries.

### 2. Linux — BGFX with Vulkan

```bash
cmake -S . -B build \
  -DENABLE_BGFX_BACKEND=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DW3D_BUILD_OPTION_OPENAL=ON \
  -DW3D_BUILD_OPTION_FFMPEG=ON
cmake --build build --parallel
```

Requires `libvulkan-dev` installed. BGFX will auto-select Vulkan as the default renderer on Linux.

### 3. Linux — BGFX with OpenGL (fallback)

```bash
cmake -S . -B build \
  -DENABLE_BGFX_BACKEND=ON \
  -DBGFX_BACKEND=gl \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### 4. Windows — MSVC + DX9 native

```powershell
cmake -S . -B build `
  -G "Visual Studio 17 2022" `
  -Ax64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DW3D_BUILD_OPTION_FFMPEG=ON
cmake --build build --parallel
```

### 5. Windows — MinGW + BGFX

```bash
# In MSYS2 mingw64 shell
cmake -S . -B build `
  -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DENABLE_BGFX_BACKEND=ON `
  -DW3D_BUILD_OPTION_FFMPEG=ON `
  -DW3D_BUILD_OPTION_SDL3=ON
cmake --build build --parallel
```

### 6. Windows — MSVC + BGFX

```powershell
cmake -S . -B build `
  -G "Visual Studio 17 2022" `
  -Ax64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DENABLE_BGFX_BACKEND=ON `
  -DW3D_BUILD_OPTION_FFMPEG=ON
cmake --build build --parallel
```

---

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Release` | `Debug`, `Release`, `RelWithDebInfo` |
| `ENABLE_BGFX_BACKEND` | `OFF` (Win), `ON` (Linux) | Enable BGFX renderer |
| `W3D_BUILD_OPTION_FFMPEG` | `ON` | FFmpeg for video/audio decode |
| `W3D_BUILD_OPTION_OPENAL` | `ON` (non-Win), `OFF` (Win) | OpenAL spatial audio |
| `W3D_BUILD_OPTION_SDL3` | `OFF` (Win only) | SDL3 for input/windowing |
| `W3D_BUILD_OPTION_WEBBROWSER` | `ON` (Win only) | Embedded browser (requires IE) |
| `W3D_TOOLS` | `ON` | Build mod tools (LevelEdit, etc.) |
| `W3D_CLIENT` | `ON` | Build game client |
| `W3D_FDS` | `ON` | Build free dedicated server |

---

## Output

Build artifacts land in:

```
build/             # Libraries and intermediates
Run/               # Final binaries (renegade.exe, combat.exe, etc.)
build/shaders/     # Compiled BGFX shaders (.bin)
```

---

## Game Data

OpenW3D needs retail C&C Renegade `.mix` data files. Copy them from your installation:

```
# Linux/macOS
cp /path/to/Renegade/*.mix  Run/

# Windows — from the Run/ directory in this repo
copy "C:\Program Files\EA Games\Command & Conquer Renegade\*.mix"
```

Without `.mix` files, the engine will start but display missing-asset placeholders.

---

## Shader Compilation (BGFX)

BGFX shaders (`.sc` source files in `Code/ww3d2/backends/bgfx/shaders/source/`) are compiled to `.bin` at build time via the `shaderc` tool, built as part of the BGFX library.

To rebuild shaders after modifying `.sc` files:

```bash
# Build bgfx + shaderc first
cd external/bgfx
make linux-gcc-release64   # or: make win64-vs2019

# Then rebuild OpenW3D shaders
cmake --build build --target shaderc  # if available
cmake --build build                     # triggers shader rules
```

The compiled shaders land in `build/shaders/` and are loaded at runtime by `ShaderVariantCache`.

---

## Troubleshooting

### `bx.h not found`

```bash
git submodule update --init --recursive
```

### BGFX fails to build on Linux

```bash
sudo apt install libvulkan-dev glslang-dev
cd external/bgfx && make linux-gcc-release64
```

### MSVC linker errors about `_lrotl`

Update to CMake 3.28+. Older versions had a bug where `_lrotl` wasn't found in the CRT on Windows.

### WWDEBUG asserts fire in release build

Ensure `CMAKE_BUILD_TYPE` is exactly `Release` — the `RelWithDebInfo` configuration still enables some debug checks.

### "Missing DirectX SDK" on Windows

The DX9 SDK is fetched automatically via CMake FetchContent on MSVC. MinGW requires the system DirectX SDK packages from MSYS2 (`mingw-w64-x86_64-dx9-sdk`).
