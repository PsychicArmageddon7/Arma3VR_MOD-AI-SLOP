#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#ifndef XR_USE_GRAPHICS_API_D3D11
#define XR_USE_GRAPHICS_API_D3D11
#endif
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cmath>

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;