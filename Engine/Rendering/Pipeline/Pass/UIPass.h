#pragma once
#include "RenderPass.h"

class UIPass : public RenderPass
{
public:
	// UIPass‚Ì‰Šú‰»ˆ—
	void Initialize() override;
	// UIPass‚ÌÀ‘•
	void Execute(RenderContext* rtx, Scene* scene) override;

};