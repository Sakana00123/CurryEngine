#pragma once
#include "RenderPass.h"

/** @brief TransitionPass を表すクラスです。 */
class TransitionPass : public RenderPass
{
public:
	// RenderPassの初期化処理
	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;

	// RenderPassの終了化処理
	/**
	 * @brief 終了処理を行います。
	 */
	void Finalize() override;

	// RenderPassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;

	// RenderPassのプロパティ描画処理
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty() override;

private:
	std::shared_ptr<Material> m_material; // マテリアル
	RenderTexture m_renderTexture; // 描画結果を書き込むテクスチャ

	float transitionProgress = 0.0f; // 0.0 (無変化) 〜 1.0 (完全遷移)
	int isFadeIn = false; // false: 画面->黒 (OUT) / true: 黒->画面 (IN)
};