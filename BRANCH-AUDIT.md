# OpenW3D Branch & Workflow Audit

**Date:** 2026-05-04
**Auditor:** Moss 🌿
**Repo:** `/home/dadud/projects/OpenW3D`

---

## ✅ Current State (Post-Cleanup)

### Divergence Map

```
upstream/main:  a075221  wwconfig-qt (#117)
                ↑
merge-base:     f1aaa93  cleanup(): Add new lines for debug output...
                ↓
origin/main:    21c3f10  fix(build): Linux cross-platform stubs and CMake fixes
                ↑
                │  Your work now properly committed:
                │  - Merge upstream/main (wwconfig-qt absorbed)
                │  - Submodule registration (bgfx, bx, bimg)
                │  - Cross-platform stubs and CMake fixes
```

### Active Branches

| Branch | Status | Purpose |
|--------|--------|---------|
| `main` | ✅ Clean | Integration branch — upstream + your merged work |
| `feature/crossplatform-cmake-fixes` | ✅ Merged | Linux stubs, CMake fixes, submodules |

### Remote Branches with Unmerged Work

| Branch | Commits Ahead of Main | Notes |
|--------|----------------------|-------|
| `linux-build-fixes-v2` | 3 unique | CLI args (`--map`, `--renderer`, `--windowed`) + Linux stubs |
| `bgfx-integration` | 5 unique | BGFX backend support (PR 2) + Windows fixes |
| `bgfx-backend-combined` | 3 unique | BGFX integrated into ww3d2 pipeline |
| `bgfx-stabilization` | 5 unique | BGFX texture/buffer cache invalidation fixes |
| `pr-3-bgfx-rendering-fixed` | 5 unique | Review fixes for PR 3 — blend/alpha/fog/shaders |
| `fix/review-pr3` | 5 unique | CRT runtime fixes, headless BGFX init, interactive session |
| `cmake-buildability-audit` | 3 unique | Linux CMake smoke presets + build exclusions |
| `sdl3-window` | 1 unique | SDL3 window replacement |
| `feature/sdl3-gamepad` | 1 unique + PR3 | SDL3 gamepad/joystick |
| `integration/pr105-bgfx-stack-test` | 4 unique | Integration testing for BGFX stack |
| `pr/render-device-caps` | 1 unique | BackendDeviceCapabilities interface |
| `pr/render-device-enum` | 1 unique | DX8Backend implementing WW3DBackend |

### Deleted Branches (Stale/Redundant)

- `wip` / `wip-temp` — junk branches
- `dev/bgfx-modern-graphics` — empty, pointed to merge base
- `linux-build-fixes` / `linux-build-fixes-clean` — superseded by `-v2`
- `pr-1-backend-abstraction` — already in upstream history

---

## 🎯 Recommended Next Steps

### 1. Consolidate Linux Build Work

`linux-build-fixes-v2`, `cmake-buildability-audit`, and `feature/crossplatform-cmake-fixes` all overlap. Pick the best commits and create one clean branch:

```bash
git checkout -b feature/linux-build-clean
git cherry-pick 528af8b  # CLI args
git cherry-pick f952ea6  # CLI fix
git cherry-pick 4a85e67  # Linux stubs
# ... or just keep what you have on main and cherry-pick the CLI args
```

### 2. Consolidate BGFX Work

You have **5 branches** for BGFX. This is too many. Recommended merge order:

1. `pr/render-device-caps` + `pr/render-device-enum` → `feature/backend-abstraction`
2. `bgfx-integration` (PR 2 structure)
3. `bgfx-backend-combined` (integration into ww3d2)
4. `pr-3-bgfx-rendering-fixed` (rendering fixes)
5. `bgfx-stabilization` (cache invalidation hardening)
6. `fix/review-pr3` (CRT runtime + headless)

Create a meta-branch or merge them sequentially.

### 3. SDL3 Branches

`sdl3-window` and `feature/sdl3-gamepad` are orthogonal. Keep separate or merge after BGFX is stable.

### 4. PR Strategy

Your `main` is now **2 commits ahead** of `origin/main`:
1. `68fc08c` — Merge upstream/main
2. `23847c0` — Submodules + cross-platform fixes

Push `main` to origin, then open PRs from feature branches to `upstream/main`.

---

## 📝 Workflow Rules (Going Forward)

### Branch Naming

```
feature/<name>     — new features (e.g., feature/bgfx-backend)
fix/<name>         — bug fixes (e.g., fix/linux-build)
docs/<name>        — documentation
ci/<name>          — CI/build changes
refactor/<name>    — code restructuring
integration/<name> — integration branches
```

### Commit Format (Conventional Commits)

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

Types: `feat`, `fix`, `build`, `ci`, `docs`, `refactor`, `test`

### Main Branch Protection

- `main` should only receive:
  - Merges from `upstream/main`
  - Fast-forwards from completed `feature/*` branches
- **Never commit directly to `main`**

### PR Process

1. `git checkout -b feature/my-thing`
2. Do work, commit with conventional commits
3. `git push origin feature/my-thing`
4. Open PR to `w3dhub/OpenW3D:main`
5. After merge, delete local and remote feature branch

---

## 📁 Files

- This audit: `BRANCH-AUDIT.md`
- Workflow rules: `CONTRIBUTING.md` (create me!)
- Workspace notes: `/home/dadud/.openclaw/workspace/TOOLS.md`
