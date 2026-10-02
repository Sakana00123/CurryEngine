#pragma once
#include "RenderPass.h"

/** @brief PreRenderPass を表すクラスです。 */
class PreRenderPass : public RenderPass
{
public:
	// PreRenderPassの初期化処理
	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;

	// PreRenderPassの終了化処理
	/**
	 * @brief 終了処理を行います。
	 */
	void Finalize() override;

	// PreRenderPassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;

private:
	RenderTexture m_preRenderTexture; // レンダーターゲットをメンバ変数として保持

};
