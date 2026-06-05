#pragma once
#include "BaseEventData.h"
#include "RaycastResult.h"

//レイキャストの入力元の情報を格納するためのクラス
class PointerEventData : public BaseEventData
{
public:
#ifdef USE_MULTIPOINTER
	int pointerId = -1;					//マウス：－１
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
	ObjectId pointerEnterId = ObjectId::Invalid(); // 現在ホバーしているオブジェクトのID
	GameObject* pointerPress = nullptr;		// 押しているオブジェクト
	ObjectId pointerPressId = ObjectId::Invalid(); // 押しているオブジェクトのID
	GameObject* lastPress = nullptr;		// 最後に押していたオブジェクト
	ObjectId lastPressId = ObjectId::Invalid(); // 最後に押していたオブジェクトのID
	GameObject* pointerDrag = nullptr;		// ドラッグ対象のオブジェクト
	ObjectId pointerDragId = ObjectId::Invalid(); // ドラッグ対象のオブジェクトのID

	RaycastResult pointerCurrentRaycast;	// 現在のレイキャスト結果
	RaycastResult pointerPressRaycast;		// 押したときのレイキャスト結果

	// ホバー状態のオブジェクトを設定
	void SetPointerEnter(GameObject* obj);
	// 押しているオブジェクトを設定
	void SetPointerPress(GameObject* obj);
	// 最後に押していたオブジェクトを設定
	void SetLastPress(GameObject* obj);
	// ドラッグ対象のオブジェクトを設定
	void SetPointerDrag(GameObject* obj);

	// 現在ホバーしているオブジェクトを取得
	GameObject* GetPointerEnter() const;
	// 押しているオブジェクトを取得
	GameObject* GetPointerPress() const;
	// 最後に押していたオブジェクトを取得
	GameObject* GetLastPress() const;
	// ドラッグ対象のオブジェクトを取得
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