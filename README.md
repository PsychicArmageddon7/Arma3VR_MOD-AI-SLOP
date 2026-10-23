# Arma 3 VR (OpenXR)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![OpenXR](https://img.shields.io/badge/OpenXR-1.0-blue.svg)](https://www.khronos.org/openxr/)
[![DirectX 11](https://img.shields.io/badge/DirectX-11-green.svg)](https://learn.microsoft.com/en-us/windows/win32/direct3d11/atpragma-intro)

Native OpenXR Virtual Reality mod for **Arma 3 (64-bit)**.

This mod hooks directly into Arma 3's DirectX 11 graphics pipeline to capture the active render target and scene depth buffer, passing real-time head rotational tracking straight into the engine using an integrated FreeTrack / TrackIR shared memory interface.

---

## Installation & Setup (For Players)

If you simply want to play without building anything from source:

1. Download the latest **`@Arma3VR.zip`** from the **[Releases](../../releases)** tab.
2. Extract the archive somewhere on your system so that you have the **`@Arma3VR`** folder.
3. Ensure your VR headset is connected and your runtime (**Meta Quest Link / AirLink**, **Virtual Desktop**, or **SteamVR**) is running.
4. Launch the **Arma 3 Launcher**.
5. Click on the **MODS** tab on the left.
6. Click the **+ Local mod** button near the top of the launcher window.
7. Browse to and select the extracted **`@Arma3VR`** folder.
8. Locate **Arma 3 VR (OpenXR)** in your mod list and toggle the checkbox to **Enabled**.
9. Launch the game, load into any mission or the Virtual Arsenal, and press **`Ctrl + V`** to activate VR mode.

---

## Default Controls

Keybindings can be customized in-game via **Options &rarr; Controls &rarr; Configure Addons &rarr; Arma 3 VR**.

| Action | Default Keybind | Description |
| :--- | :--- | :--- |
| **Toggle VR** | `Ctrl + V` | Starts or cleanly stops the VR session and tracking loop |
| **Recenter** | `Numpad 5` | Re-centers head orientation to current view |
| **Toggle 3D / 2D** | `Numpad 6` | Toggles between Stereoscopic 3D and Monoscopic mode |
| **Separation (+)** | `Numpad +` | Increases 3D stereoscopic depth separation |
| **Separation (-)** | `Numpad -` | Decreases 3D stereoscopic depth separation |
| **Convergence (+)** | `Page Up` | Shifts the 3D focal convergence forward |
| **Convergence (-)** | `Page Down` | Shifts the 3D focal convergence backward |

---

## Repository Structure (For Modders & Developers)

```text
Arma3VR/
├── include/
│   ├── Common.h
│   ├── Extension/
│   │   └── RVExtension.h
│   ├── Graphics/
│   │   ├── ArmaGraphics.h
│   │   ├── DepthCaptureHook.h
│   │   └── StereoPipeline.h
│   ├── Tracking/
│   │   └── FreeTrackProvider.h
│   └── VR/
│       └── OpenXRContext.h
├── src/
│   ├── dllmain.cpp
│   ├── Extension/
│   │   └── RVExtension.cpp
│   ├── Graphics/
│   │   ├── ArmaGraphics.cpp
│   │   ├── DepthCaptureHook.cpp
│   │   └── StereoPipeline.cpp
│   ├── Tracking/
│   │   └── FreeTrackProvider.cpp
│   └── VR/
│       └── OpenXRContext.cpp
├── @Arma3VR/
│   ├── mod.cpp
│   ├── openxr_loader.dll
│   └── addons/
│       └── main/
│           ├── $PBOPREFIX$
│           ├── config.cpp
│           └── functions/
│               ├── fn_adjustConvergence.sqf
│               ├── fn_adjustSeparation.sqf
│               ├── fn_recenter.sqf
│               ├── fn_startVR.sqf
│               ├── fn_stopVR.sqf
│               ├── fn_toggleMode.sqf
│               └── fn_toggleVR.sqf
├── .gitignore
├── CMakeLists.txt
├── build.ps1
├── LICENSE
└── README.md

Building & Modifying from Source
Prerequisites

    Visual Studio 2022 (with Desktop development with C++)

    CMake 3.20+

    Arma 3 (64-bit)

Building the Extension DLL

    Clone this repository:
    code Powershell

    git clone https://github.com/PsychicArmageddon7/Arma3VR_MOD-AI-SLOP.git
    cd Arma3VR_MOD-AI-SLOP

    Build via the provided PowerShell script:
    code Powershell

    .\build.ps1 -Configuration Release

    Or open the folder directly in Visual Studio 2022 as a CMake project and build Arma3VR_x64.

    The resulting Arma3VR_x64.dll will be written directly into the local @Arma3VR/ folder.

In-Place Development (No Packing Required)

You do not need to pack a .pbo file every time you make changes to SQF or configuration files while developing:

    Enable the -filePatching parameter in the Arma 3 Launcher under Parameters → Advanced → Developer.

    Add @Arma3VR as a local mod. Arma 3 will read the unpacked scripts directly from @Arma3VR/addons/main/.

Packaging for Release

When you are ready to prepare a build for others:

    Pack @Arma3VR/addons/main/ into @Arma3VR/addons/main.pbo using your preferred PBO tool (such as Addon Builder, HEMTT, or PBO Manager).

    Zip the entire @Arma3VR folder.

    Attach the zip to a new release on GitHub.

License

This project is licensed under the MIT License - see the LICENSE file for details.
