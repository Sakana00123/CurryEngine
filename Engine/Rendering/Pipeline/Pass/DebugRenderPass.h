#pragma once
#include "RenderPass.h"

/** @brief DebugRenderPass を表すクラスです。 */
class DebugRenderPass : public RenderPass
{
public:
	// DebugRenderPassの初期化処理
	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;

	// DebugRenderPassの終了化処理
	/**
	 * @brief 終了処理を行います。
	 */
	void Finalize() override;

	// DebugRenderPassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;

private:

};