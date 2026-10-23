if !(missionNamespace getVariable ["A3VR_isActive", false]) exitWith {};

private _mode = "Arma3VR" callExtension "mode";
systemChat format ["[Arma3VR] %1", _mode];