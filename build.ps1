#
# OpenW3D Interactive Build Tool for Windows
# PowerShell script for building on Windows with MSVC or MinGW
#
# Usage:
#   .\build.ps1              # Interactive mode
#   .\build.ps1 -Quick       # Non-interactive, sensible defaults
#   .\build.ps1 -Help        # Show usage
#

param(
    [switch]$Help,
    [switch]$Quick
)

# ---------------------------------------------------------------------------
# Colors & Output
# ---------------------------------------------------------------------------
function Info($msg) { Write-Host "[INFO] $msg" -ForegroundColor Cyan }
function Warn($msg) { Write-Host "[WARN] $msg" -ForegroundColor Yellow }
function Error($msg) { Write-Host "[ERROR] $msg" -ForegroundColor Red }
function Success($msg) { Write-Host "[OK] $msg" -ForegroundColor Green }
function Step($msg) { Write-Host "==> $msg" -ForegroundColor Magenta }

# ---------------------------------------------------------------------------
# Platform & Compiler Detection
# ---------------------------------------------------------------------------
function Detect-Compiler {
    $clPath = Get-Command cl -ErrorAction SilentlyContinue
    $gccPath = Get-Command g++ -ErrorAction SilentlyContinue
    
    if ($clPath) {
        Success "MSVC (Visual Studio) detected"
        return "msvc"
    } elseif ($gccPath) {
        Success "MinGW/GCC detected"
        return "mingw"
    } else {
        Error "No C++ compiler found. Install Visual Studio or MinGW/MSYS2."
        exit 1
    }
}

$Compiler = Detect-Compiler

# ---------------------------------------------------------------------------
# Dependency Checks
# ---------------------------------------------------------------------------
function Check-Deps {
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    $git = Get-Command git -ErrorAction SilentlyContinue
    
    if (-not $cmake) {
        Error "CMake not found. Install from https://cmake.org/download/"
        exit 1
    }
    
    if (-not $git) {
        Error "Git not found. Install from https://git-scm.com/download/win"
        exit 1
    }
    
    Success "Dependencies OK"
}

# ---------------------------------------------------------------------------
# Submodule Setup
# ---------------------------------------------------------------------------
function Setup-Submodules {
    Step "Setting up external dependencies..."
    
    New-Item -ItemType Directory -Force -Path external | Out-Null
    
    if (-not (Test-Path "external/bgfx/.git")) {
        Info "Cloning bgfx..."
        Remove-Item -Recurse -Force -ErrorAction SilentlyContinue external/bgfx
        git clone --depth 1 https://github.com/bkaradzic/bgfx.git external/bgfx
    } else {
        Info "bgfx already present"
    }
    
    if (-not (Test-Path "external/bx/.git")) {
        Info "Cloning bx..."
        Remove-Item -Recurse -Force -ErrorAction SilentlyContinue external/bx
        git clone --depth 1 https://github.com/bkaradzic/bx.git external/bx
    } else {
        Info "bx already present"
    }
    
    if (-not (Test-Path "external/bimg/.git")) {
        Info "Cloning bimg..."
        Remove-Item -Recurse -Force -ErrorAction SilentlyContinue external/bimg
        git clone --depth 1 https://github.com/bkaradzic/bimg.git external/bimg
    } else {
        Info "bimg already present"
    }
    
    Success "External dependencies ready"
}

# ---------------------------------------------------------------------------
# BGFX Build
# ---------------------------------------------------------------------------
function Build-BGFX {
    Step "Building BGFX..."
    
    Push-Location external/bgfx
    
    $geniePath = "..\..\bx\tools\bin\windows\genie.exe"
    
    if (-not (Test-Path $geniePath)) {
        Error "genie.exe not found at $geniePath"
        Pop-Location
        return
    }
    
    if ($Compiler -eq "msvc") {
        & $geniePath vs2022
        Info "BGFX VS2022 solution generated."
        Info "Build it manually or use: cmake --build . --config Release"
        # Could use MSBuild here but it's complex to find the right path
    } else {
        Info "For MinGW, build manually:"
        Info "  cd external/bgfx"
        Info "  make mingw-gcc-release64"
    }
    
    Pop-Location
}

# ---------------------------------------------------------------------------
# Interactive Configuration
# ---------------------------------------------------------------------------
function Interactive-Config {
    Step "Build Configuration"
    Write-Host ""
    
    # Backend
    Write-Host "Select renderer backend:"
    Write-Host "  1) DirectX 9 (native Windows, legacy)"
    Write-Host "  2) BGFX / Vulkan (modern renderer)"
    Write-Host "  3) Null / Headless (server-only)"
    Write-Host ""
    $backend = Read-Host "Choice [1]"
    
    switch ($backend) {
        "2" { $script:EnableBGFX = "ON"; $script:BuildType = "bgfx" }
        "3" { $script:EnableBGFX = "OFF"; $script:BuildType = "headless" }
        default { $script:EnableBGFX = "OFF"; $script:BuildType = "dx9" }
    }
    
    # Build type
    Write-Host ""
    Write-Host "Select build type:"
    Write-Host "  1) Release (optimized)"
    Write-Host "  2) Debug (with symbols)"
    Write-Host "  3) RelWithDebInfo"
    Write-Host ""
    $bt = Read-Host "Choice [1]"
    
    switch ($bt) {
        "2" { $script:CMakeBuildType = "Debug" }
        "3" { $script:CMakeBuildType = "RelWithDebInfo" }
        default { $script:CMakeBuildType = "Release" }
    }
    
    # FFmpeg
    Write-Host ""
    $ff = Read-Host "Enable FFmpeg (video/audio)? [Y/n]"
    if ($ff -eq "" -or $ff -eq "Y" -or $ff -eq "y") {
        $script:EnableFFmpeg = "ON"
    } else {
        $script:EnableFFmpeg = "OFF"
    }
    
    # Miles
    Write-Host ""
    $miles = Read-Host "Enable Miles Audio (legacy)? [Y/n]"
    if ($miles -eq "" -or $miles -eq "Y" -or $miles -eq "y") {
        $script:EnableMiles = "ON"
    } else {
        $script:EnableMiles = "OFF"
    }
}

# ---------------------------------------------------------------------------
# Quick Configuration
# ---------------------------------------------------------------------------
function Quick-Config {
    $script:EnableBGFX = "OFF"
    $script:BuildType = "dx9"
    $script:CMakeBuildType = "Release"
    $script:EnableFFmpeg = "ON"
    $script:EnableMiles = "ON"
}

# ---------------------------------------------------------------------------
# CMake Configure
# ---------------------------------------------------------------------------
function Configure-CMake {
    Step "Configuring with CMake..."
    
    $args = @("-S", ".", "-B", "build", "-DCMAKE_BUILD_TYPE=$CMakeBuildType")
    
    if ($EnableBGFX -eq "ON") {
        $args += "-DENABLE_BGFX_BACKEND=ON"
    } else {
        $args += "-DENABLE_BGFX_BACKEND=OFF"
    }
    
    $args += "-DW3D_BUILD_OPTION_FFMPEG=$EnableFFmpeg"
    $args += "-DW3D_BUILD_OPTION_MILES=$EnableMiles"
    
    if ($Compiler -eq "msvc") {
        $args += @("-G", "Visual Studio 17 2022", "-Ax64")
    } else {
        $args += @("-G", "Ninja", "-DCMAKE_BUILD_TYPE=$CMakeBuildType")
    }
    
    Info "Running: cmake $args"
    & cmake $args
    
    if ($LASTEXITCODE -ne 0) {
        Error "CMake configuration failed"
        exit 1
    }
    
    Success "CMake configuration complete"
}

# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------
function Run-Build {
    Step "Building OpenW3D..."
    
    $args = @("--build", "build", "--parallel")
    
    if ($Compiler -eq "msvc") {
        $args += @("--config", $CMakeBuildType)
    }
    
    Info "Running: cmake $args"
    & cmake $args
    
    if ($LASTEXITCODE -ne 0) {
        Error "Build failed"
        exit 1
    }
    
    Success "Build complete!"
}

# ---------------------------------------------------------------------------
# Asset Detection
# ---------------------------------------------------------------------------
function Find-RenegadeAssets {
    Step "Looking for C&C Renegade game assets..."
    
    $found = @()
    
    # Common paths
    $paths = @(
        "C:\Program Files\EA Games\Command & Conquer Renegade",
        "C:\Program Files (x86)\EA Games\Command & Conquer Renegade",
        "C:\Program Files (x86)\Steam\steamapps\common\Renegade",
        "C:\Program Files\Steam\steamapps\common\Renegade"
    )
    
    # Registry
    $regPaths = @(
        "HKLM:\SOFTWARE\EA Games\Command and Conquer Renegade",
        "HKLM:\SOFTWARE\WOW6432Node\EA Games\Command and Conquer Renegade"
    )
    
    foreach ($rp in $regPaths) {
        try {
            $ip = (Get-ItemProperty -Path $rp -Name InstallPath -ErrorAction SilentlyContinue).InstallPath
            if ($ip -and (Test-Path "$ip\conquer.mix")) {
                $paths += $ip
            }
        } catch {}
    }
    
    foreach ($p in $paths) {
        if (Test-Path "$p\conquer.mix") {
            $found += $p
        }
    }
    
    if ($found.Count -eq 0) {
        Warn "Could not find C&C Renegade installation automatically."
        Warn "Manually copy .mix files to the Run/ directory."
        return
    }
    
    Write-Host ""
    Write-Host "Found installs:"
    for ($i = 0; $i -lt $found.Count; $i++) {
        Write-Host "  $($i+1)) $($found[$i])"
    }
    Write-Host ""
    
    $choice = Read-Host "Select install [1]"
    if ($choice -eq "") { $choice = 1 }
    $idx = [int]$choice - 1
    
    if ($idx -ge 0 -and $idx -lt $found.Count) {
        $sel = $found[$idx]
        Info "Copying .mix files from: $sel"
        New-Item -ItemType Directory -Force -Path Run | Out-Null
        Copy-Item "$sel\*.mix" Run\ -ErrorAction SilentlyContinue
        if (Test-Path "$sel\data") {
            Copy-Item "$sel\data\*.mix" Run\ -ErrorAction SilentlyContinue
        }
        Success "Assets copied to Run/"
    }
}

# ---------------------------------------------------------------------------
# Post-Build
# ---------------------------------------------------------------------------
function Post-Build {
    Step "Post-build setup..."
    
    New-Item -ItemType Directory -Force -Path Run | Out-Null
    
    # Copy executables
    $exeDir = if ($Compiler -eq "msvc") { "build\Release" } else { "build" }
    if (Test-Path $exeDir) {
        Get-ChildItem $exeDir -Filter "*.exe" | ForEach-Object {
            Copy-Item $_.FullName Run\ -ErrorAction SilentlyContinue
        }
    }
    
    # Check assets
    if (-not (Test-Path "Run\conquer.mix")) {
        Warn "Game assets (.mix files) not found in Run/"
        $detect = Read-Host "Try to auto-detect C&C Renegade install? [Y/n]"
        if ($detect -eq "" -or $detect -eq "Y" -or $detect -eq "y") {
            Find-RenegadeAssets
        }
    } else {
        Success "Game assets already present in Run/"
    }
    
    Write-Host ""
    Write-Host "==========================================" -ForegroundColor Green
    Write-Host "  OpenW3D Build Complete!" -ForegroundColor Green
    Write-Host "==========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Build type:    $BuildType"
    Write-Host "CMake config:  $CMakeBuildType"
    Write-Host "Backend:       $EnableBGFX"
    Write-Host ""
    Write-Host "Binaries:      build\"
    Write-Host "Game data:     Run\"
    Write-Host ""
    
    if (-not (Test-Path "Run\conquer.mix")) {
        Write-Host "WARNING: Place your C&C Renegade .mix files in Run/ before running" -ForegroundColor Yellow
    }
    
    Write-Host ""
    Write-Host "To run:"
    Write-Host "  .\Run\renegade.exe    (client)"
    Write-Host "  .\Run\renegadeserver.exe  (dedicated server)"
    Write-Host ""
}

# ---------------------------------------------------------------------------
# Help
# ---------------------------------------------------------------------------
function Show-Help {
    Write-Host @"
OpenW3D Interactive Build Tool for Windows

Usage:
  .\build.ps1              Run interactive build wizard
  .\build.ps1 -Quick       Build with sensible defaults
  .\build.ps1 -Help        Show this help

Compilers:
  MSVC     - Visual Studio 2022 (recommended)
  MinGW    - MSYS2 / mingw-w64

Prerequisites:
  - Visual Studio 2022 with "Desktop development with C++"
    OR MSYS2 with mingw-w64 toolchain
  - CMake 3.25+
  - Git

Dependencies (MSYS2):
  pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
            mingw-w64-x86_64-gcc mingw-w64-x86_64-ffmpeg

Game Assets:
  You need a legal copy of C&C Renegade.
  This script can auto-detect Steam, EA App, and retail installs.
"@
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
function Main {
    Write-Host ""
    Write-Host "==========================================" -ForegroundColor Cyan
    Write-Host "  OpenW3D Interactive Build Tool" -ForegroundColor Cyan
    Write-Host "  Platform: Windows ($Compiler)" -ForegroundColor Cyan
    Write-Host "==========================================" -ForegroundColor Cyan
    Write-Host ""
    
    if ($Help) {
        Show-Help
        exit 0
    }
    
    Check-Deps
    Setup-Submodules
    
    if ($Quick) {
        Info "Quick mode: using defaults"
        Quick-Config
    } else {
        Interactive-Config
    }
    
    if ($EnableBGFX -eq "ON") {
        Build-BGFX
    }
    
    Configure-CMake
    Run-Build
    Post-Build
}

Main
