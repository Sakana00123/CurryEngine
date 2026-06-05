#pragma once
#include "RenderPass.h"

class ParticlePass : public RenderPass
{
public:
	// ParticlePass‚Ì‰Šú‰»ˆ—
	void Initialize() override;

	// ParticlePass‚ÌÀ‘•
	void Execute(RenderContext* rtx, Scene* scene) override;

};