# Stage OpenW3D binaries + import retail Renegade data into Run/.
# Usage: .\scripts\make-run-package.ps1 [-InstallPath "C:\...\Renegade"] [-BuildDir build]

param(
    [string]$InstallPath = "",
    [string]$BuildDir = "build",
    [string]$RunDir = "Run",
    [switch]$Zip
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $root

Write-Host "=== OpenW3D Run package ===`n"

& (Join-Path $root "scripts\stage-run.ps1") -BuildDir $BuildDir -RunDir $RunDir

$importArgs = @{ RunDir = $RunDir }
if ($InstallPath) { $importArgs["InstallPath"] = $InstallPath }
& (Join-Path $root "scripts\import-renegade-install.ps1") @importArgs

$runPath = Join-Path $root $RunDir
Write-Host "`n=== Package ready ==="
Write-Host "  $runPath"
Write-Host "  Play:  $runPath\Play-Renegade.bat"

if ($Zip) {
    $zipPath = Join-Path $root "OpenW3D-Run.zip"
    if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
    Compress-Archive -Path (Join-Path $runPath "*") -DestinationPath $zipPath -Force
    Write-Host "  Zip:   $zipPath"
}
