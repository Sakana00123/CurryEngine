#include "pch.h"
#include "RenderSystem.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

#include "Engine/Core/Time.h"

#include "Engine/Editor/EditorGUI.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Rendering/Camera/EditorCamera.h"
#include "Engine/Rendering/Camera/CameraComponent.h"

#include "Engine/Editor/History.h"
#include "Engine/Editor/ImGuiTheme.h"

#ifdef USE_IMGUI
#include <imgui.h>
#include <ImGuizmo.h>
#include <profiler.h>
#include <imgui_internal.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#endif

#include <Engine\Physics\Physics.h>
#include <Engine\Editor\EffectEditor.h>
#include <Engine\Editor\ImportSettings\ImportSettingsWindow.h>

void RenderSystem::Initialize(Time* time)
{
	// シーンビューの描画パイプラインを作成して初期化
	sceneRenderPipeline = std::make_unique<SceneRenderPipeline>();
	sceneRenderPipeline->SetupRenderPasses();
	sceneRenderPipeline->Initialize();

	// ゲームビューの描画パイプラインを作成して初期化
	gameRenderPipeline = std::make_unique<GameRenderPipeline>();
	gameRenderPipeline->SetupRenderPasses();
	gameRenderPipeline->Initialize();

	// プレビュー用の描画パイプラインを作成して初期化
	previewRenderPipeline = std::make_unique<PreviewRenderPipeline>();
	previewRenderPipeline->SetupRenderPasses();
	previewRenderPipeline->Initialize();

	// エフェクトプレビュー用の描画パイプラインを作成して初期化
	effectPreviewRenderPipeline = std::make_unique<EffectPreviewRenderPipeline>();
	effectPreviewRenderPipeline->SetupRenderPasses();
	effectPreviewRenderPipeline->Initialize();

	this->time = time;
}

void RenderSystem::Render()
{
    // カメラ情報
    Vector3 cameraPos = { 0,0,0 };
    XMMATRIX View = XMMatrixIdentity();
    XMMATRIX Projection = XMMatrixIdentity();
    bool acceptRendering = true;
    RenderContext sceneContext(Graphics::GetDeviceContext(), Graphics::fullScreenQuad.get(), Graphics::GetSharedResources());
    RenderContext gameContext(Graphics::GetDeviceContext(), Graphics::fullScreenQuad.get(), Graphics::GetSharedResources());
	RenderContext previewContext(Graphics::GetDeviceContext(), Graphics::fullScreenQuad.get(), Graphics::GetSharedResources());
	RenderContext effectPreviewContext(Graphics::GetDeviceContext(), Graphics::fullScreenQuad.get(), Graphics::GetSharedResources());
    // カメラ情報取得
    {
		auto scene = SceneManager::GetCurrentScene();
		if (scene && scene->IsStarted())
        {
            //新カメラシステム
#ifdef _DEBUG
            {
                ProfileScopedSection_2(0, "SceneView::Rendering", ImGuiControl::Profiler::Yellow);
                // エディタビュー用の描画処理
				EditorCamera* editorCamera = scene->GetEditorCamera(EDITOR_CAMERA_SCENE_VIEW);
                cameraPos = editorCamera->GetPosition();
                View = editorCamera->GetViewMatrix();
                Projection = editorCamera->GetProjectionMatrix();

                // RenderContextの設定
                {
                    sceneContext.renderState = Graphics::GetRenderState();
                    sceneContext.deltaTime = Time::DeltaTime();
                    sceneContext.unscaledDeltaTime = Time::UnscaledDeltaTime();
                    sceneContext.totalTime = time->TimeStamp();

                    sceneContext.cameraPosition = cameraPos;
                    DirectX::XMStoreFloat4x4(&sceneContext.view, View);
                    DirectX::XMStoreFloat4x4(&sceneContext.projection, Projection);
                    auto ViewProjection = View * Projection;
                    DirectX::XMStoreFloat4x4(&sceneContext.viewProjection, ViewProjection);
                    DirectX::XMStoreFloat4x4(&sceneContext.inverseView, XMMatrixInverse(nullptr, View));
                    DirectX::XMStoreFloat4x4(&sceneContext.inverseProjection, XMMatrixInverse(nullptr, Projection));
                    DirectX::XMStoreFloat4x4(&sceneContext.inverseViewProjection, XMMatrixInverse(nullptr, ViewProjection));
                }
                // エディタビュー用の描画処理
                sceneRenderPipeline->Execute(&sceneContext, scene);
            }

            // プレビュー用の描画処理
            {
				EditorCamera* editorCamera = scene->GetEditorCamera(EDITOR_CAMERA_PREVIEW);
                cameraPos = editorCamera->GetPosition();
                View = editorCamera->GetViewMatrix();
                Projection = editorCamera->GetProjectionMatrix();

                // RenderContextの設定
                {
                    previewContext.renderState = Graphics::GetRenderState();
                    previewContext.deltaTime = Time::DeltaTime();
                    previewContext.unscaledDeltaTime = Time::UnscaledDeltaTime();
                    previewContext.totalTime = time->TimeStamp();

                    previewContext.cameraPosition = cameraPos;
                    DirectX::XMStoreFloat4x4(&previewContext.view, View);
                    DirectX::XMStoreFloat4x4(&previewContext.projection, Projection);
                    auto ViewProjection = View * Projection;
                    DirectX::XMStoreFloat4x4(&previewContext.viewProjection, ViewProjection);
                    DirectX::XMStoreFloat4x4(&previewContext.inverseView, XMMatrixInverse(nullptr, View));
                    DirectX::XMStoreFloat4x4(&previewContext.inverseProjection, XMMatrixInverse(nullptr, Projection));
                    DirectX::XMStoreFloat4x4(&previewContext.inverseViewProjection, XMMatrixInverse(nullptr, ViewProjection));
                }

                previewRenderPipeline->Execute(&previewContext, scene);
            }

            // エフェクトプレビュー用の描画処理
            {
                EditorCamera* editorCamera = scene->GetEditorCamera(EDITOR_CAMERA_EFFECT_PREVIEW);
                cameraPos = editorCamera->GetPosition();
                View = editorCamera->GetViewMatrix();
                Projection = editorCamera->GetProjectionMatrix();

                // RenderContextの設定
                {
                    effectPreviewContext.renderState = Graphics::GetRenderState();
                    effectPreviewContext.deltaTime = Time::DeltaTime();
                    effectPreviewContext.unscaledDeltaTime = Time::UnscaledDeltaTime();
                    effectPreviewContext.totalTime = time->TimeStamp();

                    effectPreviewContext.cameraPosition = cameraPos;
                    DirectX::XMStoreFloat4x4(&effectPreviewContext.view, View);
                    DirectX::XMStoreFloat4x4(&effectPreviewContext.projection, Projection);
                    auto ViewProjection = View * Projection;
                    DirectX::XMStoreFloat4x4(&effectPreviewContext.viewProjection, ViewProjection);
                    DirectX::XMStoreFloat4x4(&effectPreviewContext.inverseView, XMMatrixInverse(nullptr, View));
                    DirectX::XMStoreFloat4x4(&effectPreviewContext.inverseProjection, XMMatrixInverse(nullptr, Projection));
                    DirectX::XMStoreFloat4x4(&effectPreviewContext.inverseViewProjection, XMMatrixInverse(nullptr, ViewProjection));
                }
                effectPreviewRenderPipeline->Execute(&effectPreviewContext, scene);
			}

#endif // DEBUG
            {
                // ゲームビュー用の描画処理
#ifdef _DEBUG
                ProfileScopedSection_2(0, "GameView::Rendering", ImGuiControl::Profiler::Purple);
#endif // _DEBUG
                auto* cam = scene->cameraSystem.GetMainCamera();
                if (cam)
                {
                    auto* transform = cam->GetTransform();
                    cameraPos = transform->GetWorldPosition();
                    View = cam->GetViewMatrix();
                    Projection = cam->GetProjectionMatrix();

                    // RenderContextの設定
                    {
                        gameContext.renderState = Graphics::GetRenderState();
                        gameContext.deltaTime = Time::DeltaTime();
                        gameContext.unscaledDeltaTime = Time::UnscaledDeltaTime();
                        gameContext.totalTime = time->TimeStamp();

                        gameContext.cameraPosition = cameraPos;
                        DirectX::XMStoreFloat4x4(&gameContext.view, View);
                        DirectX::XMStoreFloat4x4(&gameContext.projection, Projection);
                        auto ViewProjection = View * Projection;
                        DirectX::XMStoreFloat4x4(&gameContext.viewProjection, ViewProjection);
                        DirectX::XMStoreFloat4x4(&gameContext.inverseView, XMMatrixInverse(nullptr, View));
                        DirectX::XMStoreFloat4x4(&gameContext.inverseProjection, XMMatrixInverse(nullptr, Projection));
                        DirectX::XMStoreFloat4x4(&gameContext.inverseViewProjection, XMMatrixInverse(nullptr, ViewProjection));
                    }

                    // ゲームビュー用の描画処理
                    gameRenderPipeline->Execute(&gameContext, scene);
                }
                else
                {
                    gameContext.acceptRendering = false;
                }
            }
        }
        else
        {
            sceneContext.acceptRendering = false;
            gameContext.acceptRendering = false;
        }
    }

	// デバッグ描画 (Renderに入れてる理由は、GUI描画でRenderContextの情報を使いたいから)
#ifdef USE_IMGUI
    // ImGui描画
    SceneManager::DrawGUI(&sceneContext, &gameContext);

    // RenderSystemのGUI描画
    DrawEditorGUI();

	// インポート設定ウィンドウのGUI描画
	CurryEngine::Resources::ImportSettingsWindow::DrawGUI(&previewContext);

	// ImGuiデバッグログウィンドウの表示
    ImGui::ShowDebugLogWindow();

    // ImGuiTheme描画
    ImGuiTheme::DrawGUI();

    //エフェクトエディタGUI描画
    EffectEditor::DrawGUI(&effectPreviewContext);

    //物理エンジンデバッグ描画
    Physics::DrawGUI();

    //レイヤー設定GUI描画
    LayerManager::Get().DrawLayerSettingsGUI();

    // Undo/Redoテスト用 (確認が終わったら別場所に移す)
    {
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z))
        {
            CurryEngine::History::Undo();
        }
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y))
        {
            CurryEngine::History::Redo();
        }

        // デバッグ用: Undo/Redoの内容表示
		static bool showHistoryWindow = true;
		if (showHistoryWindow)
        {
			ImGui::Begin("History", &showHistoryWindow);
            {
                std::vector<std::string> undoDescriptions, redoDescriptions;
                CurryEngine::History::GetUndoRedoDescriptions(undoDescriptions, redoDescriptions);
                ImGui::Text("Undo Stack:");
                for (const auto& action : undoDescriptions)
                {
                    ImGui::BulletText("%s", action.c_str());
                }
                ImGui::Separator();
                ImGui::Text("Redo Stack:");
                for (const auto& action : redoDescriptions)
                {
                    ImGui::BulletText("%s", action.c_str());
                }
            }
            ImGui::End();
        }
    }
    //ImGui::Text("Video memory usage %d MB", VideoMemoryUsage());

    ProfileDrawUI();
    // ImGui描画
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif // USE_IMGUI
}

void RenderSystem::DrawEditorGUI()
{
#ifdef USE_IMGUI
	static bool showRenderSystemWindow = true;
	if (showRenderSystemWindow)
    {
        ImGui::Begin("Render System", &showRenderSystemWindow);
        {
            if (ImGui::CollapsingHeader("Scene View Pipeline"))
            {
                sceneRenderPipeline->DrawProperty();
            }
            if (ImGui::CollapsingHeader("Game View Pipeline"))
            {
                gameRenderPipeline->DrawProperty();
            }
        }
        ImGui::End();
    }

#endif // USE_IMGUI
}

void RenderSystem::OnSizeChanged(ID3D11Device* device, uint32_t width, uint32_t height)
{
    // 最小化時（0x0）は無視
    if (width == 0 || height == 0) return;

    sceneRenderPipeline->OnSizeChanged(device, width, height);
    gameRenderPipeline->OnSizeChanged(device, width, height);
}

void RenderSystem::Finalize()
{
	gameRenderPipeline->Finalize();
	sceneRenderPipeline->Finalize();
}