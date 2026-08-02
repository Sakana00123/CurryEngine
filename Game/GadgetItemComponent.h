#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "GadgetItemData.h"

class GadgetItemComponent : public Component
{
	C_REFLECT(GadgetItemComponent)
public:
	GadgetItemComponent() = default;
	~GadgetItemComponent() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	//void DrawProperty() override; // エディタでプロパティを描画するためのオーバーライド関数

	// シリアライズ関数
	json Serialize() const override;

	// デシリアライズ関数
	void Deserialize(const json& j) override;

	// アイテムのデータを設定する関数
	void SetItemData(const GadgetItemData& newData, bool reloadTexture = false);

	// アイテムのデータを取得する関数
	GadgetItemData GetItemData() const { return data; } // アイテムデータを取得する関数

private:

	GadgetItemData data; // アイテムのデータ
};
