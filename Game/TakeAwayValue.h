#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Physics/CollisionEvent.h"
#include "Engine/Physics/Rigidbody.h"
#include "Gadget.h"

class TakeAwayValue : public Gadget
{
	C_REFLECT(TakeAwayValue)
public:
	TakeAwayValue() = default;
	~TakeAwayValue() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnColliderEnter(const CollisionInfo& collisionInfo);

	void OnAction() override;
	void OnAttachment() override;
private:

	std::weak_ptr<Rigidbody> cachedBallRigidbody; // Rigidbodyのキャッシュ
	Vector3 cachedCollisionNormal; // 衝突法線のキャッシュ
	
};