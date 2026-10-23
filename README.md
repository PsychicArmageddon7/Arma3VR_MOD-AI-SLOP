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
