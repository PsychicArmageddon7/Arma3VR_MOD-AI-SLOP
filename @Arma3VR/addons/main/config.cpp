class CfgPatches
{
    class A3VR_Main
    {
        name = "Arma 3 VR";
        author = "Arma 3 VR Project";
        url = "";
        units[] = {};
        weapons[] = {};
        requiredVersion = 2.06;
        requiredAddons[] = {"A3_Data_F"};
    };
};

class CfgFunctions
{
    class A3VR
    {
        tag = "A3VR";
        class Core
        {
            file = "\main\functions";
            class startVR {};
            class stopVR {};
            class toggleVR {};
            class recenter {};
            class toggleMode {};
            class adjustSeparation {};
            class adjustConvergence {};
        };
    };
};

class CfgUserActions
{
    class A3VR_ToggleVR
    {
        displayName = "Toggle VR Mode";
        tooltip = "Starts or stops the OpenXR VR loop";
        onActivate = "call A3VR_fnc_toggleVR";
        modifierBlocking = 1;
    };

    class A3VR_Recenter
    {
        displayName = "Recenter Tracking";
        tooltip = "Resets head orientation to center";
        onActivate = "call A3VR_fnc_recenter";
        modifierBlocking = 1;
    };

    class A3VR_ToggleMode
    {
        displayName = "Toggle 3D Stereo / 2D Mono";
        tooltip = "Toggles between 3D stereoscopic depth and 2D monoscopic view";
        onActivate = "call A3VR_fnc_toggleMode";
        modifierBlocking = 1;
    };

    class A3VR_SepUp
    {
        displayName = "Increase 3D Separation";
        tooltip = "Increases stereoscopic 3D strength/pop";
        onActivate = "['+'] call A3VR_fnc_adjustSeparation";
        modifierBlocking = 1;
    };

    class A3VR_SepDown
    {
        displayName = "Decrease 3D Separation";
        tooltip = "Decreases stereoscopic 3D separation for tighter silhouettes";
        onActivate = "['-'] call A3VR_fnc_adjustSeparation";
        modifierBlocking = 1;
    };

    class A3VR_ConvUp
    {
        displayName = "Increase Convergence";
        tooltip = "Shifts 3D focal convergence forward";
        onActivate = "['+'] call A3VR_fnc_adjustConvergence";
        modifierBlocking = 1;
    };

    class A3VR_ConvDown
    {
        displayName = "Decrease Convergence";
        tooltip = "Shifts 3D focal convergence backward";
        onActivate = "['-'] call A3VR_fnc_adjustConvergence";
        modifierBlocking = 1;
    };
};

class CfgDefaultKeysPresets
{
    class Arma3
    {
        class Mappings
        {
            A3VR_ToggleVR[] = { 0x1D13002F }; // Ctrl + V
            A3VR_Recenter[] = { 0x4C };       // Numpad 5
            A3VR_ToggleMode[] = { 0x4D };     // Numpad 6
            A3VR_SepUp[] = { 0x4E };          // Numpad +
            A3VR_SepDown[] = { 0x4A };        // Numpad -
            A3VR_ConvUp[] = { 0xC9 };         // Page Up
            A3VR_ConvDown[] = { 0xD1 };       // Page Down
        };
    };

    class Arma2
    {
        class Mappings
        {
            A3VR_ToggleVR[] = { 0x1D13002F }; // Ctrl + V
            A3VR_Recenter[] = { 0x4C };       // Numpad 5
            A3VR_ToggleMode[] = { 0x4D };     // Numpad 6
            A3VR_SepUp[] = { 0x4E };          // Numpad +
            A3VR_SepDown[] = { 0x4A };        // Numpad -
            A3VR_ConvUp[] = { 0xC9 };         // Page Up
            A3VR_ConvDown[] = { 0xD1 };       // Page Down
        };
    };
};

class UserActionGroups
{
    class A3VR_Category
    {
        name = "Arma 3 VR";
        isAddon = 1;
        group[] = {
            "A3VR_ToggleVR",
            "A3VR_Recenter",
            "A3VR_ToggleMode",
            "A3VR_SepUp",
            "A3VR_SepDown",
            "A3VR_ConvUp",
            "A3VR_ConvDown"
        };
    };
};