#pragma once
#include "RenderPass.h"


/** @brief FinalPass を表すクラスです。 */
class FinalPass : public RenderPass
{
public:
	// FinalPassの初期化処理
	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;
	// FinalPassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;

private:
	// 最終合成用のリソース（例: シェーダーなど）をここに追加
	Microsoft::WRL::ComPtr<ID3D11PixelShader> finalPassPs;

};