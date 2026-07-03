#pragma once
#include "RenderPass.h"
#include "Engine/Rendering/Buffers/CascadedShadowMaps.h"

class ShadowMapPass : public RenderPass
{
public:
	// ShadowMapPassの初期化処理
	void Initialize() override;

	// ShadowMapPassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

private:
	// シャドウマップ用のリソース（例: 深度ステンシルビュー、シェーダーなど）をここに追加
	std::unique_ptr<CascadedShadowMaps> cascadedShadowMaps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> cascadedShadowPs;
	float criticalDepthValue = 990.0f;
};