#pragma once
#include "Engine/UI/UIComponent.h"
#include "AchievementData.h"

class AchievementListItem : public UIComponent
{
	C_REFLECT(AchievementListItem)
public:
	AchievementListItem() = default;
	~AchievementListItem() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void SetAchievementData(const AchievementData& data); // 実績データを設定する関数

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId nameTextReference; // 実績名を表示する Text コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId descriptionTextReference; // 実績の説明を表示する Text コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId progressTextReference; // 実績の進行状況を表示する Text コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId unlockedDateTextReference; // 実績の解除日時を表示する Text コンポーネントへの参照

};