#pragma once
#include "RenderPass.h"

class ParticlePass : public RenderPass
{
public:
	// ParticlePassの初期化処理
	void Initialize() override;

	// ParticlePassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

};