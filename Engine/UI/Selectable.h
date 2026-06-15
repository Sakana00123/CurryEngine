#pragma once
#include "Engine/Input/InputSystem.h"
#include "UIComponent.h"
#include "Image.h"
#include "Engine/Core/Color.h"
#include "Engine/Events/EventHandlers.h"

/**
 * @file
 * @brief 選択可能な UI 要素の基底コンポーネント。
 * @details ポインタイベント（押下/解放/進入/退出）および選択/非選択、
 *          軸入力による移動を受け取り、ビジュアル状態（色など）を更新します。
 */

/**
 * @brief 選択可能な UI 要素の基底クラス。
 * @details `UIComponent` と各種イベントハンドラを実装します。
 */
class Selectable : public UIComponent, public IPointerDownHandler, public IPointerUpHandler, 
	public IPointerEnterHandler, public IPointerExitHandler,
	public ISelectHandler, public IDeselectHandler, 
	public IMoveHandler
{
	C_REFLECT(Selectable)
public:
	/**
	 * @brief 隣接ナビゲーション先。
	 * @details キー操作による移動先を指定します。
	 */
	struct Navigation {
		Selectable* up = nullptr;   //!< 上方向の遷移先
		Selectable* down = nullptr; //!< 下方向の遷移先
		Selectable* left = nullptr; //!< 左方向の遷移先
		Selectable* right = nullptr;//!< 右方向の遷移先
	};
	/** @brief ナビゲーション設定。*/
	Navigation navigation{};
protected:
	friend class UIAnimationController;
	/** @brief クリック可能か。*/
	C_PROPERTY()
	bool interactable = true;
	/** @brief ホバー中か。*/
	bool isHovered = false;
	/** @brief 押下中か。*/
	bool isPressed = false;
	/** @brief 選択中か。*/
	bool isSelected = false;
public:
	//bool isDragAccept = false;
	//bool isDragging = false;
	//XMFLOAT2 lastMousePos;
public:
	/** @brief 表示に使用する画像コンポーネント。*/
	//Image* image = nullptr;
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"))
	ObjectId imageReference = ObjectId::Invalid(); // 画像コンポーネントの参照ID（シリアライズ用）
	/** @brief 既定色。*/
	C_PROPERTY()
	Color defaultColor{ 1,1,1,1 };
	/** @brief 選択時の色。*/
	C_PROPERTY()
	Color selectedColor{ 1.2f, 1.2f, 1.2f, 1.5f };
	/** @brief ホバー時の色。*/
	C_PROPERTY()
	Color hoveringColor{ 0.75f, 0.75f, 0.75f, 1.0f };
	/** @brief 押下時の色。*/
	C_PROPERTY()
	Color pressingColor{ 0.5f, 0.5f, 0.5f, 1.0f };

	/** @brief 無効時の色。*/
	C_PROPERTY()
	Color disabledColor{ 0.4f, 0.4f, 0.4f, 1.0f };
public:
	/** @brief 既定コンストラクタ。*/
	Selectable() = default;
	/** @brief デストラクタ。*/
	virtual ~Selectable() override = default;

	/** @brief ポインタ押下イベント。*/
	virtual void OnPointerDown(PointerEventData* eventData) override;
	/** @brief ポインタ解放イベント。*/
	virtual void OnPointerUp(PointerEventData* eventData) override;
	/** @brief ポインタ進入イベント。*/
	virtual void OnPointerEnter(PointerEventData* eventData) override;
	/** @brief ポインタ退出イベント。*/
	virtual void OnPointerExit(PointerEventData* eventData) override;
	/** @brief 選択イベント。*/
	virtual void OnSelect(BaseEventData* eventData) override;
	/** @brief 選択解除イベント。*/
	virtual void OnDeselect(BaseEventData* eventData) override;
	/** @brief 軸入力による移動イベント。*/
	virtual void OnMove(AxisEventData* eventData) override;

	/**
	 * @brief 現在ホバー中かを返します。
	 */
	bool IsHovering() const;

private:

	/**
	 * @brief 現在状態に応じて `image->color` を更新します。
	 * @details 優先度: 無効 > 押下 > ホバー > 選択 > 既定
	 */
	void UpdateVisual();

public:
	/** @brief 現在インタラクティブか。*/
	bool IsInteractable() const;
	/** @brief インタラクティブ状態を設定。*/
	void SetInteractable(bool value);
	/** @brief 画像コンポーネントを取得。*/
	Image* GetImage() const;

protected:
#ifdef USE_IMGUI
	/** @brief インスペクタ用プロパティ描画。*/
	virtual void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};