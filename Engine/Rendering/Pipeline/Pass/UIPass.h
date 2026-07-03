#pragma once
#include "RenderPass.h"

class UIPass : public RenderPass
{
public:
	// UIPassの初期化処理
	void Initialize() override;
	// UIPassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

};