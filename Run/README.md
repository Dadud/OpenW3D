# OpenW3D run folder

This is the **game working directory**. `renegade.exe` must be launched from here (or use `Play-Renegade.bat`).

## One-command setup (you have a retail install)

From the repo root in PowerShell:

```powershell
.\scripts\make-run-package.ps1 -InstallPath "D:\Games\Renegade"
```

If `-InstallPath` is omitted, the script tries Steam and common Westwood/EA paths.

That copies your retail **Data** tree into `Run\data\` and stages `renegade.exe` plus BGFX `shaders\` from `build\`.

## Manual drop-in

1. Build OpenW3D (BGFX): see [docs/BUILD.md](../docs/BUILD.md).
2. Run `.\scripts\stage-run.ps1` to copy `renegade.exe` and `shaders\` into this folder.
3. Copy everything from your Renegade install **`Data`** folder into **`Run\data\`** (all `.mix`, `strings.tdb`, `Always.dat`, etc.).

Retail layout (v1.037):

```
Your Renegade install\
  Renegade.exe
  Data\
    Always.dat
    Always2.dat
    strings.tdb
    *.mix
    ...
```

OpenW3D expects:

```
Run\
  renegade.exe          ← from scripts\stage-run.ps1
  shaders\              ← from build (BGFX)
  data\                 ← your retail Data\ contents
    Always.dat
    strings.tdb
    *.mix
```

On Windows, `DATA\` and `data\` are the same folder.

## Play

```powershell
cd Run
.\Play-Renegade.bat
```

Or double-click `Play-Renegade.bat`.

## Verify before playing (BGFX)

```powershell
.\scripts\verify-bgfx-build.ps1 -BuildDir build
```
