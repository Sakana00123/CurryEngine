#pragma once
#include "UIComponent.h"
class Canvas;

/**
 * @file
 * @brief UI の描画要素の基底コンポーネント。
 * @details 親階層の `Canvas` への登録/解除を自動で行い、ヒットテスト（レイキャスト）可否の
 *          フラグを提供します。`ImGui` が有効な場合はインスペクタで簡易編集が可能です。
 */

/**
 * @brief UI 描画要素の基底クラス。
 * @details `UIComponent` を継承し、`RectTransform` による当たり判定と、
 *          親 `Canvas` への登録管理を担います。
 */
class Graphic : public UIComponent
{
	C_REFLECT(Graphic)
public:
	/**
	 * @brief レイキャスト判定の対象とするか。
	 * @details `true` の場合、`Raycast` で `RectTransform` 内にあるかを判定します。
	 */
	C_PROPERTY()
	bool isRaycastTarget = true;

	/** @brief 既定コンストラクタ。*/
	Graphic() = default;

	/**
	 * @brief デストラクタ。
	 * @details まだ `Canvas` に登録されている場合は登録解除します（遅延削除）。
	 */
	virtual ~Graphic() override = default;

	/**
	 * @brief 有効化処理。
	 * @details `Canvas` に登録されていない場合は登録します。
	 */
	void OnEnable() override;

	/**
	 * @brief 無効化処理。
	 * @details `Canvas` に登録されている場合は登録解除します（遅延削除）。
	 */
	void OnDisable() override;

	/**
	 * @brief 親 `Canvas` を取得します。
	 * @return 親 `Canvas` へのポインタ。存在しない場合は `nullptr` を返します。
	 */
	Canvas* GetCanvas() const;

	/**
	 * @brief レイキャスト（ヒットテスト）を行います。
	 * @param position スクリーン座標系でのテスト位置。
	 * @return 当たり判定が有効かつ、矩形内であれば `true`。それ以外は `false`。
	 * @details `isRaycastTarget` が `true` のときのみ、`RectTransform::Contains` を用いて
	 *          矩形内判定を行います。
	 */
	bool Raycast(const Vector2& position);

#ifdef USE_IMGUI
	/**
	 * @brief インスペクタ用のプロパティ描画。
	 * @details `USE_IMGUI` 定義時のみ、レイキャスト対象フラグをトグル表示します。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI
};