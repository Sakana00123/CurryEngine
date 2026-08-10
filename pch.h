#pragma once

// C++標準ライブラリ（よく使うもの）
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <functional>
#include <algorithm>

// Windows / DirectX
#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h> // Microsoft::WRL::ComPtr
#include <d3dcompiler.h>

// ImGui
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

// Tracy
#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif // TRACY_ENABLE

// 他にほぼ全体で使うヘッダーがあればここに追加
#include "Engine/Common/EngineCommon.h"

#ifndef ENGINE_API
#define ENGINE_API extern "C" __declspec(dllexport)
#endif
