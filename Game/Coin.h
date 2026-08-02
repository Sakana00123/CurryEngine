#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Physics/CollisionEvent.h"

class Coin : public Component
{
	C_REFLECT(Coin)
public:
	Coin() = default;
	~Coin() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnCollisionEnter(const CollisionInfo& collisionInfo);

	void SetVelocity(const Vector3& vel);

private:
	// 移動速度の減衰率(0.0f から 1.0f)。値が小さいほど速く減衰します。
	float damping = 0.4f;
	float accelerationY = -2.0f; // Y軸方向の加速度（重力のような効果）
	class Rigidbody* rb = nullptr; // Rigidbody コンポーネントへのキャッシュされたポインタ
};