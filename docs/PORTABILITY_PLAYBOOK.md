# OpenW3D Cross-Platform Portability Playbook

Goal: make OpenW3D compile, package, and eventually run on Windows, Linux, macOS, iOS, Android, and web/other ports without turning the tree into `#ifdef` soup.

This playbook is based on direct inspection of nearby W3D/SAGE projects and broader battle-tested C/C++ game ports.

## Reference repos inspected

Local reference clones live under `C:/Users/Dadud/projects/engine-reference/` unless noted.

| Repo | Why it matters | Best things to steal |
|---|---|---|
| `../Generals-Mac-iOS-iPad` | SAGE lineage, already pushed Generals to Linux/macOS/iOS using SDL3 + DXVK/MoltenVK | SDL3 entry/input patterns, iOS packaging, DXVK native build recipe, asset root fixes, portability diary |
| `Thyme` | W3D/SAGE-ish codebase with clean POSIX platform layer | pthread `ThreadClass`, POSIX `RawFileClass`, non-Windows `cpudetect`, platform file split |
| `fbraz3-dxvk` | DXVK 2.6 fork pinned by Generals iOS port | ARM-capable D3D8/D3D9 native DXVK path; use instead of current DXVK 3.x for Android/iOS experiments |
| `OpenRCT2` | Modern C++ game with desktop + Android + Emscripten packaging | Android Gradle layout, CMake platform helpers, AppImage/XDG/project structure |
| `OpenTTD` | Best release/CI packaging discipline | Option taxonomy, FHS install, dependency packaging, release workflows |
| `ScummVM` | Widest platform support: Android, iOS/tvOS, Switch, Vita, 3DS, Emscripten, POSIX | Filesystem factory model, Android SAF/JNI, per-platform backends, many packaging lessons |
| `SDL` | Canonical Android/iOS/desktop abstraction library | `SDLActivity`, Android Gradle/CMake example, CMake APK helper functions, Java glue |
| `ioquake3` | Old C game ported cleanly across desktop/web | simple `sys_*` split, SDL dynamic loading, XDG paths, macOS signing docs |
| `xray-16` | Large legacy D3D-ish engine with platform header split | `Platform.hpp` + `PlatformLinux.inl`/`PlatformApple.inl`, `_splitpath`, path separator helpers |
| `OpenSAGE` | Modern C# rewrite | Data/renderer design reference only; not directly backportable |

## Core architecture rule

Do **not** scatter platform checks across gameplay/renderer code. Build a small platform layer and route old Win32 assumptions through it.

Recommended structure:

```text
Code/platform/
  platform.h              # platform detection + common typedefs
  file_posix.cpp
  file_win32.cpp
  thread_pthread.cpp
  thread_win32.cpp
  paths_android.cpp
  paths_ios.mm
  paths_macos.mm
  paths_linux.cpp
  dynlib_sdl.cpp           # prefer SDL_LoadObject where SDL exists
  dynlib_win32.cpp
```

Then gradually migrate old code:

- `wwlib/rawfile.*` → platform file backend
- `wwlib/thread.*` → platform thread backend
- `wwlib/cpudetect.*` → SDL CPU info + fallback OS/memory info
- `wwutil/ProcessManager.*` → platform process backend, stub where unsupported
- `Commando/WINMAIN.CPP` → platform entrypoint wrappers
- D3D dynamic loading → platform dynamic library wrappers

## Platform detection macros

Use explicit project macros, not raw OS/compiler macros everywhere.

Borrow pattern from `xray-16/src/Common/Platform.hpp`:

```cpp
#if defined(_WIN32)
  #define OPENW3D_PLATFORM_WINDOWS 1
#elif defined(__ANDROID__)
  #define OPENW3D_PLATFORM_ANDROID 1
  #define OPENW3D_PLATFORM_POSIX 1
#elif defined(__APPLE__)
  #include <TargetConditionals.h>
  #if TARGET_OS_IPHONE
    #define OPENW3D_PLATFORM_IOS 1
  #else
    #define OPENW3D_PLATFORM_MACOS 1
  #endif
  #define OPENW3D_PLATFORM_POSIX 1
#elif defined(__linux__)
  #define OPENW3D_PLATFORM_LINUX 1
  #define OPENW3D_PLATFORM_POSIX 1
#elif defined(__EMSCRIPTEN__)
  #define OPENW3D_PLATFORM_WEB 1
  #define OPENW3D_PLATFORM_POSIX 1
#else
  #error Unsupported platform
#endif
```

Keep old compatibility macros (`OPENW3D_WIN32`, `OPENW3D_SDL3`) during migration, but treat them as backend choices, not OS identity.

## Build system plan

### CMake option taxonomy

Borrow from OpenTTD `cmake/Options.cmake`: platform-neutral options with clear defaults.

Suggested top-level options:

```cmake
option(W3D_BUILD_TOOLS "Build editor/tool binaries" OFF)
option(W3D_BUILD_GAME "Build game executable/library" ON)
option(W3D_BUILD_DEDICATED "Build dedicated server only" OFF)
option(W3D_BUILD_TESTS "Build tests" OFF)

option(W3D_USE_SDL3 "Use SDL3 for window/input/platform glue" ON)
option(W3D_USE_OPENAL "Use OpenAL audio backend" ON)
option(W3D_USE_FFMPEG "Use FFmpeg video/audio decoding" ON)
option(W3D_USE_DXVK "Use native DXVK D3D translation" OFF)
option(W3D_USE_NATIVE_D3D9 "Use native Windows D3D9" OFF)
option(W3D_USE_STUB_RENDERER "Build with no renderer/stub renderer" OFF)

option(W3D_PACKAGE_DEPENDENCIES "Copy runtime dependencies beside executable" OFF)
option(W3D_INSTALL_FHS "Install using Linux FHS paths" OFF)
```

Important: avoid `cmake_dependent_option(... FORCE ...)` patterns that make cross-compiles impossible to override. Android/iOS first-pass builds need `-DW3D_USE_FFMPEG=OFF`, `-DW3D_USE_DXVK=OFF`, etc.

### Presets / toolchains

Add `CMakePresets.json` once the current configure is stable:

- `windows-mingw`
- `linux-sdl3-dxvk`
- `macos-sdl3-dxvk-moltenvk`
- `android-arm64-stub`
- `android-arm64-sdl3-dxvk`
- `ios-arm64-stub`
- `ios-arm64-sdl3-dxvk-moltenvk`
- `emscripten-tools-or-viewer` (later)

Reference patterns:

- SDL Android Gradle project: `SDL/android-project/app/build.gradle`
- Generals iOS Meson cross file: `Generals-Mac-iOS-iPad/cmake/meson-arm64-ios-cross.ini.in`
- OpenTTD release CMake: `OpenTTD/cmake/InstallAndPackage.cmake`

## Platform priorities

### Windows

Use native MinGW/MSVC path first.

- Keep native D3D9 for fastest confidence.
- Keep current OpenAL/FFmpeg work.
- Build tools only on Windows unless/until MFC/tool dependencies are isolated.
- Validate clean MinGW build after every platform-layer change.

### Linux

Primary non-Windows desktop target.

- SDL3 window/input.
- Native DXVK D3D9 via `libdxvk_d3d9.so`.
- OpenAL + FFmpeg.
- XDG paths.
- AppImage/Flatpak later.

Borrow:

- `ioquake3/code/sys/sys_unix.c` for XDG config/data/state split.
- `OpenTTD/cmake/InstallAndPackage.cmake` for FHS install layout.
- `Generals-Mac-iOS-iPad/docs/DEV_BLOG` for Linux asset-root lessons.

### macOS

Use SDL3 + DXVK native + MoltenVK first. Do **not** write a Metal renderer first.

Borrow:

- `Generals-Mac-iOS-iPad/cmake/dx8.cmake` Meson-driven DXVK source build.
- `Generals-Mac-iOS-iPad/docs/BUILD/MACOS.md` for MoltenVK deployment lessons.
- `ioquake3/docs/macos-codesign-notarization.txt` for signing/notarization packaging.

### iOS

Start stub renderer, then SDL3 + DXVK/MoltenVK.

Borrow:

- `Generals-Mac-iOS-iPad/ios/project.yml`
- `Generals-Mac-iOS-iPad/ios/Stub/main.m`
- `Generals-Mac-iOS-iPad/scripts/build/ios/package-ios-zh.sh`
- `Generals-Mac-iOS-iPad/Patches/dxvk-ios.patch`

Caveats:

- iOS cannot freely browse arbitrary install dirs. Assets need bundle resources or Documents/import flow.
- Dynamic library loading is constrained; package and sign frameworks correctly.
- `@executable_path/Frameworks/...` loader paths matter.

### Android

Start with stub/no renderer native `.so`, then SDL3 + DXVK native.

Use SDL’s Android project as the shell:

- `SDL/android-project/app/build.gradle`
- `SDL/android-project/app/src/main/AndroidManifest.xml`
- `SDL/android-project/app/src/main/java/org/libsdl/app/SDLActivity.java`

Use ScummVM for real Android storage lessons:

- `scummvm/backends/fs/android/*`
- SAF support for user-selected external asset dirs.
- JNI helpers for DPI, clipboard, storage locations, network state.

Renderer path:

- Use `fbraz3-dxvk` pinned fork (`46a3bc0`) as starting point, not current DXVK 3.x.
- Generate Meson Android cross file using NDK clang.
- Build `libdxvk_d3d9.so` for `arm64-v8a`.
- Package beside game `.so` in APK `lib/arm64-v8a/`.
- Set `DXVK_WSI_DRIVER=SDL3` before loading D3D9/DXVK.

### Web/Emscripten

Not urgent, but do not accidentally block it.

Use it as a portability forcing function:

- no threads unless explicitly enabled
- no arbitrary filesystem; use virtual FS/assets
- no dynamic libraries unless planned
- no Win32 APIs, ever

References:

- OpenRCT2 Emscripten build flags in `src/openrct2/CMakeLists.txt`
- OpenTTD `ci-emscripten.yml`
- ScummVM Emscripten FS/network backends
- ioquake3 `cmake/platforms/emscripten.cmake`

## Current OpenW3D blockers and reference fixes

| Blocker | Reference fix |
|---|---|
| `pthread_cancel` in GameSpy on Android | Do not build GameSpy for mobile. Long-term: replace with direct-IP/master-server shim. |
| `wwfile.h` no POSIX handle type | Use Thyme `src/w3d/lib/wwfile.h` + `rawfile.cpp` POSIX pattern. |
| `thread.cpp` ad-hoc pthread branch | Use Thyme `src/w3d/lib/thread.cpp/h` pthread implementation; remove Android-specific hacks. |
| `cpudetect.cpp` Windows/x86 assumptions | Use Thyme non-Windows `Init_Memory`, `Init_OS`, CPUID fallback. Use SDL CPU info where SDL exists. |
| `_splitpath`, `_MAX_DIR`, path separator junk | Use xray-16 `PlatformLinux.inl` / `PlatformApple.inl` helpers, or centralize in `platform/path.h`. |
| `msgloop.cpp` Win32 UI assumptions | Exclude on non-Windows or replace with SDL event pump. Do not stub HWND/HACCEL through the whole tree. |
| D3D9 dynamic load names | Use ioquake3-style `Sys_LoadLibrary` wrapper; on SDL platforms prefer `SDL_LoadObject`. |
| asset roots on packaged platforms | Use ScummVM FS factory + Generals asset-root override pattern. |
| Android external storage | Use ScummVM SAF model; do not rely on POSIX paths only. |
| macOS/iOS Vulkan/MoltenVK quirks | Use Generals DXVK/MoltenVK notes and `dxvk-ios.patch`. |

## Packaging matrix

| Platform | Artifact | Package dependencies? | Asset location |
|---|---|---|---|
| Windows | `.exe` + DLL folder | Yes | game install dir / user documents |
| Linux | executable + `.so`, AppImage/Flatpak later | Yes for portable bundles | XDG data dirs + override env/ini |
| macOS | `.app` bundle | Yes, Frameworks/ | app Resources + Application Support |
| iOS | `.app`/IPA | Frameworks signed inside bundle | bundled subset + Documents import |
| Android | APK/AAB | `.so` under `lib/<abi>` | assets/ for small files, app data/SAF for game data |
| Emscripten | `.html/.js/.wasm/.data` | bundled | virtual FS/preload |

## Immediate implementation sequence

1. Add `Code/platform/platform.h` and normalize platform macros.
2. Replace quick Android hacks in `thread.*`, `rawfile.*`, `cpudetect.*` with Thyme-derived POSIX branches.
3. Exclude GameSpy, Win32 UI, editors/tools from mobile configure.
4. Make Android `arm64-v8a` stub build compile to a static lib or `.so`.
5. Add SDL Android shell using SDL reference Gradle project.
6. Add `fbraz3-dxvk` Meson Android source build behind `W3D_USE_DXVK`.
7. Build `libdxvk_d3d9.so`; only then wire renderer startup.
8. Add Linux SDL3/DXVK validation path.
9. Add macOS MoltenVK path.
10. Add CI matrix: Windows MinGW, Linux stub/DXVK, Android stub.

## Rule of thumb

If a file needs more than two `#ifdef OPENW3D_PLATFORM_*` branches, it probably belongs in `Code/platform/`.

If a platform cannot implement a feature cleanly (process spawning, GameSpy, editors), make the feature optional and compile it out. Do not fake Win32 forever.
