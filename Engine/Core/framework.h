#pragma once
#include "Windows.h"
#include "tchar.h"
#include <sstream>

#include "Engine/Core/Misc.h"
#include "Engine/Core/Time.h"

#ifdef USE_IMGUI
#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_internal.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];
#endif

#include <d3d11_1.h>
#include <wrl.h>
#include <dxgi1_6.h>

#include <vector>
#include <memory>

#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Rendering/Pipeline/RenderSystem.h"

#if 0
#ifdef _DEBUG
CONST LONG SCREEN_WIDTH{ 1280 };
CONST LONG SCREEN_HEIGHT{ 720 };
#else
CONST LONG SCREEN_WIDTH{ 1920 };
CONST LONG SCREEN_HEIGHT{ 1080 };
#endif

#ifdef _DEBUG
CONST BOOL FULLSCREEN{ FALSE };
#else
CONST BOOL FULLSCREEN{ TRUE };
#endif // _DEBUG  
#else
CONST LONG SCREEN_WIDTH{ 1920 };
CONST LONG SCREEN_HEIGHT{ 1080 };

#ifdef _DEBUG
CONST BOOL FULLSCREEN{ FALSE };
#else
CONST BOOL FULLSCREEN{ TRUE };
#endif // _DEBUG

#endif // 0



CONST LPCWSTR APPLICATION_NAME{ L"CurryEngine" };

class Framework
{
public:
    BOOL vsync{ TRUE };

    const float fixedTimeStep = 1.0f / 60;//固定更新間隔(FPS)
    float accumulatedTime = 0.0f;
    //更新間隔を固定長にするかどうか
//#define FIXED

#ifdef FIXED
    bool timerActive = true;
#endif

    size_t VideoMemoryUsage()
    {
        DXGI_QUERY_VIDEO_MEMORY_INFO video_memory_info;
        Graphics::GetAdapter()->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &video_memory_info);
        return video_memory_info.CurrentUsage / 1024 / 1024;
    }

	// 仮 RenderSystem
	std::unique_ptr<RenderSystem> renderSystem;

	// コンストラクタ・デストラクタ
    Framework(HWND hwnd);
    ~Framework();

    Framework(const Framework&) = delete;
    Framework& operator=(const Framework&) = delete;
    Framework(Framework&&) noexcept = delete;
    Framework& operator=(Framework&&) noexcept = delete;

    // メインループ
    int Run();

    // ウィンドウプロシージャ
    LRESULT CALLBACK HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
    
    static inline bool isPaused = false;// Profiler用
    static void SetPause(bool pause) {
        isPaused = pause;
    }
private:
    bool Initialize();
    void BeginFrame();
    void Update(float deltaTime/*Elapsed seconds from last frame*/);
    void Render(float deltaTime/*Elapsed seconds from last frame*/);
	void EndFrame();
    bool Uninitialize(HWND hwnd);

private:
    Time time;
    uint32_t frames{ 0 };
    float elapsedTime{ 0.0f };
    void calculate_frame_stats()
    {
        if (++frames, (time.TimeStamp() - elapsedTime) >= 1.0f)
        {
            float fps = static_cast<float>(frames);
            std::wostringstream outs;
            outs.precision(6);
#ifdef _DEBUG
            outs << APPLICATION_NAME << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
#else
            outs << APPLICATION_NAME;
#endif // _DEBUG
            SetWindowTextW(Graphics::GetHwnd(), outs.str().c_str());

            frames = 0;
            elapsedTime += 1.0f;
        }
    }
};