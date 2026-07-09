# Copy OpenW3D build outputs into Run/ (exe, DLLs, BGFX shaders).
# Usage: .\scripts\stage-run.ps1 [-BuildDir build] [-RunDir Run]

param(
    [string]$BuildDir = "build",
    [string]$RunDir = "Run"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $root

$buildPath = Join-Path $root $BuildDir
$runPath = Join-Path $root $RunDir

if (-not (Test-Path $buildPath)) {
    throw "Build directory not found: $buildPath`nConfigure and build first (see docs/BUILD.md)."
}

New-Item -ItemType Directory -Force -Path $runPath | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $runPath "data") | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $runPath "shaders") | Out-Null

$exes = @("renegade.exe", "renegadeserver.exe", "combat.exe")
foreach ($name in $exes) {
    $src = Join-Path $buildPath $name
    if (Test-Path $src) {
        Copy-Item -Path $src -Destination $runPath -Force
        Write-Host "Staged $name"
    }
}

Get-ChildItem -Path $buildPath -Filter "*.dll" -File -ErrorAction SilentlyContinue | ForEach-Object {
    Copy-Item -Path $_.FullName -Destination $runPath -Force
    Write-Host "Staged $($_.Name)"
}

$runtimeNames = @(
    "libbandtest.dll",
    "libmss32.dll",
    "binkw32.dll",
    "libstdc++-6.dll",
    "libgcc_s_seh-1.dll",
    "libwinpthread-1.dll"
)
$runtimeSearch = @(
    $buildPath,
    (Join-Path $buildPath "Release"),
    (Join-Path $buildPath "Debug"),
    (Join-Path $env:MINGW_PREFIX "bin")
)
foreach ($name in $runtimeNames) {
    $found = $null
    foreach ($dir in $runtimeSearch) {
        if ($dir) {
            $candidate = Join-Path $dir $name
            if (Test-Path $candidate) { $found = $candidate; break }
        }
    }
    if ($found) {
        Copy-Item $found (Join-Path $runPath $name) -Force
        Write-Host "Staged $name"
    } else {
        Write-Warning "Runtime DLL not found: $name"
    }
}

$shaderSrc = Join-Path $buildPath "shaders"
$shaderDst = Join-Path $runPath "shaders"
if (Test-Path $shaderSrc) {
    Copy-Item -Path (Join-Path $shaderSrc "*") -Destination $shaderDst -Force -Recurse
    $count = (Get-ChildItem $shaderDst -File).Count
    Write-Host "Staged $count shader file(s) to $shaderDst"
} else {
    Write-Warning "No build/shaders/ — build with -DENABLE_BGFX_BACKEND=ON for BGFX."
}

if (-not (Test-Path (Join-Path $runPath "renegade.exe"))) {
    throw "renegade.exe not found under $buildPath. Build: cmake --build $BuildDir --target renegade"
}

Write-Host "`nRun folder ready: $runPath"
Write-Host "Next: import retail data — .\scripts\import-renegade-install.ps1 -InstallPath `"path\to\Renegade`""
