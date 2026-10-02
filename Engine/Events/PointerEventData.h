#pragma once
#include "BaseEventData.h"
#include "RaycastResult.h"

//レイキャストの入力元の情報を格納するためのクラス
/** @brief PointerEventData を表すクラスです。 */
class PointerEventData : public BaseEventData
{
public:
#ifdef USE_MULTIPOINTER
	int pointerId = -1;					//マウス：−１
#endif // USE_MULTIPOINTER

	Vector2 position{};			// 現在のスクリーン座標
	Vector2 lastPosition{};		// 前フレームのスクリーン座標
	Vector2 delta{};				// 前フレームからの移動量
	Vector2 pressPosition{};		// 押した位置
	float scrollDelta = 0.f;				// スクロール量
	float clickTime = 0.f;					// 最後のクリック時間
	int clickCount = 0;						// クリック回数
	bool eligibleForClick = false;			// クリック候補状態
	bool dragging = false;					// ドラッグ中かどうか

	GameObject* pointerEnter = nullptr;		// 現在ホバーしているオブジェクト
	/**
	 * @brief Invalid の処理を行います。
	 */
	ObjectId pointerEnterId = ObjectId::Invalid(); // 現在ホバーしているオブジェクトのID
	GameObject* pointerPress = nullptr;		// 押しているオブジェクト
	/**
	 * @brief Invalid の処理を行います。
	 */
	ObjectId pointerPressId = ObjectId::Invalid(); // 押しているオブジェクトのID
	GameObject* lastPress = nullptr;		// 最後に押していたオブジェクト
	/**
	 * @brief Invalid の処理を行います。
	 */
	ObjectId lastPressId = ObjectId::Invalid(); // 最後に押していたオブジェクトのID
	GameObject* pointerDrag = nullptr;		// ドラッグ対象のオブジェクト
	/**
	 * @brief Invalid の処理を行います。
	 */
	ObjectId pointerDragId = ObjectId::Invalid(); // ドラッグ対象のオブジェクトのID

	RaycastResult pointerCurrentRaycast;	// 現在のレイキャスト結果
	RaycastResult pointerPressRaycast;		// 押したときのレイキャスト結果

	// ホバー状態のオブジェクトを設定
	/**
	 * @brief PointerEnter を設定します。
	 */
	void SetPointerEnter(GameObject* obj);
	// 押しているオブジェクトを設定
	/**
	 * @brief PointerPress を設定します。
	 */
	void SetPointerPress(GameObject* obj);
	// 最後に押していたオブジェクトを設定
	/**
	 * @brief LastPress を設定します。
	 */
	void SetLastPress(GameObject* obj);
	// ドラッグ対象のオブジェクトを設定
	/**
	 * @brief PointerDrag を設定します。
	 */
	void SetPointerDrag(GameObject* obj);

	// 現在ホバーしているオブジェクトを取得
	/**
	 * @brief GetPointerEnter に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	GameObject* GetPointerEnter() const;
	// 押しているオブジェクトを取得
	/**
	 * @brief GetPointerPress に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	GameObject* GetPointerPress() const;
	// 最後に押していたオブジェクトを取得
	/**
	 * @brief GetLastPress に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	GameObject* GetLastPress() const;
	// ドラッグ対象のオブジェクトを取得
	/**
	 * @brief GetPointerDrag に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	GameObject* GetPointerDrag() const;

public:
	PointerEventData(EventSystem* eventSystem) : BaseEventData(eventSystem) {}

	void Reset() override {
		position = {};
		lastPosition = {};
		delta = {};
		pressPosition = {};
		scrollDelta = {};
		clickTime = 0.0f;
		clickCount = 0;
		eligibleForClick = false;
		dragging = false;
		pointerEnter = nullptr;
		pointerPress = nullptr;
		lastPress = nullptr;
		pointerDrag = nullptr;
		pointerEnterId = ObjectId::Invalid();
		pointerPressId = ObjectId::Invalid();
		lastPressId = ObjectId::Invalid();
		pointerDragId = ObjectId::Invalid();
		pointerCurrentRaycast = {};
		pointerPressRaycast = {};
	}
};