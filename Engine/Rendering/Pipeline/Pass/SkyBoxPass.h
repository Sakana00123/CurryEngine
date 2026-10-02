#pragma once
#include "RenderPass.h"
#include "Engine/Rendering/Renderers/Skymap.h"

/** @brief SkyBoxPass を表すクラスです。 */
class SkyBoxPass : public RenderPass
{
public:
	// SkyBoxPassの初期化処理
	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;
	// SkyBoxPassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;

	// SkyBoxPassのプロパティ描画処理
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty() override;

private:
	// スカイボックス用のリソース（例: シェーダー、テクスチャなど）をここに追加
	std::unique_ptr<Skymap> skymap;

	std::shared_ptr<AssetTexture> gameBackgroundTexture; // ゲーム背景テクスチャ
	std::unique_ptr<Material> backgroundMaterial; // 背景描画用のマテリアル
};