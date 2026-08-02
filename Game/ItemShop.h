#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "MagnificationZone.h"

class ItemShop : public Component
{
	C_REFLECT(ItemShop)
public:
	ItemShop() = default;
	~ItemShop() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void UpdateInteraction();

	// アイテムが購入されたかどうかを取得する関数
	bool IsPurchased() const { return isPurchased; }

	// アイテムを購入する関数
	void Purchase();

	void ResetPurchase(); // 購入フラグをリセットする関数


	//void DrawProperty() override; // エディタでプロパティを描画するためのオーバーライド関数

private:
	C_PROPERTY(CurryEngine::PropertyAttributes::HideInInspector)
	int itemType = 1; // アイテムの種類を表す(0: Passive, 1: Gadget)


	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Button"), CurryEngine::PropertyAttributes::Tooltip("button reference"))
	ObjectId buttonReference; // 例: Button コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"), CurryEngine::PropertyAttributes::Tooltip("icon image"))
	ObjectId iconImageReference; // アイコンを表示するイメージコンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"), CurryEngine::PropertyAttributes::Tooltip("background image"))
	ObjectId backgroundImageReference; // アイテムの背景画像への参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"), CurryEngine::PropertyAttributes::Tooltip("price text"))
	ObjectId priceTextReference; // アイテムの価格を表示するテキストコンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("ItemTooltipController"), CurryEngine::PropertyAttributes::Tooltip("tooltip controller"))
	ObjectId tooltipReference; // ツールチップを表示するコントローラーへの参照


	C_PROPERTY()
		int itemPrice = 100; // アイテムの価格

	bool isPurchased = false; // アイテムが購入されたかどうかのフラグ

	//C_PROPERTY()
	//	std::string itemPrefabPath; // アイテムのプレハブのファイルパス

};
