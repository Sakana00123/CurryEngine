#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Events/EventHandlers.h"

class TutorialClickAdvance : public Component, public IPointerClickHandler
{
	C_REFLECT(TutorialClickAdvance)
public:
	TutorialClickAdvance() = default;
	~TutorialClickAdvance() = default;

public:

	void OnEnable() override; // コンポーネントが有効になったときの処理をオーバーライドして実装します。

	void OnDisable() override; // コンポーネントが無効になったときの処理をオーバーライドして実装します。

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnPointerClick(PointerEventData* eventData) override; // クリックイベントの処理

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("TutorialSystem"))
	ObjectId tutorialSystemReference; // TutorialSystem コンポーネントへの参照

};