#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Physics/CollisionEvent.h"

class CoinDestroyArea : public Component
{
	C_REFLECT(CoinDestroyArea)
public:
	CoinDestroyArea() = default;
	~CoinDestroyArea() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;


	void OnTriggerEnter(const TriggerInfo& info);

private:

};