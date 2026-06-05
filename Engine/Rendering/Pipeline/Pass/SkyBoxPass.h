#pragma once
#include "RenderPass.h"
#include "Engine/Rendering/Renderers/Skymap.h"

class SkyBoxPass : public RenderPass
{
public:
	// SkyBoxPassの初期化処理
	void Initialize() override;
	// SkyBoxPassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

	// SkyBoxPassのプロパティ描画処理
	void DrawProperty() override;

private:
	// スカイボックス用のリソース（例: シェーダー、テクスチャなど）をここに追加
	std::unique_ptr<Skymap> skymap;

	std::shared_ptr<AssetTexture> gameBackgroundTexture; // ゲーム背景テクスチャ
	std::unique_ptr<Material> backgroundMaterial; // 背景描画用のマテリアル
};