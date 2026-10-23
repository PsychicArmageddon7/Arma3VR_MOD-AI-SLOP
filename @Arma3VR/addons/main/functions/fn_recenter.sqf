if !(missionNamespace getVariable ["A3VR_isActive", false]) exitWith {};

private _res = "Arma3VR" callExtension "recenter";
systemChat format ["[Arma3VR] Headset %1.", _res];