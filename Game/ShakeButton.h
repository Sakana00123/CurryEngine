#pragma once
#include "Engine/UI/UIComponent.h"

class ShakeButton : public UIComponent
{
	C_REFLECT(ShakeButton)
public:
	ShakeButton() = default;
	~ShakeButton() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Shaker"))
		ObjectId shakerReference; // Shaker コンポーネントへの参照

};