#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class DialoguePlayer : public Component
{
	C_REFLECT(DialoguePlayer)
public:
	DialoguePlayer() = default;
	~DialoguePlayer() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId dialogueUIReference; // ダイアログUIへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId textReference; // テキストオブジェクトへの参照


	std::wstring tempDialogue; // ダイアログテキストの一時変数(全文保持用)

};