#pragma once
#include "Engine/UI/UIComponent.h"
#include "Engine/Core/Transform.h"

class Exit : public Component
{
	C_REFLECT(Exit)
public:
	Exit() = default;
	~Exit() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;


private:


};