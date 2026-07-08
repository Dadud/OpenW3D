# AI Coding Agent Instructions

## Project Overview

This is the **OpenW3D** project — a community-driven effort to fix, modernize, and cross-platform Renegade. Renegade is a 2002 first-person shooter / third-person hybrid based on the Westwood 3D engine. The codebase is being modernized from a Win32/VC6/MSVC 6 baseline to CMake + MSVC 2022 (and MinGW for CI), and ported from Windows-only to run on Linux and macOS.

## Architecture

### Single source tree (no separate games)
Unlike sister projects (Generals, GeneralsMD) which have parallel `Generals/` and `GeneralsMD/` directories, Renegade has a single `Code/` tree.

### Engine layout
- `Code/ww3d2/` — W3D2 graphics engine (rendering pipeline, texture loader, sort renderer)
- `Code/wwphys/` — physics, terrain, vehicles
- `Code/wwlib/` — base library (registry, file I/O, threads, debug, etc.)
- `Code/wwui/` — UI library
- `Code/Combat/` — gameplay layer (soldiers, vehicles, buildings, scripts)
- `Code/Commando/` — game client + server (WinMain, dialogs, networking, save/load)
- `Code/Scripts/` — campaign/multiplayer level scripts (compiled into the engine binary, not a mod .dll)
- `Code/Tools/` — level editor, asset pipeline, etc. (MFC, Windows-only by default)
- `Code/Launcher/`, `Code/Installer/` — Windows installer/patcher (Windows-only)

### Cross-platform abstractions
- `Code/wwlib/win32_compat.h` — Win32→POSIX shim layer (Sleep, GetLastError, OutputDebugString, file APIs, mutex/event, library loading). No-op on Windows; POSIX-backed on non-Windows. Transitively included through `Code/dxvk_wrapper/dxvk_wrapper_compat.h`.
- `Code/wwlib/registryini.h/.cpp` — INI-file backend for the Win32 registry. Active on non-Windows; the real Win32 registry is used on Windows.
- `Code/wwlib/mutex.h` — `CriticalSectionClass` (std::recursive_mutex) and `FastCriticalSectionClass` (std::atomic_flag) replacements for the Win32 critical section.
- `Code/wwnet/socket_wrapper_posix.cpp` / `socket_wrapper_win32.cpp` — WinSock→BSD sockets abstraction, selected by CMake `if(WIN32)`.
- `Code/Combat/directinput_sdl3.cpp` — SDL3 backend for the DirectInput class API. CMake picks this on non-Windows instead of `Code/Combat/directinput.cpp` (the original DirectInput8 implementation).
- `Code/Combat/sdl3_window.h` — C-linkage accessors for the SDL3 window reference shared between `WINMAIN.CPP` and the DirectInput SDL3 backend.
- `Code/Commando/WINMAIN.CPP` — `Create_Main_Window()` and `Set_Working_Directory()` have `#if defined(OPENW3D_WIN32)` / `#elif defined(OPENW3D_SDL3)` branches for the same APIs.

### Build system
- `CMakeLists.txt` (top-level) + `Code/CMakeLists.txt` per subsystem
- `CMakePresets.json` — `win` (MSVC), `linux` (apt-installed deps), `linux-qt` (vcpkg + Qt tools), `windows-qt`
- `cmake/` modules — `ffmpeg.cmake`, `openal.cmake`, `sdl3.cmake`, `freetype.cmake`, `miles.cmake`, `bink.cmake`, `gamespy.cmake`, `dx9.cmake`, `FindFFmpeg.cmake`, `FindOpenAL.cmake`
- Backend options: `W3D_BUILD_OPTION_OPENAL` (or Miles fallback), `W3D_BUILD_OPTION_FFMPEG` (or Bink fallback), `W3D_BUILD_OPTION_SDL3`, `W3D_BUILD_OPTION_FREETYPE`, `W3D_BUILD_OPTION_ICU`, `W3D_BUILD_OPTION_MILES`, `W3D_BUILD_OPTION_BINK`, `W3D_BUILD_OPTION_WEBBROWSER`, `W3D_BUILD_QT_TOOLS`
- Audio backend selection: openal/ > miles/ > null/ (each falls through to the next in CMakeLists.txt)
- Video backend selection: ffmpeg > bink > null
- The `OPENW3D_WIN32` and `OPENW3D_SDL3` defines select which platform-specific code branches compile

## Development Workflow

### Code change documentation
**Every user-facing change requires an OpenW3D comment with this format:**
```cpp
// OpenW3D @keyword author DD/MM/YYYY Description
```

Common keywords: `@bugfix`, `@feature`, `@performance`, `@refactor`, `@tweak`, `@build`, `@info`, `@todo`

The `OpenW3D` word and `@keyword` are mandatory. `author` and date can be omitted.

**Example:**
```cpp
// OpenW3D @bugfix BenderAI 20/02/2026 TARGA.h long->int32_t 64-bit fix
```

For multi-line explanations, use the same format then a continuation block:
```cpp
// OpenW3D @bugfix author DD/MM/YYYY Short summary.
// The root cause is X. The fix replaces Y with Z because W.
// Multi-line blocks are welcome for non-obvious changes.
```

For backports from GeneralsX / GeneralsGameCode / TheSuperHackers, cite the source:
```cpp
// OpenW3D @refactor Backported from TheSuperHackers/GeneralsGameCode
// commit 98f1db9929 (SortingRenderer merge, June 2026).
```

### Commit message format
**Conventional commits** with extended types. The format is:
```
type(scope): Description starting with action verb
```

Allowed types (extended set modeled on TheSuperHackers/GeneralsGameCode):
```
bugfix:   Fixes a user-facing bug
build:    Addresses a compile warning or error
chore:    Maintenance work with no user-facing impact
ci:       CI/CD changes
docs:     Documentation-only changes
feat:     New user-facing feature
fix:      Internal fix, not a user-facing bug
perf:     Performance improvement
refactor: Code moved or rewritten, no behavior change
revert:   Reverts a previous commit
style:    Code style, formatting
test:     Test infrastructure
tweak:    Value or setting change
unify:    Move duplicated code into shared location
```

**Rules:**
- Use **present tense** ("Fix X" not "Fixed X")
- Don't end the subject with a period
- Start with an action verb
- Scope is the module: `ww3d2`, `wwlib`, `Combat`, `Commando`, `cmake`, etc.
- Subject under 70 chars
- Body explains why; use `* Fixes #123` / `* Closes #456` for issue/PR refs

**Examples:**
```
ww3d2: backport SortingRenderer optimization from GeneralsX
wwlib: back RegistryClass storage with INI files on non-Windows
Commando/Combat: extract SDL3 native window handle for DXVK
wwlib: add win32_compat.h shim layer for non-Windows builds
```

### Pull request guidelines
- **One focused change per PR** — don't mix refactors with logical changes
- **PR title** = the commit's `type(scope): Description` (or a summary if multiple commits)
- **PR body** should:
  - Link related issues with `* Fixes #N` / `* Closes #N`
  - Explain the why, not just the what
  - Note if the change has cross-platform implications ("Dormant on Windows, active on non-Windows")
- **Squash and merge** for single-commit PRs; **Rebase and merge** for multi-commit PRs
- **Zero Hour changes take precedence over Generals** — in the Generals-side project, but for Renegade (single game), the equivalent is: **core engine changes first, then level scripts, then tools**

### Code style
- Match the surrounding legacy code style. The original is C++98-ish with the occasional modern feature. Don't introduce C++20 idioms where C++98 would do.
- `#pragma once` over include guards (when touching new files)
- Use `WWASSERT` (defined in `Code/wwdebug/wwdebug.h`) for asserts
- Use `WWDEBUG_SAY((...))` for debug output (it's a no-op in release)
- Platform-specific code: `#if defined(_WIN32)` / `#if defined(OPENW3D_WIN32)` / `#if defined(OPENW3D_SDL3)` — pick the most specific
- Prefer `#if defined(X)` over `#ifdef X`

### Testing
- **Windows MinGW build is the Linux test-bed** since this machine doesn't have MSVC. The `build/gcc-test/` directory is the working build.
- **Verify `renegadeserver.exe` and `renegade.exe` both link cleanly** after any change to `Code/Commando/`, `Code/Combat/`, `Code/ww3d2/`, `Code/wwlib/`, etc.
- **Linux runtime tests happen on a real Linux host** (we don't have one locally; the work is verified via `cmake --preset linux` configure + `cmake --build` which catches compile errors)
- The `Code/Tests/` directory has small demo programs that exercise individual subsystems

## Common Patterns

### Adding a new cross-platform shim
1. Add inline definitions in `Code/wwlib/win32_compat.h` (guarded by `#if !defined(_WIN32)`)
2. The shim is auto-included via `Code/dxvk_wrapper/dxvk_wrapper_compat.h` which is pulled in by any file using `<windows.h>`
3. If a `.cpp` file uses Win32 but doesn't include `<windows.h>` directly, add `#include "win32_compat.h"` to it explicitly
4. Test: `cmake --build build/gcc-test --target renegadeserver` should still build clean

### Adding a new audio backend
- The `WWAudio/CMakeLists.txt` has a 3-way pick: `if(W3D_BUILD_OPTION_OPENAL) ... elseif(W3D_BUILD_OPTION_MILES) ... else() ... endif()`
- Add your new backend as a new `mydir/MyClass.cpp` source file, then add it to the appropriate branch
- Implement `WWAudioClass::Create_Instance()` factory in your backend — that's the only function the rest of the engine calls

### Modifying a header used by both engine and scripts
- `Code/Combat/action.h`, `Code/Combat/input.h` etc. are used in both the engine (`Code/Combat/`) and the level scripts (`Code/Scripts/`)
- Changes must be ABI-compatible with the existing compiled `scripts.dll` binary (if users have one installed)
- Prefer additive changes (new enums, new functions) over breaking changes

### Working with the DXVK rendering path
- `Code/ww3d2/dx8wrapper.cpp` uses `IDirect3D9::CreateDevice(...)` with an `HWND` argument
- On non-Windows, DXVK's `d3d9.h` defines `HWND` as `uint32_t` (the X11 Window id)
- The `Create_Main_Window()` function in `Code/Commando/WINMAIN.CPP` extracts the X11 handle via `SDL_GetWindowWMInfo` (or `SDL_GetWindowProperties` with the `SDL_PROP_WINDOW_X11_WINDOW_POINTER` property)
- Don't store `SDL_Window*` in `MainWindow` — that breaks DXVK

## Key Files to Understand

- `CMakePresets.json` — all build configurations
- `Code/ww3d2/dx8wrapper.cpp` — the rendering pipeline
- `Code/ww3d2/sortingrenderer.cpp` — the particle/poly sort renderer (shared with Generals)
- `Code/Commando/WINMAIN.CPP` — entry point + window creation
- `Code/Combat/directinput.cpp` / `directinput_sdl3.cpp` — input backends
- `Code/Combat/input.cpp` — keyboard/mouse mapping (uses `DirectInput::Get_Button_Value` + `DIK_*` codes)
- `Code/WWAudio/CMakeLists.txt` — audio backend selection
- `Code/wwlib/win32_compat.h` — cross-platform Win32 shim
- `Code/wwlib/registryini.h/.cpp` — INI-backed registry for non-Windows
- `Code/wwlib/mutex.h` — thread primitives (used by everything)
- `Code/wwnet/socket_wrapper_posix.cpp` / `socket_wrapper_win32.cpp` — socket abstraction
- `cmake/FindFFmpeg.cmake`, `cmake/FindOpenAL.cmake` — explicit-path find modules for dev libs
- `cmake/dx9.cmake` — DXVK native discovery (non-Windows only)

## What NOT to do

- Don't reintroduce MFC, COM, or other Windows-only tech that we explicitly moved away from
- Don't write platform-specific code without `#if defined(_WIN32)` or `#if defined(OPENW3D_SDL3)` guards
- Don't add a new dependency without updating `vcpkg.json` (if we have it) or `CMakePresets.json` and the appropriate `cmake/<lib>.cmake`
- Don't add SDL3, OpenAL, FFmpeg, FreeType, or ICU to the engine without going through the existing `W3D_BUILD_OPTION_*` switch
- Don't remove the `WINMAIN.CPP` Win32 path — the SDL3 path is `#elif defined(OPENW3D_SDL3)`, it falls through to the Win32 path on Windows
- Don't break the `WWAudio/CMakeLists.txt` 3-way branch (openal/miles/null) — each backend must compile and link in isolation
