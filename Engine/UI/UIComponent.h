#pragma once
#include "Engine/Core/Component.h"
#include "RectTransform.h"
#include "Engine/Core/GameObject.h"

/**
 * @file
 * @brief すべての UI 用コンポーネントの基底クラス。
 * @details `RectTransform` を保持し、UI 入力の有効/無効を切り替える基本機能を提供します。
 */
class UIComponent : public Component
{
	C_REFLECT(UIComponent)
	/**
	 * @brief 本 UI の矩形変換。
	 * @details レイアウトやヒットテストに使用します（非所有）。
	 */
	RectTransform* rect = nullptr;
protected:
	/**
	 * @brief 入力受付フラグ。
	 * @details 一部の操作を受け付けないようにするための内部フラグです。
	 */
	C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly)
	bool isInputEnabled = true;
public:

	/**
	 * @brief `RectTransform` を設定します。
	 * @param rt 設定する `RectTransform` へのポインタ。
	 */
	void SetRectTransform(RectTransform* rt) { rect = rt; }

	/**
	 * @brief `RectTransform` を取得します。
	 * @return `RectTransform` へのポインタ。存在しない場合は `nullptr` を返します。
	 */
	RectTransform* GetRectTransform();
	
	/** @brief 既定コンストラクタ。*/
	UIComponent() = default;
	/** @brief デストラクタ。*/
	virtual ~UIComponent() override = default;

	/**
	 * @brief オブジェクト生成直後に一度だけ呼ばれます。
	 * @details `rect` の取得・初期化などを実装側で行います。
	 */
	void Awake() override;

	/**
	 * @brief UI 入力の有効/無効を設定します。
	 * @param enabled `true` で入力有効、`false` で無効。
	 */
	void SetInputEnabled(bool enabled);
};