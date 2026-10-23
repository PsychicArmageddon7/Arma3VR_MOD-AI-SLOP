if (missionNamespace getVariable ["A3VR_isActive", false]) exitWith {
    systemChat "[Arma3VR] VR is already active.";
};

systemChat "[Arma3VR] Initializing OpenXR...";
private _init = "Arma3VR" callExtension "initialize";

// Checks for success response from refactored C++ extension ("OpenXR initialized.")
if !("OpenXR initialized" in _init) exitWith {
    systemChat format ["[Arma3VR Error] Initialization failed: %1", _init];
    diag_log format ["[Arma3VR Error] Initialization failed: %1", _init];
};

systemChat "[Arma3VR] Starting VR session...";
private _session = "Arma3VR" callExtension "session";

// Checks for success response from refactored C++ extension ("Session started.")
if !("Session started" in _session) exitWith {
    systemChat format ["[Arma3VR Error] Session creation failed: %1", _session];
    diag_log format ["[Arma3VR Error] Session creation failed: %1", _session];
};

// Main render & tracking pump
private _frameHandle = addMissionEventHandler ["EachFrame", {
    private _status = "Arma3VR" callExtension "step";
    
    // Silently log non-fatal and unexpected errors to RPT without causing frame stutters
    if (!("OK" in _status) && {_status != "OpenXR session is not running"}) then {
        diag_log format ["[Arma3VR Error] %1", _status];
    };
}];

// Auto-cleanup hook: stops VR and releases DirectX resources if mission ends or restarts
private _endHandle = addMissionEventHandler ["Ended", {
    call A3VR_fnc_stopVR;
}];

missionNamespace setVariable ["A3VR_FrameLoop", _frameHandle];
missionNamespace setVariable ["A3VR_EndLoop", _endHandle];
missionNamespace setVariable ["A3VR_isActive", true];

systemChat "[Arma3VR] VR Mode Active! [Ctrl+V] Toggle, [Num 5] Recenter, [Num 6] Mode.";