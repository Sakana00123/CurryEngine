#pragma once
#include "RenderPass.h"

class DebugRenderPass : public RenderPass
{
public:
	// DebugRenderPass‚Ì‰Šú‰»ˆ—
	void Initialize() override;

	// DebugRenderPass‚ÌI—¹‰»ˆ—
	void Finalize() override;

	// DebugRenderPass‚ÌÀ‘•
	void Execute(RenderContext* rtx, Scene* scene) override;

private:

};