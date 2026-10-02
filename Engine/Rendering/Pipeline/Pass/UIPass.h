#pragma once
#include "RenderPass.h"

/** @brief UIPass を表すクラスです。 */
class UIPass : public RenderPass
{
public:
	// UIPassの初期化処理
	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;
	// UIPassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;

};