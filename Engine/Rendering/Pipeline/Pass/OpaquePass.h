#pragma once

#include "RenderPass.h"

class OpaquePass : public RenderPass
{
public:

	// OpaquePassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

};

class PreviewPass : public RenderPass
{
public:
	// PreviewPassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

};