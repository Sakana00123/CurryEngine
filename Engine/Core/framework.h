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
/**
 * @brief ImGui_ImplWin32_WndProcHandler の処理を行います。
 */
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];
#endif

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

/** @brief Framework を表すクラスです。 */
class Framework
{
public:
    BOOL vsync{ FALSE };

    const float fixedTimeStep = 1.0f / 60;//固定更新間隔(FPS)
    float accumulatedTime = 0.0f;
    //更新間隔を固定長にするかどうか
//#define FIXED

#ifdef FIXED
    bool timerActive = true;
#endif

	// ビデオメモリ使用量をMB単位で取得
    /**
     * @brief VideoMemoryUsage の処理を行います。
     */
    size_t VideoMemoryUsage();

	// 仮 RenderSystem
	class RenderSystem* renderSystem;

	// コンストラクタ・デストラクタ
    /**
     * @brief Framework を構築します。
     */
    Framework(HWND hwnd);
    /**
     * @brief Framework を破棄します。
     */
    ~Framework();

    /**
     * @brief Framework を構築します。
     */
    Framework(const Framework&) = delete;
    /**
     * @brief 演算子処理を行います。
     */
    Framework& operator=(const Framework&) = delete;
    /**
     * @brief Framework を構築します。
     */
    Framework(Framework&&) noexcept = delete;
    /**
     * @brief 演算子処理を行います。
     */
    Framework& operator=(Framework&&) noexcept = delete;

    // メインループ
    /**
     * @brief Run の処理を行います。
     */
    int Run();

    // ウィンドウプロシージャ
    /**
     * @brief HandleMessage の処理を行います。
     */
    LRESULT CALLBACK HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
    /**
     * @brief 初期化します。
     */
    bool Initialize();
    /**
     * @brief BeginFrame の処理を行います。
     */
    void BeginFrame();
    /**
     * @brief 状態を更新します。
     */
    void Update(float deltaTime/*Elapsed seconds from last frame*/);
    /**
     * @brief 描画処理を行います。
     */
    void Render(float deltaTime/*Elapsed seconds from last frame*/);
	/**
	 * @brief EndFrame の処理を行います。
	 */
	void EndFrame();
    /**
     * @brief Uninitialize の処理を行います。
     */
    bool Uninitialize(HWND hwnd);

private:
    Time time;
    uint32_t frames{ 0 };
    float elapsedTime{ 0.0f };
    /**
     * @brief CalculateFrameStatus の処理を行います。
     */
    void CalculateFrameStatus();
    
};
