#include "pch.h"
#include "PreRenderPass.h"

#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"

void PreRenderPass::Initialize()
{
	m_preRenderTexture.Create(Graphics::GetDevice(), 1920, 1080);
	RegisterResizableRenderTexture(&m_preRenderTexture); // リサイズが必要なレンダーターゲットとして登録(※これを呼び出さないと、ウィンドウサイズ変更時にリサイズされないので注意)
}

void PreRenderPass::Finalize()
{
	m_preRenderTexture.Release();
}

void PreRenderPass::Execute(RenderContext* rtx, Scene* scene)
{
	auto immediateContext = rtx->immediateContext;
	auto renderState = rtx->renderState;
	
	// サンプラーステートオブジェクトをバインドする
	renderState->BindSamplerStates(immediateContext);

	// レンダリング前の画面クリア
	Graphics::Clear(0.2f, 0.2f, 0.2f, 1.0f);

	// レンダーターゲットを設定してクリア
	rtx->SetRenderTarget(m_preRenderTexture);
	rtx->ClearCurrentRenderTarget(Color::Black);

	// レンダーテクスチャを共有リソースとしてRenderContextに設定
	rtx->SetSharedResource("PreRenderTexture", &m_preRenderTexture);
}