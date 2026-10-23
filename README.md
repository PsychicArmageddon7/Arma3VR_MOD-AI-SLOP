# Arma 3 VR (OpenXR Native Mod)

[![Build](https://github.com/YourUsername/Arma3VR/actions/workflows/build.yml/badge.svg)](https://github.com/YourUsername/Arma3VR/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![OpenXR](https://img.shields.io/badge/OpenXR-1.0-blue.svg)](https://www.khronos.org/openxr/)
[![DirectX 11](https://img.shields.io/badge/DirectX-11-green.svg)](https://docs.microsoft.com/en-us/windows/win32/direct3d11/atpragma-intro)

Native OpenXR Virtual Reality implementation for **Arma 3 (64-bit)**. 

Unlike desktop mirror projection or injector tools, **Arma 3 VR** hooks directly into Arma 3's native DirectX 11 rendering pipeline and passes real-time head pose data straight into the engine via a high-performance FreeTrack / TrackIR shared memory interface.

---

## Features

- **OpenXR Native**: Compatible with Meta Quest (Link / AirLink), Virtual Desktop, SteamVR (Valve Index, Vive), and Windows Mixed Reality headsets.
- **Direct DirectX 11 Interception**: Intercepts the engine's active render target and main scene depth buffer directly via context VTable hooking.
- **Depth-Based Stereoscopic Reprojection**: Custom HLSL screen-space depth reprojection shader generates genuine binocular 3D separation and convergence in real time.
- **2D / 3D Toggle**: Switch between full stereoscopic 3D and high-clarity 2D monoscopic view on the fly.
- **Sub-Millimeter FreeTrack Integration**: Feeds HMD rotational tracking directly into the game engine with zero external tracking applications required.
- **Automatic Lifecycle Cleanup**: Automatically detects mission end or graphics device resets to prevent stranded background sessions and memory leaks.

---

## Default In-Game Keybinds

Keybinds are fully customizable in **Options &rarr; Controls &rarr; Configure Addons &rarr; Arma 3 VR**.

| Action | Default Keybind | Description |
| :--- | :--- | :--- |
| **Toggle VR** | `Ctrl + V` | Starts or cleanly stops the OpenXR session and tracking loop |
| **Recenter** | `Numpad 5` | Re-centers head orientation to current view |
| **Toggle 3D / 2D** | `Numpad 6` | Toggles between Stereoscopic 3D and Monoscopic mode |
| **Separation (+)** | `Numpad +` | Increases 3D depth pop / stereo separation |
| **Separation (-)** | `Numpad -` | Decreases 3D depth separation |
| **Convergence (+)** | `Page Up` | Moves the focal plane forward |
| **Convergence (-)** | `Page Down` | Moves the focal plane backward |

---

## Installation & Usage

1. Download the latest release from the [Releases](https://github.com/YourUsername/Arma3VR/releases) page.
2. Extract the `@Arma3VR` folder into your Arma 3 game directory:
   ```text
   C:\Program Files (x86)\Steam\steamapps\common\Arma 3\@Arma3VR\

    Ensure your VR headset is connected and your runtime (Meta Quest Link, SteamVR, or Virtual Desktop) is active.

    Launch Arma 3 via the launcher, go to MODS, and check Arma 3 VR.

    Once in-game or in the Virtual Arsenal, press Ctrl + V to activate VR.

Architecture Overview

    RVExtension.cpp: Exposes Arma's standard C-ABI extension interface, parsing control commands from SQF.

    ArmaGraphics.cpp: Resolves engine memory pointers to get access to the primary ID3D11Device and ID3D11DeviceContext.

    DepthCaptureHook.cpp: Hooks virtual table slot #33 (OMSetRenderTargets) to intercept the full-resolution scene depth stencil view while ignoring shadow maps.

    StereoPipeline.cpp: Executes a screen-space depth-disparity pixel shader to generate distinct left and right eye images with custom separation and convergence.

    FreeTrackProvider.cpp: Simulates a native TrackIR device in shared memory (FT_SharedMem) to feed rotational telemetry into Arma.

Building from Source
Prerequisites

    Windows 10/11 64-bit

    Visual Studio 2022 (with Desktop development with C++)

    CMake 3.20 or newer

    OpenXR SDK (or install via vcpkg: vcpkg install openxr-sdk-windows:x64-windows)

Build Steps

    Clone the repository:
    code Bash

    git clone https://github.com/YourUsername/Arma3VR.git
    cd Arma3VR

    Run the automated PowerShell build script:
    code Powershell

    .\build.ps1 -Configuration Release

    The compiled mod and binaries will be staged into dist/@Arma3VR.


---

### 5. `LICENSE` (MIT Standard)

```text
MIT License

Copyright (c) 2026 Arma 3 VR Project Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

