# Copy retail C&C Renegade game data into Run/data/.
# Usage: .\scripts\import-renegade-install.ps1 [-InstallPath "C:\...\Renegade"] [-RunDir Run]

param(
    [string]$InstallPath = "",
    [string]$RunDir = "Run"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $root

$runPath = Join-Path $root $RunDir
$dataPath = Join-Path $runPath "data"
New-Item -ItemType Directory -Force -Path $dataPath | Out-Null

function Find-RenegadeInstall {
    param([string]$Hint)
    $candidates = @()
    if ($Hint -and (Test-Path $Hint)) { $candidates += (Resolve-Path $Hint).Path }

  $steamRoots = @(
        "${env:ProgramFiles(x86)}\Steam\steamapps\common",
        "$env:ProgramFiles\Steam\steamapps\common"
    )
    foreach ($steam in $steamRoots) {
        if (Test-Path $steam) {
            $candidates += @(
                (Join-Path $steam "Renegade"),
                (Join-Path $steam "Command and Conquer Renegade"),
                (Join-Path $steam "Command & Conquer Renegade")
            )
        }
    }

    $candidates += @(
        "${env:ProgramFiles(x86)}\Westwood\Command & Conquer Renegade",
        "${env:ProgramFiles(x86)}\EA Games\Command and Conquer Renegade",
        "${env:ProgramFiles}\EA Games\Command and Conquer Renegade"
    )

    foreach ($dir in $candidates) {
        if (-not $dir) { continue }
        $data = Join-Path $dir "Data"
        if (Test-Path $data) { return (Resolve-Path $dir).Path }
        $dataLower = Join-Path $dir "data"
        if (Test-Path $dataLower) { return (Resolve-Path $dir).Path }
    }
    return $null
}

function Resolve-DataDir {
    param([string]$InstallRoot)
    foreach ($name in @("Data", "data", "DATA")) {
        $p = Join-Path $InstallRoot $name
        if (Test-Path $p) { return (Resolve-Path $p).Path }
    }
    return $null
}

if (-not $InstallPath) {
    $InstallPath = Find-RenegadeInstall
    if (-not $InstallPath) {
        throw @"
Could not find a Renegade install. Pass -InstallPath, for example:
  .\scripts\import-renegade-install.ps1 -InstallPath `"C:\Program Files (x86)\Steam\steamapps\common\Renegade`"
"@
    }
    Write-Host "Detected install: $InstallPath"
} elseif (-not (Test-Path $InstallPath)) {
    throw "Install path not found: $InstallPath"
} else {
    $InstallPath = (Resolve-Path $InstallPath).Path
}

$retailData = Resolve-DataDir $InstallPath
if (-not $retailData) {
    throw "No Data\ folder under $InstallPath"
}

Write-Host "Copying $retailData -> $dataPath"
$robocopyArgs = @(
    $retailData,
    $dataPath,
    "/E",
    "/NFL", "/NDL", "/NJH", "/NJS", "/nc", "/ns", "/np"
)
& robocopy @robocopyArgs | Out-Null
# robocopy exit codes 0-7 are success / acceptable
if ($LASTEXITCODE -ge 8) {
    throw "robocopy failed with exit code $LASTEXITCODE"
}

$required = @("strings.tdb", "Always.dat")
$missing = @()
foreach ($file in $required) {
    $found = Get-ChildItem -Path $dataPath -Recurse -Filter $file -File -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $found) {
        $alt = Join-Path $InstallPath $file
        if (Test-Path $alt) {
            Copy-Item $alt $dataPath -Force
            Write-Host "Copied $file from install root"
        } else {
            $missing += $file
        }
    }
}

$mixCount = (Get-ChildItem -Path $dataPath -Filter "*.mix" -Recurse -File).Count
Write-Host "Imported $mixCount .mix file(s) into $dataPath"

if ($missing.Count -gt 0) {
    Write-Warning "Still missing: $($missing -join ', ') — use a full v1.037 Renegade Data folder."
} else {
    Write-Host "Required assets present (strings.tdb, Always.dat)."
}

Write-Host "`nDone. Launch:  cd Run; .\Play-Renegade.bat"
