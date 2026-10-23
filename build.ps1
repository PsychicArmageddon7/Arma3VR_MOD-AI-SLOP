[CmdletBinding()]
param (
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

Write-Host "=== [1/2] Configuring CMake (x64) ===" -ForegroundColor Cyan
$BuildDir = "$PSScriptRoot\build"
if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

cmake -B $BuildDir -S $PSScriptRoot -A x64

Write-Host "=== [2/2] Building Extension ($Configuration) ===" -ForegroundColor Cyan
cmake --build $BuildDir --config $Configuration

$TargetDll = "$PSScriptRoot\@Arma3VR\Arma3VR_x64.dll"
if (Test-Path $TargetDll) {
    Write-Host "Build complete! Output located at:" -ForegroundColor Green
    Write-Host $TargetDll
    Write-Host "`nThe @Arma3VR folder is ready to be loaded by Arma 3." -ForegroundColor Green
} else {
    throw "Build failed: $TargetDll was not produced."
}
