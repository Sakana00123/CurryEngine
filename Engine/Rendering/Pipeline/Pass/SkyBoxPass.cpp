#include "pch.h"
#include "SkyBoxPass.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Resources/ResourceManager.h"

void SkyBoxPass::Initialize()
{
	skymap = std::make_unique<Skymap>(Graphics::GetDevice());

	// ゲーム背景テクスチャのロード
	/*gameBackgroundTexture = ResourceManager::GetOrLoad<AssetTexture>("./Assets/Texture/image2.png");
	backgroundMaterial = std::make_unique<Material>();
	backgroundMaterial->SetShader(Graphics::GetDevice(), ResourceManager::GetOrLoadShader<PixelShader>("BackgroundShaderPS"));
	backgroundMaterial->SetTexture("texture_map", gameBackgroundTexture);*/
}

void SkyBoxPass::Execute(RenderContext* rtx, Scene* scene)
{
	auto immediateContext = rtx->immediateContext;
	auto renderState = rtx->renderState;

	// スカイマップの描画
	if (skymap)
	{
		renderState->BindDepthStencilState(immediateContext, DepthStencilState::NoTestNoWrite);
		renderState->BindRasterizerState(immediateContext, RasterizerState::SolidCullNone);
		skymap->Draw(immediateContext);
	}

	//if (gameBackgroundTexture)
	//{
	//	// ゲーム背景テクスチャの描画
	//	renderState->BindDepthStencilState(immediateContext, DepthStencilState::NoTestNoWrite);
	//	renderState->BindRasterizerState(immediateContext, RasterizerState::SolidCullNone);

	//	//Vector2 offset = { 0.0f, 0.0f }; // オフセットを必要に応じて変更
	//	//offset.x -= rtx->totalTime * 0.01f; // 時間経過に応じてオフセットを変更（例: ゆっくりと右にスクロール）
	//	//offset.y += rtx->totalTime * 0.01f; 
	//	//backgroundMaterial->SetValue("uvOffset", offset);

	//	// フルスクリーンクアッドを描画
	//	rtx->DrawFullScreenQuad(backgroundMaterial.get());
	//}
}

void SkyBoxPass::DrawProperty()
{
	// ImGui を使用してプロパティを描画
#ifdef USE_IMGUI
	if (ImGui::CollapsingHeader("SkyBoxPass"), ImGuiTreeNodeFlags_DefaultOpen)
	{
		ImGui::Text("Skymap:");
		if (skymap)
		{
			ImGui::Text("Loaded");
		}
		else
		{
			ImGui::Text("Not Loaded");
		}
		ImGui::Separator();
		// ゲーム背景テクスチャのプロパティ
		ImGui::Text("Game Background Material:");
		if (backgroundMaterial)
		{
			backgroundMaterial->DrawProperty();
		}
	}
#endif // USE_IMGUI
}