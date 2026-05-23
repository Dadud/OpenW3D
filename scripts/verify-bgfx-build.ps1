# Verifies BGFX prerequisites before an interactive Windows run.
# Usage: .\scripts\verify-bgfx-build.ps1 [-BuildDir build]

param(
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $root

$fail = 0

function Require-Path($path, $hint) {
    if (-not (Test-Path $path)) {
        Write-Host "FAIL: missing $path"
        if ($hint) { Write-Host "      $hint" }
        $script:fail++
    } else {
        Write-Host "OK:   $path"
    }
}

Write-Host "=== BGFX build verification ==="

Require-Path "external/bgfx/include/bgfx/bgfx.h" "Run: git submodule update --init --recursive"
Require-Path "external/bx/include/bx/bx.h" "Run: git submodule update --init --recursive"

$bgfxLib = Get-ChildItem -Path "external/bgfx/.build" -Recurse -Filter "bgfxRelease.lib" -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $bgfxLib) {
    $bgfxLib = Get-ChildItem -Path "external/bgfx/.build" -Recurse -Filter "bgfx*.lib" -ErrorAction SilentlyContinue | Select-Object -First 1
}
if ($bgfxLib) {
    Write-Host "OK:   $($bgfxLib.FullName)"
} else {
    Write-Host "FAIL: no bgfx .lib under external/bgfx/.build"
    Write-Host "      cd external/bgfx; ..\bx\tools\bin\windows\genie.exe vs2022"
    Write-Host "      msbuild .build\projects\vs2022\bgfx.sln /p:Configuration=Release /p:Platform=x64"
    $fail++
}

$shaderDir = Join-Path $BuildDir "shaders"
$shaders = @("vs_uber_s_5_0.bin", "fs_uber_s_5_0.bin", "vs_mesh_s_5_0.bin", "fs_mesh_s_5_0.bin")
foreach ($s in $shaders) {
    Require-Path (Join-Path $shaderDir $s) "Build with -DENABLE_BGFX_BACKEND=ON (shaderc runs at compile time)"
}

$exe = Join-Path $BuildDir "renegade.exe"
if (-not (Test-Path $exe)) { $exe = Join-Path $root "Run/renegade.exe" }
Require-Path $exe "cmake --build $BuildDir --config Release --target renegade"

if ($fail -gt 0) {
    Write-Host "`n$fail check(s) failed."
    exit 1
}
$strings = Join-Path $root "Run/data/strings.tdb"
if (Test-Path $strings) {
    Write-Host "OK:   $strings"
} else {
    Write-Host "WARN: no Run/data/strings.tdb — run .\scripts\make-run-package.ps1 -InstallPath <Renegade>"
}

Write-Host "`nAll build checks passed."
