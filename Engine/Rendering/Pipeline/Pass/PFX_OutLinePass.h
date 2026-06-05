#pragma once
#include "RenderPass.h"

class PFX_OutLinePass : public RenderPass
{
public:
	// RenderPassの初期化処理
	void Initialize() override;

	// RenderPassの終了化処理
	void Finalize() override;

	// RenderPassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

	// RenderPassのプロパティ描画処理
	void DrawProperty() override;

private:
	std::shared_ptr<Material> m_material; // マテリアル
	RenderTexture m_renderTexture; // 描画結果を書き込むテクスチャ
};