#pragma once
#include "Engine/Core/Component.h"
#include "Engine/UI/RectTransform.h"

class ShopItemList : public Component
{
	C_REFLECT(ShopItemList)
public:
	ShopItemList() = default;
	~ShopItemList() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	// プロパティを描画する関数
	//void DrawProperty() override;

	// シリアライズ関数
	json Serialize() const override;

	// デシリアライズ関数
	void Deserialize(const json& j) override;
	

	// アイテムを再生成する関数
	void RerollItems();

private:

	// アイテム間の間隔（サイズとスペースの合計）
	C_PROPERTY()
	float spacing = 350.0f;

	C_PROPERTY()
	int itemCount = 5; // アイテムの数

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("PassiveSkillContainer"), CurryEngine::PropertyAttributes::Tooltip("item container reference"))
	ObjectId itemContainerReference; // アイテムを配置するコンテナの参照

	// アイテムのプレハブのファイルパスのリスト
	struct ItemEntry
	{
		std::string prefabPath; // アイテムのプレハブのファイルパス
		int stackCount; // アイテムのスタック数
		int rarity; // アイテムのレアリティ(0=Common, 1=Rare, 2=Legendary)
	};
	std::vector<ItemEntry> itemEntries; // アイテムのエントリのリスト

};
