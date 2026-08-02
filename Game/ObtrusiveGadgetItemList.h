#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class ObtrusiveGadgetItemList : public Component
{
	C_REFLECT(ObtrusiveGadgetItemList)
public:
	ObtrusiveGadgetItemList() = default;
	~ObtrusiveGadgetItemList() = default;

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
	void RerollItem();

	void SpawnItem(int itemIndex); // アイテムを出現させる関数。itemIndex は itemPrefabPaths のインデックスを指定します。

	// 残りラウンド数を更新する関数
	void UpdateRoundText(int roundsRemaining);

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("RoundManager"))
	ObjectId roundManagerReference; // 出現までの残りラウンドを管理するオブジェクトへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId roundTextReference; // 出現までの残りラウンドを表示するテキストコンポーネントへの参照


	std::vector<std::string> itemPrefabPaths; // アイテムのプレハブのファイルパスのリスト
	std::vector<int> itemSpawnedList; // すでに出現しているアイテムのインデックスのリスト。これを使用して、同じアイテムが複数回出現するのを防ぎます。
};
