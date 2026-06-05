#pragma once
#include "UIComponent.h"
#include "RectTransform.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

/**
 * @file
 * @brief UI のマスク処理を行うコンポーネント。
 * @details 指定された矩形（`maskRect`）を基準に、以降の UI 描画をクリッピングします。
 *          `Begin` でマスクを有効化し、`End` でマスクを無効化します。
 */

/**
 * @brief 矩形マスクを提供する UI コンポーネント。
 * @details 子要素の UI 描画を矩形で切り抜く用途で使用します。
 */
class Mask : public UIComponent
{
	C_REFLECT(Mask)
public:
	// Graphic があればそのサイズを、なければRectTransformのサイズを使用してマスク矩形を計算します。
	D3D11_RECT GetScissorRect() const;

	///**
	// * @brief マスクの有効化を開始します。
	// * @param rtx 描画コンテキスト。
	// */
	//void Begin(RenderContext* rtx) override;

	///**
	// * @brief マスクの有効化を終了します。
	// * @param rtx 描画コンテキスト。
	// */
	//void End(RenderContext* rtx) override;

	///**
	// * @brief インスペクタ用のプロパティ描画。
	// */
	//void DrawProperty() override;
};