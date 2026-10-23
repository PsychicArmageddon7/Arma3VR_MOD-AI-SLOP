[CmdletBinding()]
param (
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

# 1. Locate cmake.exe (Check PATH first, then search Visual Studio installations)
$cmakeExe = "cmake"

if (!(Get-Command "cmake" -ErrorAction SilentlyContinue)) {
    # Check via Visual Studio Locator tool (vswhere)
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -property installationPath
        if ($vsPath) {
            $candidate = Join-Path $vsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
            if (Test-Path $candidate) {
                $cmakeExe = $candidate
            }
        }
    }

    # Fallback to standard installation paths if vswhere did not resolve it
    if ($cmakeExe -eq "cmake") {
        $commonPaths = @(
            "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
            "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
            "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
            "C:\Program Files\CMake\bin\cmake.exe"
        )
        foreach ($path in $commonPaths) {
            if (Test-Path $path) {
                $cmakeExe = $path
                break
            }
        }
    }
}

# Ensure cmake was found somewhere
if ($cmakeExe -eq "cmake" -and !(Get-Command "cmake" -ErrorAction SilentlyContinue)) {
    throw "CMake was not found on your system PATH or in your Visual Studio installation.`nPlease either install CMake or run this script from 'Developer PowerShell for VS 2022'."
}

Write-Host "Using CMake at: $cmakeExe" -ForegroundColor DarkGray

# 2. Configure build directory
$BuildDir = "$PSScriptRoot\build"
if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Write-Host "=== [1/2] Configuring CMake (x64) ===" -ForegroundColor Cyan
& $cmakeExe -B $BuildDir -S $PSScriptRoot -A x64

Write-Host "=== [2/2] Building Extension ($Configuration) ===" -ForegroundColor Cyan
& $cmakeExe --build $BuildDir --config $Configuration

$TargetDll = "$PSScriptRoot\@Arma3VR\Arma3VR_x64.dll"
if (Test-Path $TargetDll) {
    Write-Host "`nBuild complete! Output located at:" -ForegroundColor Green
    Write-Host "  $TargetDll" -ForegroundColor Green
    Write-Host "`nRemember: Pack '@Arma3VR\addons\main\' into 'main.pbo' before loading into Arma 3." -ForegroundColor Yellow
} else {
    throw "Build failed: $TargetDll was not produced."
}
