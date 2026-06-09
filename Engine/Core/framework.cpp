#include "pch.h"
#include "framework.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Physics/Physics.h"
#include "Engine/Events/EventSystem.h"
#include "Engine/Core/PersistentObjectManager.h"

#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

#include "Engine/Core/GameObject.h"

#include "Engine/Audio/Audio.h"

#include "Engine/Rendering/Pipeline/Graphics.h"

#include "Engine/Resources/ResourceManager.h"

#include "Engine/Rendering/Pipeline/RenderContext.h"

#include "Engine/Effects/EffectManager.h"
#include "Engine/Editor/EffectEditor.h"

#include "Engine/Editor/History.h"
#include "Engine/Editor/ImGuiTheme.h"

#include <profiler.h>
#include "Generated/ReflectionGenerated.h"
#include "Engine/Rendering/Camera/CameraSystem.h"
#include "Engine/Rendering/Camera/CameraComponent.h"
#include "Engine/Rendering/Camera/EditorCamera.h"
#include <implot.h>

#include "Engine/Scripting/ScriptSystem.h"
#include "Engine/Scripting/ScriptHost.h"

#include "Engine/Rendering/Renderers/DebugRenderer.h"
#include "Engine/EditorSupport/VcxprojHelper.h"

#include "Engine/Rendering/Pipeline/RenderSystem.h"

#include "Engine/EditorConfig/EditorConfigManager.h"


CONST LONG SHADOWMAP_WIDTH{ 2048 };
CONST LONG SHADOWMAP_HEIGHT{ 2048 };


Framework::Framework(HWND hwnd)
{
    //プロファイラ初期化
    ProfileInitialize(&isPaused, Framework::SetPause, ImGuiControl::Profiler::DefaultMaxThreads);
    ProfileThreadName(0, "Main Thread");

    Graphics::Initialize(hwnd, FULLSCREEN);
    InputSystem::Initialize();
    //EffectManager::Initialize();

    ResourceManager::Initialize();

	DebugRenderer::Initialize();

	Physics::Initialize();

    ScriptSystem::Initialize();

    AssetBrowser::Initialize();
    AssetBrowser::InitializeDropTarget(hwnd);


#ifdef _DEBUG
    EditorConfigManager::Initialize();
#endif // _DEBUG
}

bool Framework::Initialize()
{
    HRESULT hr{ S_OK };
    auto device = Graphics::GetDevice();

	// レンダリングシステム
	renderSystem = new RenderSystem;
	renderSystem->Initialize(&time);

    //Audio
    Audio::Initialize();

    SceneManager::Initialize();
    //SceneManager::SetLoadingScene("EmptyScene");
    //SceneManager::ChangeScene("SampleScene");

    // エフェクトマネージャー初期化
    EffectManager::Initialize();

    //エフェクトエディタ初期化
    EffectEditor::Initialize();

    // エディタカメラ初期化
    EditorCamera::Initialize();

	// 最初のシーンをロード
	SceneManager::LoadFirstScene();

    return true;
}

int Framework::Run()
{
    MSG msg{};

    if (!Initialize())
    {
        return 0;
    }

#ifdef USE_IMGUI
    IMGUI_CHECKVERSION();
    ImGuiContext* imguiContext = ImGui::CreateContext();
    //ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.0f, nullptr, glyphRangesJapanese);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(".\\Data\\Fonts\\NotoSansJP-Medium.ttf", 18.0f, nullptr, glyphRangesJapanese);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplWin32_Init(Graphics::GetHwnd());
    ID3D11Device* device = Graphics::GetDevice();
    ID3D11DeviceContext* immediate_context = Graphics::GetDeviceContext();
    ImGui_ImplDX11_Init(device/*.Get()*/, immediate_context/*.Get()*/);
    ImGui::StyleColorsDark();

	// ImPlot初期化
	ImPlotContext* imPlotContext = ImPlot::CreateContext();
	ImPlot::SetImGuiContext(imguiContext);

    
    // ImGuiテーマ初期化
    ImGuiTheme::Initialize();
#endif

    //初期化
    time.Reset();

    //メインループ
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            time.Tick();

#ifdef FIXED
            if (timerActive)
            {
                accumulatedTime += time.UnscaledDeltaTime();
                while (accumulatedTime >= fixedTimeStep)
                {
                    calculate_frame_stats();
                    Update(fixedTimeStep);
                    Render(fixedTimeStep);
                    accumulatedTime -= fixedTimeStep;
                }
            }
#else
            CalculateFrameStatus();
            BeginFrame();
            Update(time.DeltaTime());
            Render(time.UnscaledDeltaTime());

#endif      
            EndFrame();
        }
    }

#ifdef USE_IMGUI
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif

    return Uninitialize(msg.hwnd) ? static_cast<int>(msg.wParam) : 0;
}

size_t Framework::VideoMemoryUsage()
{
    DXGI_QUERY_VIDEO_MEMORY_INFO video_memory_info;
    Graphics::GetAdapter()->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &video_memory_info);
    return video_memory_info.CurrentUsage / 1024 / 1024;
}

void Framework::CalculateFrameStatus()
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

LRESULT CALLBACK Framework::HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef USE_IMGUI
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return true; }
#endif
    if (InputSystem::HandleMessage(hwnd, msg, wparam, lparam)) { return true; }

    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        BeginPaint(hwnd, &ps);

        EndPaint(hwnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CREATE:
        break;
    case WM_KEYDOWN:
        if (wparam == VK_ESCAPE)
        {
            //PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
        break;
    case WM_ENTERSIZEMOVE:
        time.Stop();
#ifdef FIXED
        timerActive = false;
#endif
        break;
    case WM_EXITSIZEMOVE:
        time.Start();
#ifdef FIXED
        timerActive = true;
#endif
        break;
    case WM_SIZE:
    {
#if 1
        RECT client_rect{};
        GetClientRect(hwnd, &client_rect);
        Graphics::OnSizeChanged(static_cast<UINT64>(client_rect.right - client_rect.left), static_cast<UINT64>(client_rect.bottom - client_rect.top));
		renderSystem->OnSizeChanged(Graphics::GetDevice(), static_cast<uint32_t>(client_rect.right - client_rect.left), static_cast<uint32_t>(client_rect.bottom - client_rect.top));
#endif
        break;
    }
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    return 0;
}

void Framework::BeginFrame()
{
	// フレーム開始処理
    {
         SceneManager::BeginFrame();
	}
}

void Framework::EndFrame()
{
    // フレーム終了処理
    {
        InputSystem::EndFrame();
        SceneManager::EndFrame();
    }
}

void Framework::Update(float deltaTime/*Elapsed seconds from last frame*/)
{
#ifdef USE_IMGUI
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    ProfileNewFrame();
#endif
    // Audio更新
    Audio::Update(deltaTime);

	//// リソースマネージャ更新
 //   {
 //       ProfileScopedSection_2(0, "ResourceManager::Update", ImGuiControl::Profiler::Yellow);
 //       ResourceManager::Update();
 //   }

    // 入力システム更新
    if (GetForegroundWindow() == Graphics::GetHwnd())
    {
        ProfileScopedSection_2(0, "InputSystem::Update", ImGuiControl::Profiler::Green);
        InputSystem::Update(Time::UnscaledDeltaTime());
    }
#ifdef _DEBUG
	//else if (!AssetBrowser::IsExternalDragActive()) // フォーカスがない場合は、SleepしてCPU使用率を下げる(Debug時のみ)
 //   {
 //       Sleep(100);
 //   }
#endif // _DEBUG


    // イベントシステム更新
    if ((SceneManager::GetCurrentScene() && SceneManager::GetCurrentScene()->IsStarted()) && SceneManager::state == SceneManager::State::Playing)
    {
        ProfileScopedSection_2(0, "EventSystem::Update", ImGuiControl::Profiler::Yellow);
        EventSystem::Update(Time::UnscaledDeltaTime());
    }

    // ウィンドウ切替
#ifdef _DEBUG
    if (InputSystem::GetInputState("Alt") && InputSystem::GetInputState("Enter", InputStateMask::Trigger))
    {
        Graphics::StylizeWindow(!Graphics::GetFullScreenMode());
    }
#endif // _DEBUG


    // シーン更新
    {
        ProfileScopedSection_2(0, "SceneManager::Update", ImGuiControl::Profiler::Red);
        SceneManager::Update(deltaTime);
    }

    // パーティクルシステム更新
    {
        ProfileScopedSection_2(0, "EffectManager::Update", ImGuiControl::Profiler::Blue);
        // エフェクトマネージャ更新
        EffectManager::Update(deltaTime);
    }

	// エディタカメラ更新
#ifdef _DEBUG
    {
        ProfileScopedSection_2(0, "EditorCamera::Update", ImGuiControl::Profiler::Green);
        EditorCamera::Update(deltaTime);
    }
#endif // _DEBUG
}

void Framework::Render(float deltaTime/*Elapsed seconds from last frame*/)
{
	// 描画処理
	renderSystem->Render();

    //vsyncがtrueの場合、描画間隔が固定フレームレートで動作するようになる
    Graphics::Present(vsync);

    // 共有リソースのリセット
    Graphics::ResetSharedResources();
}

bool Framework::Uninitialize(HWND hwnd)
{
#ifdef USE_IMGUI
    // ImPlot終了
    ImPlot::DestroyContext();
#endif // USE_IMGUI

	// VcxprojHelperの保留中のシェーダー登録処理を完了させる
	VcxprojHelper::ProcessPendingShaderRegistrations();
	// VcxprojHelperの保留中のシェーダー登録解除処理を完了させる
	VcxprojHelper::ProcessPendingShaderUnregistrations();

	// アセットブラウザのドロップターゲット終了
	AssetBrowser::FinalizeDropTarget(hwnd);
	
    // レンダリングシステム終了
	renderSystem->Finalize();
	if (renderSystem) { delete renderSystem; renderSystem = nullptr; }

    // スクリプトシステム終了
    ScriptSystem::Shutdown();

    //プロファイラ終了
    ProfileShutdown();

    // エフェクトマネージャー終了
    EffectManager::ClearAll();

    //Audio解放
    Audio::ClearAll();

    PersistentObjectManager::Clear();

    SceneManager::Finalize();

	Physics::Terminate();

    ResourceManager::Finalize();
    
    InputSystem::Finalize();

    //終了化
    Graphics::Finalize();

    //コンソール終了
    Console::Shutdown();

	// EditorConfigManager終了
#ifdef _DEBUG
	EditorConfigManager::Shutdown();
#endif // _DEBUG

    return true;
}

Framework::~Framework()
{

}