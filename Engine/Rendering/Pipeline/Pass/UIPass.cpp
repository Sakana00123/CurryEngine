#include "pch.h"
#include "UIPass.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Rendering/Pipeline/RenderState.h"
#include <Engine\Rendering\Pipeline\Graphics.h>

void UIPass::Initialize()
{

}

void UIPass::Execute(RenderContext* rtx, Scene* scene)
{
	TracyD3D11Zone(Graphics::GetTracyD3D11Ctx(), "UIPass::Execute");
	auto immediateContext = rtx->immediateContext;
	auto renderState = rtx->renderState;
	
	// 深度ステンシルステート設定
	renderState->BindDepthStencilState(immediateContext, DepthStencilState::NoTestNoWrite);
	// ラスタライザ設定
	renderState->BindRasterizerState(immediateContext, RasterizerState::SolidCullNone);
	// ブレンドステート設定
	renderState->BindBlendState(immediateContext, BlendState::Transparency);

	// UIの描画
	scene->objectManager->Draw(rtx);
}
