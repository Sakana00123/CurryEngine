#pragma once
#include "Gadget.h"
#include "Engine/Physics/CollisionEvent.h"
#include "Engine/Easing/EasingHandler.h"
#include "Engine/Physics/Rigidbody.h"

/// <summary>
/// バンパーコンポーネント
/// ボールが衝突すると反発力を与える
/// </summary>
class Bumper : public Gadget
{
	C_REFLECT(Bumper)

public:
	/**@brief 反発力の強さ **/
	C_PROPERTY()
		float bounceForce = 20.0f;

private:
	int bounceCount = 0; // バウンド回数のカウンタ
	static constexpr int maxBounceCount = 1; // 最大バウンド回数

	EasingHandler easingHandler; // イージングハンドラー

	Vector3 originalScale = Vector3(1.0f, 1.0f, 1.0f); // バンパーの元のスケール
	Vector3 targetScale = Vector3(1.5f, 1.5f, 1.5f); // バウンド時の目標スケール
	float currentScaleFactor = 1.0f; // 現在のスケールファクター

	float stayCollisionCooldown = 0.0f;  //Stay連続発火防止

	//衝突時の情報をキャッシュする変数
	std::weak_ptr<Rigidbody> cachedBallRigidbody;
	Vector3 cachedCollisionNormal;

public:
	Bumper() = default;
	virtual ~Bumper() override = default;

	void Start() override;
	void Update(float deltaTime) override;

	/// <summary>
	/// ボール衝突時のコールバック
	/// </summary>
	void OnCollisionEnter(const CollisionInfo& collisionInfo);

	void OnCollisionStay(const CollisionInfo& collisionInfo);

	void OnAction() override;
	void OnAttachment() override;
};