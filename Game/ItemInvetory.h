#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "GadgetItemData.h"

class ItemInvetory : public Component
{
	C_REFLECT(ItemInvetory)
public:
	ItemInvetory() = default;
	~ItemInvetory() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	// アイテムをインベントリに追加する関数
	void SetItem(const GadgetItemData& item);

	void OnUseItem(); // アイテムが使用されたときの処理

	void OnCancel(); // アイテムの使用がキャンセルされたときの処理

	void OnConfirmed(); // アイテムの使用が確定したときの処理

	//表示を更新する
	void UpdateDisplay();

private:

	// オブジェクト参照プロパティを定義する場合は、C_PROPERTY() マクロの引数に ObjectReference 属性を指定します。引数には参照先の型名を文字列で指定します。
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Button"))
		ObjectId buttonReference; // 例: Button コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"))
		ObjectId imageReference; // 例: Image コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"))
		ObjectId backgroundImageReference; // 例: アイテムの背景画像への参照


	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
		ObjectId attachReference; // 例: AttachManager コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("ItemTooltipController"))
		ObjectId tooltipReference; // 例: ItemTooltipController コンポーネントへの参照

	// 購入履歴（名前と価格のペア）
	GadgetItemData itemData;
	

};