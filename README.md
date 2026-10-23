# Arma 3 VR (OpenXR)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![OpenXR](https://img.shields.io/badge/OpenXR-1.0-blue.svg)](https://www.khronos.org/openxr/)
[![DirectX 11](https://img.shields.io/badge/DirectX-11-green.svg)](https://learn.microsoft.com/en-us/windows/win32/direct3d11/atpragma-intro)

Native OpenXR Virtual Reality mod for **Arma 3 (64-bit)**.

This mod hooks directly into Arma 3's DirectX 11 graphics pipeline to capture the active render target and scene depth buffer, passing real-time head rotational tracking straight into the engine using an integrated FreeTrack / TrackIR shared memory interface.

---

## Installation & Setup (For Players)

If you simply want to play without building anything from source:

### 1. Mod Setup
1. Download the latest **`@Arma3VR.zip`** from the **[Releases](../../releases)** tab.
2. Extract the archive somewhere on your system so that you have the **`@Arma3VR`** folder.
3. Ensure your VR headset is connected and your runtime (**Meta Quest Link / AirLink**, **Virtual Desktop**, or **SteamVR**) is running.
4. Launch the **Arma 3 Launcher**.
5. Click on the **MODS** tab on the left.
6. Click the **+ Local mod** button near the top of the launcher window.
7. Browse to and select the extracted **`@Arma3VR`** folder.
8. Locate **Arma 3 VR (OpenXR)** in your mod list and toggle the checkbox to **Enabled**.
9. Launch the game.

### 2. Enabling Head Tracking in Game (Required Once)
To allow the VR headset rotation to control your head in-game:
1. In Arma 3, go to **Options &rarr; Controls &rarr; Controllers** tab.
2. Under the controllers list, select **FreeTrack** and ensure it is set to **Enabled**.
3. Load into any mission or the Virtual Arsenal, and press **`Ctrl + V`** to activate VR mode.

> **Note (If "FreeTrack" is missing from the list):**  
> If FreeTrack does not appear in your Controllers menu, Windows has not yet registered the FreeTrack protocol registry entries.  
> To fix this permanently: Download and open [OpenTrack](https://github.com/opentrack/opentrack) once, set Output to *FreeTrack 2.0 Enhanced*, start it, and launch Arma 3 once. Once "FreeTrack" appears in Arma's Controllers list and is enabled, you can close and ignore OpenTrack—the mod feeds all rotational telemetry natively from that point forward.

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
│   ├── openxr/
│   │   ├── openxr.h
│   │   ├── openxr_platform.h
│   │   └── openxr_platform_defines.h
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
├── lib/
│   └── openxr_loader.lib
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
```

---

## Building from Source

### Prerequisites
* **Visual Studio 2022** (with *Desktop development with C++*)
* **CMake 3.20+**
* **Arma 3 (64-bit)**
* Any PBO packing tool (such as PBO Manager, Addon Builder, or HEMTT)

---

### Step 1: Clone the Repository

```powershell
git clone https://github.com/PsychicArmageddon7/Arma3VR_MOD-AI-SLOP.git
cd Arma3VR_MOD-AI-SLOP
```

---

### Step 2: Build the Extension DLL

Choose either method below:

* **Via Visual Studio 2022 (Recommended)**:  
  Open the repository folder using **File &rarr; Open &rarr; Folder...**, select **`x64-Release`** in the top configuration dropdown, and press **`Ctrl + Shift + B`** (or **Build &rarr; Build All**).

* **Via PowerShell Script**:  
  Open PowerShell in the repository root and run:
  ```powershell
  .\build.ps1 -Configuration Release
  ```

Either method compiles the C++ extension and writes **`Arma3VR_x64.dll`** directly into the local **`@Arma3VR/`** directory.

---

### Step 3: Pack the Addon PBO (Required)

Compiling only generates the DLL binary. Before Arma 3 can load the mod scripts in-game, you must package the addon:

1. Using your preferred PBO tool, pack the folder:
   ```text
   @Arma3VR/addons/main/
   ```
   into:
   ```text
   @Arma3VR/addons/main.pbo
   ```
2. *(Optional)* Once `main.pbo` is created, you can safely delete the unpacked `main/` folder so that only `main.pbo` remains inside `addons/`.

---

### Step 4: Load into Arma 3 & Enable Head Tracking

Your `@Arma3VR` folder is now complete and ready to use:

1. Launch the **Arma 3 Launcher**.
2. Go to the **MODS** tab &rarr; click **+ Local mod**.
3. Select the **`@Arma3VR`** folder from your repository (or copy it into your game root first).
4. Enable **Arma 3 VR (OpenXR)** in the mod list and launch the game.
5. In Arma 3, go to **Options &rarr; Controls &rarr; Controllers** tab.
6. Under the controllers list, select **FreeTrack** and ensure it is set to **Enabled**.
7. Load into any mission or the Virtual Arsenal, and press **`Ctrl + V`** to activate VR mode.

> **Note (If "FreeTrack" is missing from the list):**  
> If FreeTrack does not appear in your Controllers menu, Windows has not yet registered the FreeTrack protocol registry entries.  
> To fix this permanently: Download and open [OpenTrack](https://github.com/opentrack/opentrack) once, set Output to *FreeTrack 2.0 Enhanced*, start it, and launch Arma 3 once. Once "FreeTrack" appears in Arma's Controllers list and is enabled, you can close and ignore OpenTrack—the mod feeds all rotational telemetry natively from that point forward.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
