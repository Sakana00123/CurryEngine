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
#include <imgui_internal.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#endif

#include <Engine\Physics\Physics.h>
#include <Engine\Editor\EffectEditor.h>
#include <Engine\Editor\ImportSettings\ImportSettingsWindow.h>
#include <Engine\Editor\AnimationEditor.h>
#include <Engine\Editor\AnimatorControllerEditor.h>
#include <Engine\EditorSupport\CompoundCommand.h>

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

	// アニメーションプレビュー用の描画パイプラインを作成して初期化
	animationPreviewRenderPipeline = std::make_unique<AnimationPreviewRenderPipeline>();
	animationPreviewRenderPipeline->SetupRenderPasses();
	animationPreviewRenderPipeline->Initialize();

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
	RenderContext animationPreviewContext(Graphics::GetDeviceContext(), Graphics::fullScreenQuad.get(), Graphics::GetSharedResources());
    // カメラ情報取得
    {
		auto scene = SceneManager::GetCurrentScene();
		if (scene && scene->IsStarted())
        {
            //新カメラシステム
#if defined(_DEBUG) || defined(CURRY_EDITOR)
            {
				ZoneScopedN("RenderSystem::Render");
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

            // アニメーションプレビュー用の描画処理
            if (0)
            {
                EditorCamera* editorCamera = scene->GetEditorCamera(EDITOR_CAMERA_ANIMATION_PREVIEW);
                cameraPos = editorCamera->GetPosition();
                View = editorCamera->GetViewMatrix();
                Projection = editorCamera->GetProjectionMatrix();
                // RenderContextの設定
                {
                    animationPreviewContext.renderState = Graphics::GetRenderState();
                    animationPreviewContext.deltaTime = Time::DeltaTime();
                    animationPreviewContext.unscaledDeltaTime = Time::UnscaledDeltaTime();
                    animationPreviewContext.totalTime = time->TimeStamp();
                    animationPreviewContext.cameraPosition = cameraPos;
                    DirectX::XMStoreFloat4x4(&animationPreviewContext.view, View);
                    DirectX::XMStoreFloat4x4(&animationPreviewContext.projection, Projection);
                    auto ViewProjection = View * Projection;
                    DirectX::XMStoreFloat4x4(&animationPreviewContext.viewProjection, ViewProjection);
                    DirectX::XMStoreFloat4x4(&animationPreviewContext.inverseView, XMMatrixInverse(nullptr, View));
                    DirectX::XMStoreFloat4x4(&animationPreviewContext.inverseProjection, XMMatrixInverse(nullptr, Projection));
                    DirectX::XMStoreFloat4x4(&animationPreviewContext.inverseViewProjection, XMMatrixInverse(nullptr, ViewProjection));
                }
                animationPreviewRenderPipeline->Execute(&animationPreviewContext, scene);
			}

#endif // DEBUG
            {
                // ゲームビュー用の描画処理
				ZoneScopedN("RenderSystem::Render_GameView");
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
    {
		ZoneScopedN("RenderSystem::DrawGUI");
        SceneManager::DrawGUI(&sceneContext, &gameContext);
    }

    // RenderSystemのGUI描画
    {
		ZoneScopedN("RenderSystem::DrawEditorGUI");
        DrawEditorGUI();
    }

	// インポート設定ウィンドウのGUI描画
    {
		ZoneScopedN("RenderSystem::DrawImportSettingsGUI");
        CurryEngine::Resources::ImportSettingsWindow::DrawGUI(&previewContext);
    }

	// ImGuiデバッグログウィンドウの表示
    {
		ZoneScopedN("RenderSystem::ShowDebugLogWindow");
        ImGui::ShowDebugLogWindow();
    }

    // ImGuiTheme描画
    {
		ZoneScopedN("RenderSystem::DrawImGuiTheme");
        ImGuiTheme::DrawGUI();
    }

    //エフェクトエディタGUI描画
    {
		ZoneScopedN("RenderSystem::DrawEffectEditorGUI");
        EffectEditor::DrawGUI(&effectPreviewContext);
    }

    // アニメーションエディタGUI描画
    {
        ZoneScopedN("RenderSystem::DrawAnimationEditorGUI");
        if (AnimationEditor::IsOpen()) AnimationEditor::DrawGUI(&animationPreviewContext);
        if (AnimatorControllerEditor::IsOpen()) AnimatorControllerEditor::DrawGUI(&animationPreviewContext);
    }

    //物理エンジンデバッグ描画
    {
		ZoneScopedN("RenderSystem::DrawPhysicsDebugGUI");
        Physics::DrawGUI();
    }

    //レイヤー設定GUI描画
    {
		ZoneScopedN("RenderSystem::DrawLayerSettingsGUI");
        LayerManager::Get().DrawLayerSettingsGUI();
    }

    // Undo/Redoテスト用 (確認が終わったら別場所に移す)
    {
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z))
        {
            auto result = CurryEngine::History::Undo();
            if (!result.success)
            {
                LOG_ERROR(std::format("Undo failed: {}", result.message));
			}
        }
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y))
        {
            auto result = CurryEngine::History::Redo();
            if (!result.success)
            {
                LOG_ERROR(std::format("Redo failed: {}", result.message));
            }
        }

        // デバッグ用: Undo/Redoの内容表示
		if (showHistoryWindow)
        {
			ImGui::Begin("History", &showHistoryWindow);
            {
                if (ImGui::Button("Undo"))
                {
                    auto result = CurryEngine::History::Undo();
                    if (!result.success)
                    {
                        LOG_ERROR(std::format("Undo failed: {}", result.message));
                    }
				}
				ImGui::SameLine();
                if (ImGui::Button("Redo"))
                {
                    auto result = CurryEngine::History::Redo();
                    if (!result.success)
                    {
                        LOG_ERROR(std::format("Redo failed: {}", result.message));
					}
				}
				ImGui::SameLine();
                // テスト用
                {
					// 成功するA,B、失敗するCの3つのアクションをまとめてUndo/Redoするテスト
					static int testValue = 0;
                    if (ImGui::Button("Test Undo/Redo"))
                    {
                        auto setter = [&](const int& value) -> CurryEngine::CommandResult
                        {
                            testValue = value;
                            return { true, "" };
						};
                        auto failedSetter = [&](const int& value) -> CurryEngine::CommandResult
                            {
                                return { false, "Failed to set value." };
                            };
                        auto firstOnlySetter = [&](const int& value) -> CurryEngine::CommandResult
                        {
							static bool firstCall = true;
                            if (firstCall)
                            {
								firstCall = false;
                                testValue = value;
                                return { true, "" };
                            }
                            else
                            {
                                return { false, "Failed to set value. This setter only allows the first call." };
                            }
							};

                        auto actionA = std::make_unique<CurryEngine::SetValueCommand<int>>("Action A", firstOnlySetter, testValue, 10);
						auto actionB = std::make_unique<CurryEngine::SetValueCommand<int>>("Action B", setter, testValue, 20);
						auto actionC = std::make_unique<CurryEngine::SetValueCommand<int>>("Action C", setter, testValue, 30);

						auto compoundAction = std::make_shared<CurryEngine::CompoundCommand>("Compound Action");
                        compoundAction->AddCommand(std::move(actionA));
                        compoundAction->AddCommand(std::move(actionB));
                        compoundAction->AddCommand(std::move(actionC));
                        auto result = CurryEngine::History::ExecuteCommand(compoundAction);
                        if (!result.success)
                        {
                            LOG_ERROR(std::format("Compound Action failed: {}", result.message));
						}
					}
					ImGui::SameLine();
					ImGui::Text("Test Value: %d", testValue);
                }


                if (ImGui::BeginChild("HistoryChild", ImVec2(0, 0), true))
                {
					// Undo/Redoの内容表示
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
				ImGui::EndChild();
            }
            ImGui::End();
        }
    }
    //ImGui::Text("Video memory usage %d MB", VideoMemoryUsage());

    // ImGui描画のために、デフォルトのレンダーターゲットに戻す
    sceneContext.SetDefaultRenderTarget();

    // ImGui描画
    {
		ZoneScopedN("RenderSystem::ImGui::Render");
        ImGui::Render();
    }
    {
		ZoneScopedN("RenderSystem::ImGui::RenderDrawData");
		TracyD3D11Zone(Graphics::GetTracyD3D11Ctx(), "ImGui::RenderDrawData");
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
#endif // USE_IMGUI
}

void RenderSystem::DrawEditorGUI()
{
#ifdef USE_IMGUI
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

void RenderSystem::ShowRenderSystemWindow()
{
	showRenderSystemWindow = true;
}

void RenderSystem::ShowHistoryWindow()
{
	showHistoryWindow = true;
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
