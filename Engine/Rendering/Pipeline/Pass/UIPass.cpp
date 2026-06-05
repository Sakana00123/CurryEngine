#include "pch.h"
#include "UIPass.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Rendering/Pipeline/RenderState.h"

void UIPass::Initialize()
{

}

void UIPass::Execute(RenderContext* rtx, Scene* scene)
{
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