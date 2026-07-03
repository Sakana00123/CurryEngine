#pragma once
#include "RenderPass.h"

class DebugRenderPass : public RenderPass
{
public:
	// DebugRenderPassの初期化処理
	void Initialize() override;

	// DebugRenderPassの終了化処理
	void Finalize() override;

	// DebugRenderPassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

private:

};