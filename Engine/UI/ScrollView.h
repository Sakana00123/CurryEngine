#pragma once
#include "UIComponent.h"

class ScrollView : public UIComponent
{
	C_REFLECT(ScrollView)
public:
	
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId contentRef; // スクロールビューの内容を配置する GameObject への参照

	C_PROPERTY()
	float scrollSensitivity = 10.f; // スクロールの感度。マウスホイールの回転量に対するスクロールオフセットの倍率。

private:
	// 現在のスクロール位置を保持する変数。初期値は (0, 0)。
	Vector2 m_scrollPosition{ 0,0 };

public:

	/** @brief 既定コンストラクタ。*/
	ScrollView() = default;

	/**
	 * @brief オブジェクト生成直後に一度だけ呼ばれます。
	 * @details `content` の取得・初期化などを実装側で行います。
	 */
	void Awake() override;

	/**
	 * @brief 更新処理。
	 * @param deltaTime 前フレームからの経過時間（秒）。
	 * @details マウスホイールの入力を検出し、`scrollSensitivity` を考慮して `m_scrollPosition` を更新します。
	 *          更新されたスクロール位置は、`content` の位置に反映されることが想定されます。
	 */
	void Update(float deltaTime) override;

};