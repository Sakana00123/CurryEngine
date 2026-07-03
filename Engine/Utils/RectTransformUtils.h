#pragma once
#include "Engine/Core/Math/Vector2.h"
#include "Engine/Core/Math/Vector3.h"
class RectTransform;
class Transform;

class RectTransformUtils
{
public:
	/**
	 * @brief `RectTransform` のアンカーとピボットを位置を変えずに設定します。
	 * @param rect 対象の `RectTransform`。
	 * @param newAnchorMin 新しい最小アンカー（0-1、親左上(0,0)、右下(1,1)）。
	 * @param newAnchorMax 新しい最大アンカー（0-1、親左上(0,0)、右下(1,1)）。
	 * @param newPivot 新しいピボット（0-1、左上(0,0)、右下(1,1)）。
	 * @details この関数は、`rect` の現在の位置を維持しつつ、アンカーとピボットを変更します。
	 *          これにより、UI 要素のレイアウトが崩れることなく、アンカーとピボットの調整が可能です。
	 */
	static void SetAnchorAndPivotWithoutAffectingPosition(
		RectTransform* rect,
		const Vector2& newAnchorMin,
		const Vector2& newAnchorMax,
		const Vector2& newPivot
	);

	/**
	 * @brief 3D `Transform` の位置に基づいて `RectTransform` のアンカーポジションを設定します。
	 * @param rect 対象の `RectTransform`。
	 * @param targetTransform 位置の基準となる `Transform`。
	 * @details この関数は、`targetTransform` のワールド位置をスクリーン座標に変換し、
	 *          それを `rect` のアンカーポジションとして設定します。
	 *          これにより、`rect` は `targetTransform` の位置に追従するようになります。
	 */
	static void SetAnchordPositionBy3DTransform(
		RectTransform* rect,
		Transform* targetTransform
	);

	/**
	 * @brief スクリーン座標をワールド座標に変換します。
	 * @param screenPos スクリーン座標（ピクセル単位）。
	 * @param depth ワールド空間での深度（Z 値）。
	 * @return 変換されたワールド座標。
	 * @details この関数は、現在のカメラ設定に基づいて、指定されたスクリーン座標を
	 *          ワールド座標に変換します。`depth` パラメータは、変換後のワールド
	 *          座標の Z 値を指定します。
	 */
	static Vector3 UIScreenToWorld(
		const Vector2& screenPos,
		float depth
	);

	/**
	 * @brief ワールド座標をスクリーン座標に変換します。
	 * @param worldPos ワールド座標。
	 * @return 変換されたスクリーン座標（ピクセル単位）。
	 * @details この関数は、現在のカメラ設定に基づいて、指定されたワールド座標を
	 *          スクリーン座標に変換します。
	 */
	static XMFLOAT2 WorldToUIScreen(
		const Vector3& worldPos
	);

};
