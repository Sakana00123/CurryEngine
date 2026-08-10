#include "pch.h"
#include "RenderPipeline.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

// NOTE: 各描画パスのヘッダーファイルをインクルードする必要があります。
#include "Engine/Rendering/Pipeline/Pass/OpaquePass.h"
#include "Engine/Rendering/Pipeline/Pass/ConstantBufferPass.h"
#include "Engine/Rendering/Pipeline/Pass/ParticlePass.h"
#include "Engine/Rendering/Pipeline/Pass/FinalPass.h"
#include "Engine/Rendering/Pipeline/Pass/PreRenderPass.h"
#include "Engine/Rendering/Pipeline/Pass/SkyBoxPass.h"
#include "Engine/Rendering/Pipeline/Pass/ShadowMapPass.h"
#include "Engine/Rendering/Pipeline/Pass/ShadowApplyPass.h"
#include "Engine/Rendering/Pipeline/Pass/UIPass.h"
#include "Engine/Rendering/Pipeline/Pass/DebugRenderPass.h"
#include "Engine/Rendering/Pipeline/Pass/PostProcessPass.h"
#include "Engine/Rendering/Pipeline/Pass/TransitionPass.h"

/// ---------------------------------- PFXインクルード ----------------------------------
#include "Engine/Rendering/Pipeline/Pass/PFX_CrtPass.h"
#include "Engine/Rendering/Pipeline/Pass/PFX_OutLinePass.h"


// -------------------------------- 描画パイプラインの共通処理 ----------------------------------

void RenderPipeline::Initialize()
{
	ZoneScopedN("RenderPipeline::Initialize");

	// 描画パイプラインの初期化処理
	for (const auto& pass : m_renderPasses)
	{
		pass->Initialize();
	}
}

void RenderPipeline::Finalize()
{
	ZoneScopedN("RenderPipeline::Finalize");

	// 描画パイプラインの終了処理
	for (const auto& pass : m_renderPasses)
	{
		pass->Finalize();
	}
	// 描画パスのリストをクリア
	m_renderPasses.clear();
}

void RenderPipeline::Execute(RenderContext* rtx, Scene* scene)
{
	ZoneScopedN("RenderPipeline::Execute");

	// 描画パイプラインの実行処理
	for (const auto& pass : m_renderPasses)
	{
		pass->Execute(rtx, scene);
	}
}

void RenderPipeline::DrawProperty()
{
	ZoneScopedN("RenderPipeline::DrawProperty");

	// 描画パイプラインのプロパティ描画処理
	for (const auto& pass : m_renderPasses)
	{
		pass->DrawProperty();
	}
}

void RenderPipeline::OnSizeChanged(ID3D11Device* device, uint32_t width, uint32_t height)
{
	ZoneScopedN("RenderPipeline::OnSizeChanged");
	// 最小化時（0x0）は無視
	if (width == 0 || height == 0) return;


	// 描画パイプラインのサイズ変更イベント処理
	for (const auto& pass : m_renderPasses)
	{
		pass->OnSizeChanged(device, width, height);
	}
}

void RenderPipeline::AddRenderPass(std::unique_ptr<RenderPass> pass)
{
	pass->SetRenderPassName(typeid(*pass).name()); // デバッグ用にパス名を設定
	m_renderPasses.push_back(std::move(pass));
}

// ---------------------------------- 各描画パイプラインのパス登録処理 ----------------------------------

// シーンビューの描画パスの登録処理
void SceneRenderPipeline::SetupRenderPasses()
{
	AddRenderPass(std::make_unique<ConstantBufferPass>());
	AddRenderPass(std::make_unique<PreRenderPass>());
	AddRenderPass(std::make_unique<SkyBoxPass>());
	AddRenderPass(std::make_unique<OpaquePass>());
	AddRenderPass(std::make_unique<DebugRenderPass>());
	AddRenderPass(std::make_unique<ParticlePass>());
	AddRenderPass(std::make_unique<ShadowMapPass>());
	AddRenderPass(std::make_unique<ShadowApplyPass>());
	AddRenderPass(std::make_unique<UIPass>());

	AddRenderPass(std::make_unique<PostProcessPass>());
	AddRenderPass(std::make_unique<TransitionPass>());
	AddRenderPass(std::make_unique<FinalPass>());
}

// ゲームビューの描画パスの登録処理
void GameRenderPipeline::SetupRenderPasses()
{
	AddRenderPass(std::make_unique<ConstantBufferPass>());
	AddRenderPass(std::make_unique<PreRenderPass>());
	AddRenderPass(std::make_unique<SkyBoxPass>());
	AddRenderPass(std::make_unique<OpaquePass>());
	AddRenderPass(std::make_unique<ParticlePass>());
	AddRenderPass(std::make_unique<ShadowMapPass>());
	AddRenderPass(std::make_unique<ShadowApplyPass>());
	AddRenderPass(std::make_unique<PFX_OutLinePass>());
	AddRenderPass(std::make_unique<UIPass>());
	AddRenderPass(std::make_unique<PostProcessPass>());
	//PFX
	AddRenderPass(std::make_unique<PfxCrtPass>());
	AddRenderPass(std::make_unique<TransitionPass>());

	AddRenderPass(std::make_unique<FinalPass>());
}

void PreviewRenderPipeline::SetupRenderPasses()
{
	AddRenderPass(std::make_unique<ConstantBufferPass>());
	AddRenderPass(std::make_unique<PreRenderPass>());
	AddRenderPass(std::make_unique<SkyBoxPass>());
	AddRenderPass(std::make_unique<PreviewPass>());
}

void EffectPreviewRenderPipeline::SetupRenderPasses()
{
	AddRenderPass(std::make_unique<ConstantBufferPass>());
	AddRenderPass(std::make_unique<PreRenderPass>());
	//AddRenderPass(std::make_unique<SkyBoxPass>());
	AddRenderPass(std::make_unique<ParticlePass>());
}
