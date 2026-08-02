#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "PassiveSkillData.h"

class PassiveSkillView : public Component
{
	C_REFLECT(PassiveSkillView)
public:
	PassiveSkillView() = default;
	~PassiveSkillView() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	// パッシブスキルのデータを設定する関数
	void SetPassiveSkillData(const PassiveSkillData& newData);

	// スタック数を更新する関数
	void UpdateStackCount(int newCount);

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"))
		ObjectId iconImageReference; // パッシブスキルのアイコンを表示するイメージコンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("ItemTooltipController"))
		ObjectId tooltipReference; // スキルのツールチップを表示するコントローラーへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId itemCountReference; // スキルの個数を表示するテキストへの参照
		
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"))
		ObjectId backgroundImageReference; // スキルの背景画像への参照

};