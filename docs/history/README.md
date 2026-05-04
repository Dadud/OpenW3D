# OpenW3D Historical Archive

This directory preserves original artifacts from the `electronicarts/CnC_Renegade` source release and the early modernization era of the OpenW3D project. These are kept for historical context — they document the original state of the codebase before modernizations.

## Contents

### Original EA Documentation

| File | Description |
|------|-------------|
| `README.original.md` | The original README from `electronicarts/CnC_Renegade`. Written for the 2002 source release. Recommends MSVC 6.0, lists dead SDK dependencies (DirectX SDK 8, GameSpy, SafeDisk), states "we will not accept contributions." |
| `LICENSE.original.md` | The original GPL v3 license as published by Electronic Arts Inc. with EA's additional terms. |

### EA Internal DX8 Documentation

These `.txt` files are EA/Westwood internal developer documents covering the DirectX 8 rendering subsystem. They are embedded in `Code/ww3d2/` and describe the original architecture before OpenW3D modernization.

| File | Description |
|------|-------------|
| `dx8-rationalizations.txt` | Design decisions and reasoning behind DX8 API choices. Covers lighting models, mesh rendering, and FVF format decisions. |
| `dx8-status.txt` | Status of the DX8 implementation — what was working, what was pending, known issues at time of source release. |
| `dx8-tool-modifications.txt` | Documents modifications made to external tools for DX8 integration. |
| `render-object-guide.txt` | Internal guide to the render object hierarchy (MeshClass, LightClass, SceneClass, etc.). |
| `state-management.txt` | Documents the render state management approach in the original DX8 pipeline. |

### Legacy Project Files

The `.dsw` (Visual Studio 6 Workspace) and `.dsp` (Visual Studio 6 Project) files are from the original EA/Westwood build system. These are preserved as-is in `Code/` and subdirectories. They are no longer actively used — the project has migrated to CMake.

Notable workspaces:
- `Code/commando.dsw` — Main Renegade game project (Visual Studio 6)
- `Code/tools.dsw` — Build tools workspace

For a complete list: `find Code -name "*.dsw" -o -name "*.dsp" | grep -v build`

### Modernization Timeline

Key commits marking the transition from legacy to modern:

| Commit | Description |
|--------|-------------|
| `373b5af` | `docs: add TASKS.md with accurate status tracking` |
| `e95aa19` | `docs: update TASKS.md — accurate status after verification` |
| `b8298fb` | `ci: expand Linux CI — submodules, BGFX, full ww3d2+renegade builds` |
| `e84f153` | `docs: update TASKS.md — CI expanded, shader pipeline documented, E–I tracks added` |
| `068a27d` | `feat(ww3d): wire WW3D_BGFX_BACKEND into backend selection` |
| `5b10ac7` | `fix(cmake): move target_include_directories after add_library(ww3d2)` |

See `docs/TASKS.md` for the full task tracking system.
