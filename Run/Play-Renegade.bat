@echo off
cd /d "%~dp0"
if not exist "renegade.exe" (
  echo renegade.exe not found. From repo root run:  scripts\make-run-package.ps1
  pause
  exit /b 1
)
if not exist "data\strings.tdb" (
  echo Missing game data. Copy your retail Data folder to Run\data\
  echo   or run:  scripts\make-run-package.ps1 -InstallPath "path\to\Renegade"
  pause
  exit /b 1
)
start "" "%~dp0renegade.exe"
