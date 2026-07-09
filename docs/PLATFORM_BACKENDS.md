# OpenW3D platform/backend matrix

OpenW3D should not grow one-off platform hacks. The build is split by **platform layer** and **renderer backend** so Windows, Linux, macOS, iOS, Android, and web can advance on the same architecture.

## Build dimensions

### Platform layer

| Layer | Purpose | Initial targets |
|---|---|---|
| `OPENW3D_WIN32` | Native Win32 window/input/filesystem glue | Windows desktop |
| `OPENW3D_SDL3` | Portable window/input/events layer | Linux, macOS, Android, iOS, web |
| `OPENW3D_POSIX` | Core/static-lib portability without windowing | Android/iOS/web/core CI, early bring-up |

### Renderer backend

`W3D_RENDERER` is the renderer selector:

| Value | Purpose | Platforms |
|---|---|---|
| `NATIVE` | Native Windows D3D9 SDK/libs | Windows only |
| `DXVK` | Real D3D9 headers + `libdxvk_d3d9` | Linux, macOS+MoltenVK, Android+Vulkan, future iOS experiments |
| `NULL` | No renderer/app stack; compile portable core libs | Every platform/toolchain |

`NULL` is not the final game. It is the bootstrap lane that proves the low-level engine code compiles for a target before renderer/window/audio/package work is added.

## Target tiers

### Tier 0 — core/null

Goal: produce static libraries from renderer-neutral code on every toolchain.

Expected libs today:

- `libwwbitpack.a`
- `libwwdebug.a`
- `libwwlib.a`
- `libwwmath.a`
- `libwwsaveload.a`
- `libwwtranslatedb.a`
- `libwwutil.a`

This tier must stay green for Windows, Linux, macOS, Android, iOS, and web toolchains as they are added.

### Tier 1 — platform shell

Goal: SDL3/window/input loop compiles and links without renderer-specific code where possible.

Current target: `openw3d_platform_shell`, a minimal SDL3 executable that initializes SDL, creates a window, pumps events briefly, and exits. It deliberately uses `W3D_RENDERER=NULL` so it exercises platform/windowing without bringing the full game, renderer, audio, or physics stack back in.

Presets:

- `windows-sdl3-shell`
- `linux-sdl3-shell`
- `macos-sdl3-shell`
- `android-sdl3-shell`

Targets:

- Linux SDL3
- macOS SDL3
- Android SDL3 activity/package
- iOS SDL3 app bundle
- web/Emscripten SDL3 shell

### Tier 2 — renderer

Goal: full `ww3d2` renderer builds against real headers/libraries.

Rules:

- Do **not** re-add `Code/d3d9_stub/`.
- `dx8wrapper.h` is currently the renderer-facing interface and is included by much of `ww3d2`; source-list splitting is not enough.
- Use real D3D9 header surfaces through CMake:
  - Windows: native/min-dx9 SDK
  - Non-Windows: DXVK/Wine D3D9 headers + `libdxvk_d3d9`
- `Code/dxvk_wrapper/d3d9.h` stays first in include order for DXVK mode and forwards via `#include_next` to the real Wine/MinGW `d3d9.h` supplied by DXVK/native headers.
- Android DXVK helper script: `scripts/build-dxvk-android-arm64.sh` builds `libdxvk_d3d9.so` from the local `engine-reference/fbraz3-dxvk` checkout.

### Tier 3 — packaging/runtime

Goal: platform-specific app packaging and runtime validation.

- Windows: native executable
- Linux: SDL3 + DXVK/Vulkan bundle
- macOS: SDL3 + MoltenVK/DXVK app bundle
- Android: Gradle/APK or CMake/SDL activity + Vulkan/DXVK libs
- iOS: Xcode/CMake toolchain + MoltenVK bundle
- web: Emscripten/WASM renderer strategy still TBD; likely not DXVK

## Preset naming convention

Use `<platform>-<tier>` or `<platform>-<backend>`:

- `windows-core-null`
- `linux-core-null`
- `macos-core-null`
- `android-core-null`
- `linux-dxvk`
- `android-dxvk-compile-probe`
- `android-dxvk-linked-probe`

Avoid Android-only names for generic architecture. Android is just one consumer of the same backend matrix.

## Current verified state

Verified on Dadud's Windows/MSYS host:

- `windows-core-null`
- `android-core-null`
- `windows-sdl3-shell` + dummy-video run
- `android-sdl3-shell`
- `windows-native-renderer-probe` target `ww3d2`
- `android-dxvk-compile-probe` target `ww3d2`
- `android-dxvk-linked-probe` target `ww3d2`

Current Android DXVK artifact:

```text
C:/Users/Dadud/projects/engine-reference/fbraz3-dxvk/build-android-arm64/src/d3d9/libdxvk_d3d9.so
```

## Next engineering steps

1. Keep Tier 0/core-null and Tier 1 SDL3-shell green while renderer/runtime work advances.
2. Add a minimal renderer smoke executable that initializes SDL3 + WW3D/DXVK far enough to clear/present one frame.
3. Package Android SDL3 + `libdxvk_d3d9.so` into an APK/activity.
4. Enable Tier 2 `W3D_RENDERER=DXVK` on Linux/macOS with real platform toolchains.
5. Evaluate iOS/web renderer feasibility separately; DXVK is not assumed for web.
