#pragma once
#include "RenderPass.h"

/** @brief PostProcessPass を表すクラスです。 */
class PostProcessPass : public RenderPass
{
public:
	// PostProcessPassの初期化処理
	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;

	// PostProcessPassの終了化処理
	/**
	 * @brief 終了処理を行います。
	 */
	void Finalize() override;

	// PostProcessPassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;

	// PostProcessPassのプロパティ描画処理
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty() override;

private:
	std::shared_ptr<Material> m_postProcessMaterial; // ポストプロセスマテリアル
	RenderTexture m_postProcessTexture; // レンダーターゲットをメンバ変数として保持
};