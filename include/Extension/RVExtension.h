#pragma once

#ifdef ARMA3VR_EXTENSION_EXPORTS
#define ARMA3VR_API extern "C" __declspec(dllexport)
#else
#define ARMA3VR_API extern "C" __declspec(dllimport)
#endif

ARMA3VR_API void __stdcall RVExtensionVersion(char* output, unsigned int outputSize);
ARMA3VR_API void __stdcall RVExtension(char* output, unsigned int outputSize, const char* function);