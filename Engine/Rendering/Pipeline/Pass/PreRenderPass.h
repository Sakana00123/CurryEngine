#pragma once
#include "RenderPass.h"

class PreRenderPass : public RenderPass
{
public:
	// PreRenderPassの初期化処理
	void Initialize() override;

	// PreRenderPassの終了化処理
	void Finalize() override;

	// PreRenderPassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

private:
	RenderTexture m_preRenderTexture; // レンダーターゲットをメンバ変数として保持

};
