# Arma 3 VR (OpenXR Native Mod)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![OpenXR](https://img.shields.io/badge/OpenXR-1.0-blue.svg)](https://www.khronos.org/openxr/)
[![DirectX 11](https://img.shields.io/badge/DirectX-11-green.svg)](https://docs.microsoft.com/en-us/windows/win32/direct3d11/atpragma-intro)

Native OpenXR Virtual Reality implementation for **Arma 3 (64-bit)**. 

Hooks directly into Arma 3's DirectX 11 rendering pipeline to capture the active swapchain and scene depth buffer, passing real-time head rotation into the engine via a FreeTrack / TrackIR shared memory interface.

---

## Quick Start (For Players)

If you just want to play the mod without compiling anything:

1. Download the latest **`Arma3VR.zip`** from the **[Releases](https://github.com/YourUsername/Arma3VR/releases)** tab.
2. Extract the `@Arma3VR` folder directly into your Arma 3 game directory:
   ```text
   C:\Program Files (x86)\Steam\steamapps\common\Arma 3\@Arma3VR\

    Make sure your VR headset runtime (Meta Quest Link / AirLink, Virtual Desktop, or SteamVR) is active.

    Open the Arma 3 Launcher, go to MODS, and enable Arma 3 VR.

    Launch the game and press Ctrl + V to activate VR mode.

Default Controls

Keybinds can be rebound in Options → Controls → Configure Addons → Arma 3 VR.
Action	Default Keybind	Description
Toggle VR	Ctrl + V	Starts or cleanly stops the VR render loop
Recenter	Numpad 5	Re-centers head orientation
Toggle 3D / 2D	Numpad 6	Switches between Stereoscopic 3D and Monoscopic mode
Separation (+)	Numpad +	Increases 3D depth pop / stereo separation
Separation (-)	Numpad -	Decreases 3D depth separation
Convergence (+)	Page Up	Shifts the 3D focal plane forward
Convergence (-)	Page Down	Shifts the 3D focal plane backward
Repository Structure (For Modders & Developers)
code Text

Arma3VR/
├── include/              # C++ headers
├── src/                  # C++ source code (DirectX hook, OpenXR context, FreeTrack)
├── @Arma3VR/             # Raw mod directory
│   ├── mod.cpp
│   ├── openxr_loader.dll
│   └── addons/
│       └── main/         # Unpacked SQF scripts and config.cpp
├── CMakeLists.txt        # CMake build file
├── build.ps1             # Local PowerShell compilation script
└── README.md

Developing Locally

    Building the DLL:
    Open the repository folder in Visual Studio 2022 (as a CMake project) or run:
    code Powershell

    .\build.ps1 -Configuration Release

    The compiled Arma3VR_x64.dll will be output directly into @Arma3VR\.

    Testing Changes in Arma 3:
    You do not need to pack a PBO while developing. Launch Arma 3 with the -filePatching parameter enabled in the launcher. Arma will read the loose SQF files from @Arma3VR\addons\main\ immediately.

    Packaging for Release:
    Pack @Arma3VR\addons\main\ into main.pbo using your preferred PBO tool (PBO Manager, Addon Builder, or HEMTT), zip the @Arma3VR directory, and attach it to a GitHub Release.

License

This project is licensed under the MIT License - see the LICENSE file for details.
code Code

---

### What to Keep in Git Going Forward
* Keep your **`src/`**, **`include/`**, and **`CMakeLists.txt`**.
* Keep **`@Arma3VR/mod.cpp`**, **`@Arma3VR/openxr_loader.dll`**, and **`@Arma3VR/addons/main/`** (the SQF source).
* Whenever you release a new version, compile it locally, pack `main.pbo`, zip `@Arma3VR`, and drop it on the **Releases** tab.
