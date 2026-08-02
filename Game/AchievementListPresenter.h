#pragma once
#include "Engine/Core/Component.h"

class AchievementListPresenter : public Component
{
	C_REFLECT(AchievementListPresenter)
public:
	AchievementListPresenter() = default;
	~AchievementListPresenter() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void RefreshAchievementList(); // 実績リストを更新する関数

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("AchievementManager"))
	ObjectId achievementManagerReference; // AchievementManager コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("ItemPageController"))
	ObjectId itemPageControllerReference; // ItemPageController コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Button"))
	ObjectId openButtonReference; // Button コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Button"))
	ObjectId closeButtonReference; // Button コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId achievementPanelReference; // 実績パネルオブジェクトへの参照

};