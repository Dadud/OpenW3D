# Local development (Dadud fork)

## Source of truth

- **Integration branch:** `dev` (merge of `origin/main` BGFX stack + `upstream/main` platform layer)
- **Upstream:** `upstream` → https://github.com/w3dhub/OpenW3D
- **Fork:** `origin` → https://github.com/Dadud/OpenW3D
- **Process docs:** https://github.com/Dadud/OpenW3D-private-work

## Branch policy

- Do **not** merge `pr/*` branches wholesale. Use `pr/*` as read-only archaeology.
- Cherry-pick individual commits only when a diff review shows they are still missing.
- BGFX integration is one line on `dev` / `main`, not stacked PR branches.

## Typical workflow

```powershell
git fetch upstream origin
git checkout dev
git merge upstream/main   # when w3dhub moves forward
# resolve: keep fork BGFX files; take upstream CI/SDL/LLVM
```

## CI matrix

See [docs/BUILD.md](docs/BUILD.md). Fork adds `Linux x64 BGFX` and `MSVC x64 BGFX` to the upstream 7-job matrix.

## Verification

- Build: push to `origin` and watch GitHub Actions
- BGFX Windows: `.\scripts\verify-bgfx-build.ps1` then interactive run with retail data in `Run/`
- Track status: [docs/TASKS.md](docs/TASKS.md)
