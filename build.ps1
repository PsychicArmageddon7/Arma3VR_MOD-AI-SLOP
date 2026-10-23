<#
.SYNOPSIS
    Builds the Arma 3 VR C++ Extension and stages the @Arma3VR mod folder.
#>

[CmdletBinding()]
param (
    [string]$Configuration = "Release",
    [string]$OutputDir = "$PSScriptRoot\dist\@Arma3VR"
)

$ErrorActionPreference = "Stop"

Write-Host "=== [1/4] Configuring with CMake (x64) ===" -ForegroundColor Cyan
$BuildDir = "$PSScriptRoot\build"
if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

cmake -B $BuildDir -S $PSScriptRoot -A x64

Write-Host "=== [2/4] Building Extension ($Configuration) ===" -ForegroundColor Cyan
cmake --build $BuildDir --config $Configuration

$CompiledDll = "$BuildDir\$Configuration\Arma3VR_x64.dll"
if (!(Test-Path $CompiledDll)) {
    # Check alternate generator path
    $CompiledDll = "$BuildDir\Arma3VR_x64.dll"
}

if (!(Test-Path $CompiledDll)) {
    throw "Compiled DLL not found at expected path: $CompiledDll"
}

Write-Host "=== [3/4] Staging @Arma3VR distribution folder ===" -ForegroundColor Cyan
if (Test-Path $OutputDir) {
    Remove-Item $OutputDir -Recurse -Force
}
New-Item -ItemType Directory -Path "$OutputDir\addons\main" -Force | Out-Null

# Copy Mod root files
Copy-Item $CompiledDll -Destination "$OutputDir\Arma3VR_x64.dll"
Copy-Item "$PSScriptRoot\mod.cpp" -Destination "$OutputDir\mod.cpp"

# Copy OpenXR Loader if present in repo root or build
if (Test-Path "$PSScriptRoot\openxr_loader.dll") {
    Copy-Item "$PSScriptRoot\openxr_loader.dll" -Destination "$OutputDir\openxr_loader.dll"
}

# Copy Addon files
Copy-Item "$PSScriptRoot\addons\main\*" -Destination "$OutputDir\addons\main" -Recurse

Write-Host "=== [4/4] Mod staged successfully ===" -ForegroundColor Green
Write-Host "Staged mod path: $OutputDir"
Write-Host "To test: Launch Arma 3 with parameter: -mod=`"$OutputDir`""
