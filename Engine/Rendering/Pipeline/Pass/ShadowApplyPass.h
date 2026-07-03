#pragma once
#include "RenderPass.h"

class ShadowApplyPass : public RenderPass
{
public:
	// ShadowMapPassの初期化処理
	void Initialize() override;

	// ShadowMapPassの終了化処理
	void Finalize() override;

	// ShadowMapPassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

	// ShadowMapPassのプロパティ描画処理
	void DrawProperty() override;

private:
	// シャドウマップ適用用のリソース（例: シェーダーなど）をここに追加
	Microsoft::WRL::ComPtr<ID3D11PixelShader> cascadedShadowPs;
	// シャドウマップ用のマテリアル
	std::shared_ptr<Material> m_cascadedShadowMaterial;

	RenderTexture m_shadowRenderTexture; // シャドウマップを合成するためのレンダーテクスチャ

};