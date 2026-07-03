#include "pch.h"
#include "GameViewWindow.h"

#include "Engine/Scenes/Scene.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"
#include <Engine\Rendering\Pipeline\Graphics.h>

namespace CurryEngine
{
	void GameViewWindow::Draw(RenderContext* rtx, Scene* scene)
	{
		ImGui::Begin("Game");
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
				isGameWindowFocused = ImGui::IsItemActivated() || ImGui::IsItemHovered(); // ウィンドウがアクティブか、もしくはホバーされている場合にフォーカスをtrueにする
			}
			else {
				isGameWindowFocused = false;
			}
		}
		else {
			isGameWindowFocused = false;
		}

		ImGui::End();
	}
}