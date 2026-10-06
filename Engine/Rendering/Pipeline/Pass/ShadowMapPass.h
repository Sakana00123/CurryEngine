#pragma once
#include "RenderPass.h"
#include "Engine/Rendering/Buffers/CascadedShadowMaps.h"

/** @brief ShadowMapPass を表すクラスです。 */
class ShadowMapPass : public RenderPass
{
public:
	// ShadowMapPassの初期化処理
	void Initialize() override;

	// ShadowMapPassの実
	void Execute(RenderContext* rtx, Scene* scene) override;

	// ShadowMapPassのプロパティ描画処理
	void DrawProperty() override;

private:
	// シャドウマップ用のリソース（例: 深度ステンシルビュー、シェーダーなど）をここに追加
	std::unique_ptr<CascadedShadowMaps> cascadedShadowMaps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> cascadedShadowPs;
	float criticalDepthValue = 990.0f;
};
