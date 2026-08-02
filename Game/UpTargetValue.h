#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Physics/CollisionEvent.h"
#include "Engine/Physics/Rigidbody.h"
#include "Gadget.h"

class UpTargetValue : public Gadget
{
	C_REFLECT(UpTargetValue)
public:
	UpTargetValue() = default;
	~UpTargetValue() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnCollisionEnter(const CollisionInfo& collisionInfo);

	void OnAction() override; // ラウンド終了時の処理
	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理

private:

	std::weak_ptr<Rigidbody> cachedBallRigidbody; // Rigidbodyのキャッシュ
	Vector3 cachedCollisionNormal; // 衝突法線のキャッシュ
};