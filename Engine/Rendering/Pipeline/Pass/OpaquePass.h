#pragma once

#include "RenderPass.h"

class OpaquePass : public RenderPass
{
public:

	// OpaquePass‚ÌŽÀ‘•
	void Execute(RenderContext* rtx, Scene* scene) override;

};

class PreviewPass : public RenderPass
{
public:
	// PreviewPass‚ÌŽÀ‘•
	void Execute(RenderContext* rtx, Scene* scene) override;

};