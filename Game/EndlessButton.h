#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class EndlessButton : public Component
{
	C_REFLECT(EndlessButton)
public:
	EndlessButton() = default;
	~EndlessButton() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnClick(); // ボタンがクリックされたときの処理を行う関数

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("RankingManager"))
		ObjectId rankingManagerReference; // RankingManager コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("RoundManager"))
		ObjectId roundManagerReference; // RoundManager コンポーネントへの参照


};