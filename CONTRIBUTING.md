# How to contribute

## Process

1. **Fork** this repository
2. **Create a feature branch** (`git checkout -b fix/your-fix` or `feat/your-feature`)
3. **Make your changes** following the commit message and code style conventions below
4. **Test on Windows** — the project builds with MinGW as a Linux stand-in. See [Build](#build) below.
5. **Open a pull request** using the PR template

## Commit message conventions

We use [Conventional Commits](https://www.conventionalcommits.org/) with an extended set of types. The format is:

```
type(scope): Description starting with action verb
```

Allowed types:

| Type     | Use case                                                  |
|----------|-----------------------------------------------------------|
| bugfix   | Fixes a user-facing bug                                   |
| fix      | Internal fix, not a user-facing bug                       |
| feat     | New user-facing feature                                    |
| refactor | Code moved or rewritten, no behavior change                |
| unify    | Moves duplicated code from one location to shared        |
| perf     | Performance improvement                                    |
| build    | Addresses a compile warning or error                       |
| chore    | Maintenance work, no user-facing impact                   |
| docs     | Documentation only                                         |
| ci       | CI/CD changes                                               |
| test     | Test infrastructure                                        |
| style    | Code style, formatting                                     |
| revert   | Reverts a previous commit                                  |
| tweak    | Value or setting change                                    |

**Rules:**

- **Present tense** ("Fix X" not "Fixed X")
- **No period** at the end of the subject
- **Start with an action verb**
- **Scope is the module**: `ww3d2`, `wwlib`, `Combat`, `Commando`, `cmake`, `binkmovie`, etc.
- **Subject under 70 chars**
- **Body** explains why; use `* Fixes #123` / `* Closes #456` for issue/PR refs

**Examples (real commits from this repo):**

```
ww3d2: backport SortingRenderer optimization from GeneralsX
wwlib: back RegistryClass storage with INI files on non-Windows
Commando/Combat: extract SDL3 native window handle for DXVK
wwlib: add win32_compat.h shim layer for non-Windows builds
```

## Code change documentation

**Every user-facing change requires an in-code comment** with this format:

```cpp
// OpenW3D @keyword author DD/MM/YYYY Description
```

`OpenW3D` and `@keyword` are mandatory. `author` and date can be omitted. Common keywords match the commit types: `@bugfix`, `@feature`, `@refactor`, `@perf`, `@build`, `@tweak`, `@info`, `@todo`.

For multi-line explanations, continue the comment on subsequent lines:

```cpp
// OpenW3D @bugfix BenderAI 20/02/2026 TARGA.h long->int32_t 64-bit fix
// The root cause is that on LP64 systems (Linux, macOS) `long` is 8 bytes
// but the TGA 2.0 spec uses 32-bit file offsets. The struct ended up 34
// bytes instead of 26, breaking Targa::Open()'s fixed-size read.
```

For backports from sister projects (GeneralsX, TheSuperHackers/GeneralsGameCode), cite the source:

```cpp
// OpenW3D @refactor Backported from TheSuperHackers/GeneralsGameCode
// commit 98f1db9929 (SortingRenderer merge, June 2026).
```

## Pull request guidelines

- **One focused change per PR** — don't mix refactors with logical changes
- **PR title** = the commit's `type(scope): Description` (or a summary if multiple commits)
- **PR body** should link related issues with `* Fixes #N` / `* Closes #N`
- **Cross-platform impact**: if your change has Linux/macOS implications, say so in the PR body
- **Squash and merge** for single-commit PRs; **Rebase and merge** for multi-commit PRs
- **Module-first ordering**: when a change affects both an engine module and the game client, land the engine module change first so the game client can be built against it

## Code style

- Match the surrounding legacy code style. The original is C++98-ish with the occasional modern feature. Don't introduce C++20 idioms where C++98 would do.
- `#pragma once` over include guards (when touching new files)
- Use `WWASSERT` (defined in `Code/wwdebug/wwdebug.h`) for asserts
- Use `WWDEBUG_SAY((...))` for debug output (it's a no-op in release)
- Platform-specific code: `#if defined(_WIN32)` / `#if defined(OPENW3D_WIN32)` / `#if defined(OPENW3D_SDL3)` — pick the most specific
- Prefer `#if defined(X)` over `#ifdef X`

## Build

The project builds on **Windows (MSVC and MinGW)** and **Linux (GCC)**. The CI matrix in `.github/workflows/openw3d.yml` exercises 7 combinations:

- MSVC x86 / MSVC x64 (Release, `/W4 /WX`)
- MinGW x86 / MinGW x64 / MinGW clang x64 / MinGW x64 SDL3
- Linux x64

### Quick local build (Windows MinGW)

This is the closest stand-in we have for a Linux build since the same MinGW toolchain can build the project:

```bash
# Configure
cmake -S . -B build/gcc-test -G Ninja \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=c++ \
    -DCMAKE_CXX_FLAGS=-Wno-error

# Build the full client
cmake --build build/gcc-test --target renegade

# Build the headless server
cmake --build build/gcc-test --target renegadeserver
```

Both should produce 14.3 MB executables in `build/gcc-test/`.

### Quick local build (Linux)

```bash
# Install deps (Ubuntu/Debian)
sudo apt install g++ cmake ninja-build libsdl3-dev libopenal-dev \
    libfreetype-dev libavcodec-dev libavformat-dev libavutil-dev \
    libswresample-dev libswscale-dev libvulkan-dev

# Configure and build
cmake --preset linux
cmake --build build/linux
```

### What to verify before opening a PR

- [ ] Both `renegade.exe` and `renegadeserver.exe` build clean (Windows MinGW)
- [ ] No new compiler warnings
- [ ] On Linux, `cmake --preset linux` configures without errors
- [ ] Touched files match the scope of the change (no drive-by edits)
- [ ] Added `// OpenW3D @keyword author DD/MM/YYYY Description` comments where needed

## AI code generation

Creating changes with LLM generated code is allowed. The author is responsible for verifying that all generated code is human readable, maintainable, and logically correct. All generated code needs to be tested and verified. The author is not allowed to outsource the polishing of generated code to human code reviewers. In a Pull Request, generated code needs to be announced as such and to what extent it was polished by human intervention.

New contributors are discouraged from submitting Pull Requests with thousands of lines added with the help of LLMs — human code reviewers cannot attend to such volumes at the risk of wasting precious time with potentially poorly generated code. Prefer opening fewer high-quality PRs.

See `.github/copilot-instructions.md` for a detailed overview of the codebase architecture and conventions for AI coding agents.

## License

Renegade source is released under the GPL v3. See `LICENSE.md` for details. By submitting a contribution, you agree to license it under the same terms.
