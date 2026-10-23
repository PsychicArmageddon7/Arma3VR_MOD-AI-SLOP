if (missionNamespace getVariable ["A3VR_isActive", false]) then {
    call A3VR_fnc_stopVR;
} else {
    call A3VR_fnc_startVR;
};