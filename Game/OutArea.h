#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Physics/CollisionEvent.h"

class OutArea : public Component
{
	C_REFLECT(OutArea)
public:
	OutArea() = default;
	~OutArea() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnTriggerEnter(const TriggerInfo& info); // トリガーに入ったときの処理

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Gate"))
	ObjectId gateReference; // ゲートオブジェクトへの参照

};