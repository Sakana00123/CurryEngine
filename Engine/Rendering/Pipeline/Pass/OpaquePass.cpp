#include "pch.h"
#include "OpaquePass.h"
#include "Engine/Rendering/Pipeline/Graphics.h"



void OpaquePass::Execute(RenderContext* rtx, Scene* scene)
{
	auto immediateContext = rtx->immediateContext;
	auto renderState = rtx->renderState;
	
	//深度ステンシルステート設定
    renderState->BindDepthStencilState(immediateContext, DepthStencilState::TestAndWrite, 1);
    //ラスタライザ設定
    renderState->BindRasterizerState(immediateContext, RasterizerState::SolidCullBack);

	// 不透明オブジェクトの描画(一旦はシーン内のすべてのオブジェクトを描画する)
	scene->objectManager->Render(rtx);

	////シーン全体のキャプチャ終了
	//frameBuffer->Deactivate(immediateContext);

	//// フレームバッファのシェーダーリソースビューをRenderContextに共有リソースとして設定
	//rtx->SetSharedResource("OpaquePass_ColorMap", frameBuffer->shader_resource_views[0].Get());
	//rtx->SetSharedResource("OpaquePass_DepthMap", frameBuffer->shader_resource_views[1].Get());
}

#include "Engine/Editor/ImportSettings/ImportSettingsWindow.h"
void PreviewPass::Execute(RenderContext* rtx, Scene* scene)
{
	auto immediateContext = rtx->immediateContext;
	auto renderState = rtx->renderState;
	//深度ステンシルステート設定
	renderState->BindDepthStencilState(immediateContext, DepthStencilState::TestAndWrite, 1);
	//ラスタライザ設定
	renderState->BindRasterizerState(immediateContext, RasterizerState::SolidCullBack);
	// プレビューオブジェクトの描画
	CurryEngine::Resources::ImportSettingsWindow::Render3DPreview(rtx);

	// デフォルトのレンダーターゲットに切り替える
	rtx->SetDefaultRenderTarget();
}