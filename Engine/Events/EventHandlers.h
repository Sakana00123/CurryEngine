#pragma once

/**
 * @file
 * @brief UI/入力系イベントのハンドラインターフェース群の宣言。
 * @details 各インターフェースは対応するイベント発生時に呼ばれるコールバックを持ち、
 *          さらに安全に呼び出すための `Execute` ヘルパを提供します。
 *          `Execute` は `BaseEventData` を適切な派生型に dynamic_cast して渡します。
 */

#include "BaseEventData.h"
#include "PointerEventData.h"
#include "AxisEventData.h"

/**
 * @brief ポインタが対象へ入ったときのイベントを受け取るインターフェース。
 */
class IPointerEnterHandler
{
	C_REFLECT(IPointerEnterHandler)
	/**
	 * @brief ポインタが対象に入った際に呼ばれます。
	 * @param eventData ポインタイベントデータ（nullptr の可能性あり）
	 */
	virtual void OnPointerEnter(PointerEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IPointerEnterHandler* handler, BaseEventData* eventData) {
		handler->OnPointerEnter(dynamic_cast<PointerEventData*>(eventData));
	}
};

/**
 * @brief ポインタが対象から出たときのイベントを受け取るインターフェース。
 */
class IPointerExitHandler
{
	C_REFLECT(IPointerExitHandler)
	/**
	 * @brief ポインタが対象から離れた際に呼ばれます。
	 * @param eventData ポインタイベントデータ（nullptr の可能性あり）
	 */
	virtual void OnPointerExit(PointerEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IPointerExitHandler* handler, BaseEventData* eventData) {
		handler->OnPointerExit(dynamic_cast<PointerEventData*>(eventData));
	}
};

/**
 * @brief ポインタのボタンが離されたときのイベントを受け取るインターフェース。
 */
class IPointerUpHandler
{
	C_REFLECT(IPointerUpHandler)
	/**
	 * @brief ポインタのボタンが離された際に呼ばれます。
	 * @param eventData ポインタイベントデータ（nullptr の可能性あり）
	 */
	virtual void OnPointerUp(PointerEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IPointerUpHandler* handler, BaseEventData* eventData) {
		handler->OnPointerUp(dynamic_cast<PointerEventData*>(eventData));
	}
};

/**
 * @brief ポインタのボタンが押されたときのイベントを受け取るインターフェース。
 */
class IPointerDownHandler
{
	C_REFLECT(IPointerDownHandler)
	/**
	 * @brief ポインタのボタンが押された際に呼ばれます。
	 * @param eventData ポインタイベントデータ（nullptr の可能性あり）
	 */
	virtual void OnPointerDown(PointerEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IPointerDownHandler* handler, BaseEventData* eventData) {
		handler->OnPointerDown(dynamic_cast<PointerEventData*>(eventData));
	}
};

/**
 * @brief クリックされたときのイベントを受け取るインターフェース。
 */
class IPointerClickHandler
{
	C_REFLECT(IPointerClickHandler)
	/**
	 * @brief クリック時に呼ばれます。
	 * @param eventData ポインタイベントデータ（nullptr の可能性あり）
	 */
	virtual void OnPointerClick(PointerEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IPointerClickHandler* handler, BaseEventData* eventData) {
		handler->OnPointerClick(dynamic_cast<PointerEventData*>(eventData));
	}
};

/**
 * @brief ドラッグ開始時のイベントを受け取るインターフェース。
 */
class IBeginDragHandler
{
	C_REFLECT(IBeginDragHandler)
	/**
	 * @brief ドラッグ開始時に呼ばれます。
	 * @param eventData ポインタイベントデータ（nullptr の可能性あり）
	 */
	virtual void OnBeginDrag(PointerEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IBeginDragHandler* handler, BaseEventData* eventData) {
		handler->OnBeginDrag(dynamic_cast<PointerEventData*>(eventData));
	}
};

/**
 * @brief ドラッグ中のイベントを受け取るインターフェース。
 */
class IDragHandler
{
	C_REFLECT(IDragHandler)
	/**
	 * @brief ドラッグ中に呼ばれます。
	 * @param eventData ポインタイベントデータ（nullptr の可能性あり）
	 */
	virtual void OnDrag(PointerEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IDragHandler* handler, BaseEventData* eventData) {
		handler->OnDrag(dynamic_cast<PointerEventData*>(eventData));
	}
};

/**
 * @brief ドラッグ終了時のイベントを受け取るインターフェース。
 */
class IEndDragHandler
{
	C_REFLECT(IEndDragHandler)
	/**
	 * @brief ドラッグ終了時に呼ばれます。
	 * @param eventData ポインタイベントデータ（nullptr の可能性あり）
	 */
	virtual void OnEndDrag(PointerEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IEndDragHandler* handler, BaseEventData* eventData) {
		handler->OnEndDrag(dynamic_cast<PointerEventData*>(eventData));
	}
};

/**
 * @brief 選択中の更新イベントを受け取るインターフェース。
 */
class IUpdateSelectedHandler
{
	C_REFLECT(IUpdateSelectedHandler)
	/**
	 * @brief 選択中に毎フレーム呼ばれます。
	 * @param eventData イベントデータ
	 */
	virtual void OnUpdateSelected(BaseEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IUpdateSelectedHandler* handler, BaseEventData* eventData) {
		handler->OnUpdateSelected(eventData);
	}
};

/**
 * @brief 軸方向（ナビゲーション）入力のイベントを受け取るインターフェース。
 */
class IMoveHandler
{
	C_REFLECT(IMoveHandler)
	/**
	 * @brief 移動入力があった際に呼ばれます。
	 * @param eventData 軸イベントデータ（nullptr の可能性あり）
	 */
	virtual void OnMove(AxisEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IMoveHandler* handler, BaseEventData* eventData) {
		handler->OnMove(dynamic_cast<AxisEventData*>(eventData));
	}
};

/**
 * @brief 選択時のイベントを受け取るインターフェース。
 */
class ISelectHandler
{
	C_REFLECT(ISelectHandler)
	/**
	 * @brief 選択されたときに呼ばれます。
	 * @param eventData イベントデータ
	 */
	virtual void OnSelect(BaseEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(ISelectHandler* handler, BaseEventData* eventData) {
		handler->OnSelect(eventData);
	}
};

/**
 * @brief 非選択時のイベントを受け取るインターフェース。
 */
class IDeselectHandler
{
	C_REFLECT(IDeselectHandler)
	/**
	 * @brief 非選択になったときに呼ばれます。
	 * @param eventData イベントデータ
	 */
	virtual void OnDeselect(BaseEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IDeselectHandler* handler, BaseEventData* eventData) {
		handler->OnDeselect(eventData);
	}
};

/**
 * @brief 決定/送信（Submit）イベントを受け取るインターフェース。
 */
class ISubmitHandler
{
	C_REFLECT(ISubmitHandler)
	/**
	 * @brief Submit 入力があった際に呼ばれます。
	 * @param eventData イベントデータ
	 */
	virtual void OnSubmit(BaseEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(ISubmitHandler* handler, BaseEventData* eventData) {
		handler->OnSubmit(eventData);
	}
};

/**
 * @brief キャンセル（Cancel）イベントを受け取るインターフェース。
 */
class ICancelHandler
{
	C_REFLECT(ICancelHandler)
	/**
	 * @brief Cancel 入力があった際に呼ばれます。
	 * @param eventData イベントデータ
	 */
	virtual void OnCancel(BaseEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(ICancelHandler* handler, BaseEventData* eventData) {
		handler->OnCancel(eventData);
	}
};

/**
 * @brief スクロールイベントを受け取るインターフェース。
 */
class IScrollHandler
{
	C_REFLECT(IScrollHandler)
	/**
	 * @brief スクロール入力があった際に呼ばれます。
	 * @param eventData ポインタイベントデータ（nullptr の可能性あり）
	 */
	virtual void OnScroll(PointerEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IScrollHandler* handler, BaseEventData* eventData) {
		handler->OnScroll(dynamic_cast<PointerEventData*>(eventData));
	}
};

/**
 * @brief ドロップ（Drop）イベントを受け取るインターフェース。
 */
class IDropHandler
{
	C_REFLECT(IDropHandler)
	/**
	 * @brief ドロップされた際に呼ばれます。
	 * @param eventData ポインタイベントデータ（nullptr の可能性あり）
	 */
	virtual void OnDrop(PointerEventData* eventData) {}
public:
	/**
	 * @brief ハンドラにイベントを配送します。
	 * @param handler 受信先ハンドラ
	 * @param eventData 基底イベントデータ
	 */
	static void Execute(IDropHandler* handler, BaseEventData* eventData) {
		handler->OnDrop(dynamic_cast<PointerEventData*>(eventData));
	}
};