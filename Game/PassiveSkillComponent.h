#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "PassiveSkillData.h"

class PassiveSkillComponent : public Component
{
	C_REFLECT(PassiveSkillComponent)
public:
	PassiveSkillComponent() = default;
	~PassiveSkillComponent() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	//void DrawProperty() override; // エディタでプロパティを描画するためのオーバーライド関数

	// シリアライズ関数
	json Serialize() const override;

	// デシリアライズ関数
	void Deserialize(const json& j) override;

	// パッシブスキルのデータを設定する関数
	void SetPassiveSkillData(const PassiveSkillData& newData, bool reloadTexture = false);

	// パッシブスキルのデータを取得する関数
	const PassiveSkillData& GetPassiveSkillData() const { return data; }

private:

	PassiveSkillData data;
};
