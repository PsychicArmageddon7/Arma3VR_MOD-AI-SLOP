if !(missionNamespace getVariable ["A3VR_isActive", false]) exitWith {};

// Remove frame loop
private _frameHandle = missionNamespace getVariable ["A3VR_FrameLoop", -1];
if (_frameHandle != -1) then {
    removeMissionEventHandler ["EachFrame", _frameHandle];
};

// Remove mission-end watchdog
private _endHandle = missionNamespace getVariable ["A3VR_EndLoop", -1];
if (_endHandle != -1) then {
    removeMissionEventHandler ["Ended", _endHandle];
};

// Shut down OpenXR and remove DirectX hooks in extension
private _shutdownStatus = "Arma3VR" callExtension "shutdown";
diag_log format ["[Arma3VR] %1", _shutdownStatus];

missionNamespace setVariable ["A3VR_isActive", false];
missionNamespace setVariable ["A3VR_FrameLoop", -1];
missionNamespace setVariable ["A3VR_EndLoop", -1];

systemChat "[Arma3VR] VR Mode Stopped.";