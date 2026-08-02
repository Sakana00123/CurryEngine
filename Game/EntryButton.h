#pragma once
#include "Engine/UI/UIComponent.h"

class EntryButton : public UIComponent
{
	C_REFLECT(EntryButton)
public:
	EntryButton() = default;
	~EntryButton() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("InputField"))
	ObjectId InputFieldRef;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("PhaseManager"))
	ObjectId PhaseManagerRef;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId EntryPanelRef; // EntryPanel GameObject への参照

};