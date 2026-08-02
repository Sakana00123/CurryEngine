#pragma once
#include "Engine/UI/UIComponent.h"
#include "Engine/Core/Transform.h"

class UIWorldAnchor : public UIComponent
{
	C_REFLECT(UIWorldAnchor)
public:
	UIWorldAnchor() = default;
	~UIWorldAnchor() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Transform"))
	ObjectId targetTransformReference; // 例: Transform コンポーネントへの参照

};