#include "pch.h"
#include "Coin.h"
#include "Engine/Scenes/Scene.h"
#include "Ball.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Physics/Collider.h"
#include "PreserveValue.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Physics/Rigidbody.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(Coin, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(Coin, "UserScripts", ComponentAttributes::None, {})


void Coin::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	// 衝突イベントのコールバックを登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnCollisionEnterEvent([this](const CollisionInfo& collisionInfo) { OnCollisionEnter(collisionInfo); });
	}

	if (rb = GetOwner()->GetComponent<Rigidbody>())
	{
		
	}
}

void Coin::Update(float deltaTime)
{
	if (rb && !rb->IsKinematic() && rb->IsSleeping() == false)
	{
		// 移動速度を減衰させる
		Vector3 velocity = rb->GetVelocity();
		velocity.x *= (1.0f - damping * deltaTime); // 減衰率を適用
		velocity.z *= (1.0f - damping * deltaTime); // 減衰率を適用
		velocity.y += accelerationY * deltaTime; // Y軸方向の加速度を適用

		if (velocity.Length() < 0.01f) // 速度が非常に小さい場合は停止させる
		{
			velocity = Vector3::Zero;
		}

		rb->SetVelocity(velocity);
	}
}

void Coin::OnCollisionEnter(const CollisionInfo& collisionInfo)
{
	// 衝突したときの処理をここに実装します。
	if (collisionInfo.other == nullptr)
		return;
	// ボール以外の衝突では何もしない
	if (collisionInfo.other->GetComponent<Ball>() == nullptr)
		return;
	// コインを破壊する
	GetOwner()->Destroy();

	//所持金を増加させる
	if (PreserveValue* preserveValue = GetScene()->GetObjectManager()->Find("PreserveValue")->GetComponent<PreserveValue>())
	{
		if (Ball* ball = collisionInfo.other->GetComponent<Ball>())
		{
			preserveValue->SaveTotalValue(ball->GetValue()); // 例: 所持金を1増やす
			Audio::PlayOneShot(L"./Assets/Sounds/SE/getCoin.wav", 0.5f);
			Console::Log("Coin collected! Total value: " + std::to_string(preserveValue->GetTotalValue()));
		}
	}

}

void Coin::SetVelocity(const Vector3& vel)
{
	if (rb)
	{
		rb->SetVelocity(vel);
	}
}