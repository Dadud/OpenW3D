# External Dependencies

This directory contains git submodules for third-party libraries.

## Initial Setup

```bash
git submodule update --init --recursive
```

This will clone: `bgfx`, `bx`, `bimg`.

## Building BGFX

BGFX uses its own build system (genie). You must build it before enabling `ENABLE_BGFX_BACKEND`.

### Linux / macOS

```bash
cd external/bgfx
make linux-gcc-release64
# or: make osx-clang-release64
```

### Windows (MSYS2 / MinGW)

```bash
cd external/bgfx
..\..\bx\tools\bin\windows\genie.exe vs2022
# Open .build\projects\vs2022\bgfx.sln in Visual Studio
# Build the bgfx, bx, bimg projects
```

### Windows (Visual Studio directly)

```bash
cd external/bgfx
..\..\bx\tools\bin\windows\genie.exe vs2022
# Open .build\projects\vs2022\bgfx.sln in Visual Studio and build
```

## Building OpenW3D with BGFX

```bash
# Linux
mkdir build && cd build
cmake .. -DENABLE_BGFX_BACKEND=ON -DCMAKE_BUILD_TYPE=Release
make

# Windows (CMake + Visual Studio)
mkdir build && cd build
cmake .. -DENABLE_BGFX_BACKEND=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Shaders

When ENABLE_BGFX_BACKEND is on, CMake will automatically compile `.sc` shader sources
to `.bin` binaries using the shaderc tool (included in the bgfx submodule at
`external/bgfx/.build/*/bin/`).

Shader output goes to `build/shaders/` and is copied to the `Run/` directory at runtime.
