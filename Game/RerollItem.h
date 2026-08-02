#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class RerollItem : public Component
{
	C_REFLECT(RerollItem)
public:
	RerollItem() = default;
	~RerollItem() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void UpdateInteraction(); // インタラクションの更新処理を行う関数

	void UpdatePrice(); // 価格表示を更新する関数

	void OnRerollButtonClicked(); // ボタンがクリックされたときの処理

private:

	// ここにコンポーネントのメンバ変数を定義します。必要に応じて C_PROPERTY() マクロを使用してシリアライズ可能なプロパティを定義できます。
	C_PROPERTY()
	int initialPrice = 0; // 初期価格

	C_PROPERTY()
	float priceIncreaseRate = 0.2f; // 価格の増加率（例: 0.2 は前回の価格の20%増し）

	C_PROPERTY(CurryEngine::PropertyAttributes::NonSerialized, CurryEngine::PropertyAttributes::ReadOnly)
	int currentPrice = 0; // 現在の価格（シリアライズせず、エディタで読み取り専用）

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("PreserveValue"))
	ObjectId preserveValueId; // 関連する PreserveValue コンポーネントの ID を保持するプロパティ

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Button"))
	ObjectId rerollButtonId; // 関連する Button コンポーネントの ID を保持するプロパティ

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId priceTextId; // 関連する Text コンポーネントの ID を保持するプロパティ

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("ShopItemList"))
	ObjectId passiveShopItemListId; // 関連する ShopItemList コンポーネントの ID を保持するプロパティ

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("ShopItemList"))
	ObjectId gadgetShopItemListId; // 関連する ShopItemList コンポーネントの ID を保持するプロパティ

};