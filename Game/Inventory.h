#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "GadgetItemData.h"
#include "Engine/UI/RectTransform.h"


class Inventory : public Component
{
	C_REFLECT(Inventory)
public:
	Inventory() = default;
	~Inventory() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void Enter();
	void Exit();

	// アイテムをインベントリに追加する関数
	void AddItem(const GadgetItemData& item);

	//アイテムを消す
	void RemoveItem(RectTransform* itemRectTransform);

	//表示を更新する
	void UpdateDisplay();

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Button"))
		ObjectId shopButtonReference; // ショップボタンへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Button"))
		ObjectId nextRoundButtonReference; // 次ラウンドボタンへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
		ObjectId magnificationTextRootReference; // 倍率テキストのルートオブジェクトへの参照

	// インベントリUIのルートオブジェクトへの参照
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
		ObjectId inventoryReference;
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("UIEasing"))
		ObjectId uiEasingReference;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("BlendEasing"))
		ObjectId blendEasingReference;

	// アイテムリストの親オブジェクトへの参照
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
		ObjectId itemListReference;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Button"))
		ObjectId previousPageButtonReference; // 前のページボタンへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Button"))
		ObjectId nextPageButtonReference; // 次のページボタンへの参照

	C_PROPERTY()
		std::string itemPrefabPath; // アイテムのプレハブパス


	// 購入履歴（名前と価格のペア）
	std::vector<RectTransform*> purchasedItems;
	std::vector<RectTransform*> removedItems; // 削除されたアイテムのリスト

};