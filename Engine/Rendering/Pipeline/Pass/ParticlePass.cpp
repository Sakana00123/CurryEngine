#include "pch.h"
#include "ParticlePass.h"
#include "Engine/Effects/EffectManager.h"

void ParticlePass::Initialize()
{
    // パーティクルシステムの初期化
    EffectManager::Initialize();
}

void ParticlePass::Execute(RenderContext* rtx, Scene* scene)
{
	auto immediateContext = rtx->immediateContext;
	auto renderState = rtx->renderState;
	
	ZoneScopedN("ParticlePass::Execute");

    //深度ステンシルステート設定
    renderState->BindDepthStencilState(immediateContext, DepthStencilState::TestOnly, 1);
    //ラスタライザ設定
    renderState->BindRasterizerState(immediateContext, RasterizerState::SolidCullNone);

    // パーティクル描画
    EffectManager::Render(rtx);
}
