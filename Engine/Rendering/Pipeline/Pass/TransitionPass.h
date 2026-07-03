#pragma once
#include "RenderPass.h"

class TransitionPass : public RenderPass
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

	float transitionProgress = 0.0f; // 0.0 (無変化) 〜 1.0 (完全遷移)
	int isFadeIn = false; // false: 画面->黒 (OUT) / true: 黒->画面 (IN)
};