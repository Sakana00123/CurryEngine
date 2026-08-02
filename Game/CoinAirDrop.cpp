#include "pch.h"
#include "CoinAirDrop.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Physics/Collider.h"
#include "Ball.h"
#include "Engine/Physics/Rigidbody.h"
#include "Engine/Effects/ParticleComponent.h"
#include "Engine/Audio/Audio.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(CoinAirDrop, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(CoinAirDrop, "UserScripts", ComponentAttributes::None, {})


void CoinAirDrop::Start()
{
	Gadget::Start(); // 基底クラスの Start() を呼び出す

	// コンポーネントが開始されたときの処理をここに実装します。
	// 衝突イベントの登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnCollisionEnterEvent([this](const CollisionInfo& collisionInfo) { OnCollisionEnter(collisionInfo); });
	}
}

void CoinAirDrop::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void CoinAirDrop::OnCollisionEnter(const CollisionInfo& collisionInfo)
{
	if (collisionInfo.otherCollider == nullptr ||
		collisionInfo.otherCollider->GetOwner() == nullptr) return;

	if (collisionInfo.otherCollider->GetOwner()->GetComponent<Ball>() == nullptr) return;

	currentCollisionCount++;

	// 毎回コインを散布
	PerformAction();

	Audio::PlayOneShot(L"./Assets/Sounds/SE/coinAirDrop.wav", 0.5f);
}

void CoinAirDrop::OnPreviewEnter()
{
	// ピンに近づけたときの処理をここに実装します。
	if(ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	{
		particle->Play();
	}
}

void CoinAirDrop::OnPreviewExit()
{
	// ピンから離れたときの処理をここに実装します。
	if(ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	{
		particle->Stop();
	}
}

void CoinAirDrop::OnAction()
{
	// ラウンド終了時の処理をここに実装します。

	// コインをドロップする処理を実装
	for (int i = 0; i < coinCount; i++)
	{
		Vector3 spawnPos = GetOwner()->GetTransform()->GetWorldPosition();
		spawnPos.x += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f;
		spawnPos.y += 0.05f;
		spawnPos.z -= 0.05f;

		GameObject* coin = Component::Instantiate(coinPrefabPath, { 0,0,0 }, { 0,0,0,1 });
		coin->GetTransform()->SetWorldPosition(spawnPos);

		float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265f;
		float speed = 0.3f + static_cast<float>(rand()) / RAND_MAX * 2.0f;

		Vector3 vel;
		vel.x = std::cos(angle) * speed;
		vel.y = 0.1f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
		vel.z = std::sin(angle) * speed;

		if (Coin* coinComp = coin->GetComponent<Coin>())
		{
			coinComp->SetVelocity(vel);
		}
	}

	DecreaseDurability(); // ガジェットの耐久値を減らす
}

void CoinAirDrop::OnDeactivate()
{
	// ガジェットが非アクティブ化されたときの処理をここに実装します。
	//if(ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	//{
	//	particle->Stop();
	//}
}

void CoinAirDrop::OnActivate()
{
	// ガジェットがアクティブ化されたときの処理をここに実装します。
	//if(ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	//{
	//	particle->Play();
	//}
}

void CoinAirDrop::OnAttachment()
{
	// ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
	Gadget::OnAttachment(); // 基底の処理（必要に応じて）
}