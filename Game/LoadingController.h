#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class LoadingController : public Component
{
	C_REFLECT(LoadingController)
public:
	LoadingController() = default;
	~LoadingController() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

private:
	bool isFading;
	float fadeValue;

};