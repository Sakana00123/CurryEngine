#pragma once
#include "RenderPass.h"

/** @brief ShadowApplyPass を表すクラスです。 */
class ShadowApplyPass : public RenderPass
{
public:
	// ShadowMapPassの初期化処理
	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;

	// ShadowMapPassの終了化処理
	/**
	 * @brief 終了処理を行います。
	 */
	void Finalize() override;

	// ShadowMapPassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;

	// ShadowMapPassのプロパティ描画処理
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty() override;

private:
	// シャドウマップ適用用のリソース（例: シェーダーなど）をここに追加
	Microsoft::WRL::ComPtr<ID3D11PixelShader> cascadedShadowPs;
	// シャドウマップ用のマテリアル
	std::shared_ptr<Material> m_cascadedShadowMaterial;

	RenderTexture m_shadowRenderTexture; // シャドウマップを合成するためのレンダーテクスチャ

};