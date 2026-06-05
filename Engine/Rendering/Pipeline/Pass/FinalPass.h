#pragma once
#include "RenderPass.h"


class FinalPass : public RenderPass
{
public:
	// FinalPassの初期化処理
	void Initialize() override;
	// FinalPassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

private:
	// 最終合成用のリソース（例: シェーダーなど）をここに追加
	Microsoft::WRL::ComPtr<ID3D11PixelShader> finalPassPs;

};