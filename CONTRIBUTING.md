# Contributing to OpenW3D

## Workflow

We use a **feature branch workflow**. All work happens on branches; `main` is for integration only.

### Branch Naming

| Prefix | Use For |
|--------|---------|
| `feature/` | New functionality |
| `fix/` | Bug fixes |
| `docs/` | Documentation |
| `ci/` | CI/build changes |
| `refactor/` | Code restructuring |
| `integration/` | Integration/testing branches |

Examples: `feature/bgfx-backend`, `fix/linux-build`, `docs/api-guide`

### Commit Format

Use [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

**Types:** `feat`, `fix`, `build`, `ci`, `docs`, `refactor`, `test`

**Examples:**
```
feat(bgfx): add texture cache invalidation on release

Implements proper refcount tracking for BGFX textures
to avoid use-after-free when managed textures are destroyed.

Fixes: #42
```

```
fix(build): exclude Windows-only sources from Linux builds

Adds platform guards around dinput.h and other Win32-specific
headers to allow compilation on GCC/Clang.
```

### PR Process

1. Fork `w3dhub/OpenW3D` (already done)
2. Create a feature branch: `git checkout -b feature/my-thing`
3. Do work, commit with conventional commits
4. Push to your fork: `git push origin feature/my-thing`
5. Open a PR to `w3dhub/OpenW3D:main`
6. Address review feedback
7. After merge, delete your feature branch

### Rebasing

Keep your feature branches up to date with upstream:

```bash
git fetch upstream
git rebase upstream/main
# or:
git merge upstream/main
```

Prefer `rebase` for clean history before opening a PR. Use `merge` only for long-running integration branches.

### Submodules

This repo uses Git submodules for external dependencies:

```bash
git submodule update --init --recursive
```

Current submodules:
- `external/bgfx` — Rendering backend
- `external/bx` — BGFX support library
- `external/bimg` — Image loading for BGFX

When updating submodules, commit them separately:

```bash
git add external/bgfx external/bx external/bimg
git commit -m "build: update bgfx/bx/bimg submodules"
```

## Code Style

- C++20 compatible
- Follow existing patterns in `Code/`
- Platform guards: `#ifdef _WIN32` / `#else` / `#endif`
- Avoid stubs when possible; prefer conditional compilation

## Questions?

Open an issue or check `BRANCH-AUDIT.md` for current branch status.
