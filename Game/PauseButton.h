#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class PauseButton : public Component
{
	C_REFLECT(PauseButton)
public:
	PauseButton() = default;
	~PauseButton() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

private:

	C_PROPERTY()
	bool resumeFlag = false; // ゲーム再開のフラグ

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId pauseMenuReference; // ポーズメニューの GameObject への参照

};