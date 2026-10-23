params [["_dir", "+"]];

if !(missionNamespace getVariable ["A3VR_isActive", false]) exitWith {};

private _res = "Arma3VR" callExtension format ["conv;%1", _dir];
systemChat format ["[Arma3VR] %1", _res];