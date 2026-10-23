[CmdletBinding()]
param (
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

Write-Host "=== [1/3] Configuring CMake (x64) ===" -ForegroundColor Cyan
$BuildDir = "$PSScriptRoot\build"
if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

cmake -B $BuildDir -S $PSScriptRoot -A x64

Write-Host "=== [2/3] Building Extension ($Configuration) ===" -ForegroundColor Cyan
cmake --build $BuildDir --config $Configuration

$TargetDll = "$PSScriptRoot\@Arma3VR\Arma3VR_x64.dll"
if (!(Test-Path $TargetDll)) {
    throw "Build failed: $TargetDll was not produced."
}

Write-Host "=== [3/3] Checking Addon PBO ===" -ForegroundColor Cyan
$AddonSource = "$PSScriptRoot\@Arma3VR\addons\main"
$AddonTargetPbo = "$PSScriptRoot\@Arma3VR\addons\main.pbo"

# Look for armake2 on PATH or locally
$armake = Get-Command "armake2" -ErrorAction SilentlyContinue
if ($armake) {
    Write-Host "Packing main.pbo with armake2..." -ForegroundColor Yellow
    & armake2 pack $AddonSource $AddonTargetPbo -f
    Write-Host "Packed main.pbo successfully." -ForegroundColor Green
} else {
    Write-Host "Notice: 'armake2' not found on PATH." -ForegroundColor Yellow
    Write-Host "You can either:"
    Write-Host "  1. Launch Arma 3 with '-filePatching' to use unpacked @Arma3VR\addons\main directly."
    Write-Host "  2. Pack '@Arma3VR\addons\main' into 'main.pbo' using AddonBuilder / PBO Manager."
}

Write-Host "`n@Arma3VR is ready!" -ForegroundColor Green
