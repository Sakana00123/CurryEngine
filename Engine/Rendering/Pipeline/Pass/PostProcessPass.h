#pragma once
#include "RenderPass.h"

class PostProcessPass : public RenderPass
{
public:
	// PostProcessPassの初期化処理
	void Initialize() override;

	// PostProcessPassの終了化処理
	void Finalize() override;

	// PostProcessPassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

	// PostProcessPassのプロパティ描画処理
	void DrawProperty() override;

private:
	std::shared_ptr<Material> m_postProcessMaterial; // ポストプロセスマテリアル
	RenderTexture m_postProcessTexture; // レンダーターゲットをメンバ変数として保持
};