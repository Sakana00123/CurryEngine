#include "pch.h"
#include "SceneViewWindow.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Core/ObjectManager.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"
#include "EditorGUI.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include <ImGuizmo.h>
#include <imgui_internal.h>
#include <Engine\Input\InputSystem.h>
#include <Engine\Rendering\Camera\EditorCamera.h>
#include <Engine\Physics\Physics.h>
#include <Engine\EditorSupport\EditorRaycast.h>
#include "Engine/EditorSupport/EditorSelection.h"
#include "Engine/Physics/Collider.h"

namespace CurryEngine
{

	void SceneViewWindow::Draw(RenderContext* rtx, Scene* scene)
	{
		// シーンのレンダリングとオブジェクト選択の管理をここに実装します。
		ImGui::Begin("Scene");

		// シーンビューを表示する前に、上部にツールバーを配置
		float sceneViewToolbarHeight = EditorGUI::DrawSceneViewToolbar();

		// 16:9のアスペクト比を維持しつつ、利用可能なスペースに最大限表示するための計算
		const float targetAspect = 16.0f / 9.0f;
		ImVec2 avail = ImGui::GetContentRegionAvail();
		ImVec2 displaySize;
		ImVec2 offset(0, 0);

		// アスペクト比に応じてサイズを調整
		float availAspect = avail.x / avail.y;
		if (availAspect > targetAspect) {
			displaySize.y = avail.y;
			displaySize.x = avail.y * targetAspect;
			offset.x = (avail.x - displaySize.x) * 0.5f;
		}
		else {
			displaySize.x = avail.x;
			displaySize.y = avail.x / targetAspect;
			offset.y = (avail.y - displaySize.y) * 0.5f;
		}

		// センタリング
		ImGui::SetCursorPos(ImVec2(
			ImGui::GetCursorPosX() + offset.x,
			ImGui::GetCursorPosY() + offset.y
		));

		// フレームバッファのSRVをImGui::Imageで表示
		ID3D11ShaderResourceView* srv = nullptr;
		if (rtx->acceptRendering) {
			if (RenderTexture* renderTarget = static_cast<RenderTexture*>(rtx->GetSharedResource("PostProcessPass_RenderTexture")))
			{
				srv = renderTarget->GetColorBuffer();
			}
		}
		if (srv == nullptr) {
			// ダミーテクスチャを表示
			static std::shared_ptr<AssetTexture> whiteTexture;
			if (!whiteTexture) {
				whiteTexture = std::make_shared<AssetTexture>();
				whiteTexture->MakeDummy(Graphics::GetDevice(), 0xFFFFFFFF, 16);
			}
			srv = whiteTexture->GetSRV(); // デフォルト白テクスチャ
		}

		ImGui::Image(srv, displaySize);

		// ImGui::Imageの表示矩形を取得し、設定
		if (ImGui::IsWindowHovered())
		{
			ImVec2 imageMin = ImGui::GetItemRectMin(); // 左上スクリーン座標
			ImVec2 imageMax = ImGui::GetItemRectMax(); // 右下スクリーン座標
			//範囲設定
			Graphics::SetScreenRect(imageMin.x, imageMin.y, imageMax.x, imageMax.y);

			// Imageの範囲内にカーソルがあるかでフォーカス判定
			if (ImGui::IsMouseHoveringRect(imageMin, imageMax)) {
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
					ImGui::SetWindowFocus(); // シーンビューウィンドウにフォーカスを設定
				}
				auto navWindow = ImGui::GetCurrentContext()->NavWindow;
				std::string windowName = navWindow ? navWindow->Name : "None";
				isSceneWindowFocused = (ImGui::IsItemActivated() || ImGui::IsItemHovered()) && windowName == "Scene";
			}
			else {
				isSceneWindowFocused = false;
			}
		}
		else {
			isSceneWindowFocused = false;
		}

		if (scene != nullptr)
		{
			//ギズモ
			scene->GetObjectManager()->DrawGuizmo(rtx);


			// シーンビューのレイキャストによるオブジェクト選択
			if (isSceneWindowFocused && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				// ギズモに触れている場合は完全スキップ
				bool isOverGuizmo = ImGuizmo::IsOver() || ImGuizmo::IsUsing();

				// ImGui のUI要素（ボタン、スライダー等）に触れていない
				bool isOverImGuiItem = ImGui::IsAnyItemActive();

				if (!isOverGuizmo && !isOverImGuiItem)
				{
					Vector2 rayStartScreen = InputSystem::GetMousePosition();
					Vector3 rayStart, rayDir;
					scene->GetEditorCamera(EDITOR_CAMERA_SCENE_VIEW)->ScreenPointToRay(rayStartScreen, rayStart, rayDir);

					float rayLength = 1000.0f;
					RaycastHit hitInfo;

					bool physxHit = Physics::Raycast(rayStart, rayDir, rayLength, hitInfo, LayerMasks::Everything);
					CurryEngine::EditorSupport::EditorRaycastResult aabbHitInfo;
					bool aabbHit = (RaycastAABBFallback(rayStart, rayDir, rayLength, scene, aabbHitInfo));
					std::shared_ptr<GameObject> selectedObj = nullptr;

					float physxHitDistance = physxHit ? hitInfo.distance : std::numeric_limits<float>::max();
					float aabbHitDistance = aabbHit ? aabbHitInfo.distance : std::numeric_limits<float>::max();

					if (physxHit && physxHitDistance <= aabbHitDistance)
					{
						// PhysXヒット
						selectedObj = scene->FindGameObjectPtrById(
							hitInfo.collider->GetOwner()->GetId());
					}
					else if (aabbHit)
					{
						// AABBヒット
						selectedObj = aabbHitInfo.hitObject.lock();
					}

					// ヒットしたオブジェクトを選択。Ctrlキーが押されている場合は選択に追加、そうでない場合は単独選択。何もヒットしなかった場合は、Ctrlキーが押されていなければ選択解除。
					if (selectedObj)
					{
						if (EditorSelection* sel = scene->GetObjectManager()->GetEditorSelection())
						{
							sel->Select(selectedObj, ImGui::GetIO().KeyCtrl);
							scene->GetObjectManager()->SelectInspectorNode(selectedObj.get());
						}
					}
					else if (!ImGui::GetIO().KeyCtrl)
					{
						// 何もヒットしなかった → 選択解除
						if (EditorSelection* sel = scene->GetObjectManager()->GetEditorSelection())
						{
							sel->Clear();
							scene->GetObjectManager()->SelectInspectorNode(nullptr);
						}
					}
				}
			}
		}
		ImGui::End();
	}
}