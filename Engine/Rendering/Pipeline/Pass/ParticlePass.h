#pragma once
#include "RenderPass.h"

/** @brief ParticlePass を表すクラスです。 */
class ParticlePass : public RenderPass
{
public:
	// ParticlePassの初期化処理
	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;

	// ParticlePassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;

};