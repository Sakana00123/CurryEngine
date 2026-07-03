#include "pch.h"
#include "DebugRenderPass.h"
#include "Engine/Rendering/Renderers/DebugRenderer.h"
#include "Engine/Rendering/Pipeline/RenderState.h"
#include "Engine/Physics/Physics.h"

void DebugRenderPass::Initialize()
{
	// DebugRenderPassの初期化処理を実装
	DebugRenderer::Initialize();
}

void DebugRenderPass::Finalize()
{
	// DebugRenderPassの終了処理を実装
	DebugRenderer::Finalize();
}

void DebugRenderPass::Execute(RenderContext* rtx, Scene* scene)
{
	// DebugRenderPassの実行処理を実装
#ifdef _DEBUG
// グリッドの描画

	auto immediateContext = rtx->immediateContext;
	auto renderState = rtx->renderState;

	// 深度ステンシルステート設定
	renderState->BindDepthStencilState(immediateContext, DepthStencilState::TestOnly);
	// ラスタライザ設定
	renderState->BindRasterizerState(immediateContext, RasterizerState::SolidCullNone);
	// ブレンドステート設定
	renderState->BindBlendState(immediateContext, BlendState::Transparency);

	// Physicsのデバッグ描画
	Physics::RenderDebug(rtx);

	static constexpr float GridSize = 10.0f; // グリッドの全体サイズ
	static constexpr int GridDivisions = static_cast<int>(GridSize); // グリッドの分割数(1mごとに線を引く)
	static const Color gridColor = Color::White; // グリッドの色
	DebugRenderer::DrawGrid(Vector3::Zero, GridSize, GridDivisions, gridColor);

	// デバッグ描画
	DebugRenderer::DrawAll(rtx);
#endif // _DEBUG
}