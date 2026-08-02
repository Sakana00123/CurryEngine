#include "pch.h"
#include "ChangeballColor.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Physics/Collider.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Effects/ParticleComponent.h"
#include "Engine/Audio/Audio.h"
#include <Engine\Rendering\Renderers\PrimitiveRenderer.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(ChangeballColor, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(ChangeballColor, "UserScripts", ComponentAttributes::None, {})


void ChangeballColor::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	//衝突イベントの登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnCollisionEnterEvent([this](const CollisionInfo& collisionInfo) { OnCollisionEnter(collisionInfo); });
	}

	// ガジェットのタイプを妨害ガジェットに設定
	SetGadgetType(GadgetType::ObtrusiveGadget);
}

void ChangeballColor::Update(float deltaTime)
{
	auto rb = cachedBallRigidbody.lock();
	if (!rb) return;

	Ball* ball = rb->GetOwner()->GetComponent<Ball>();
	if (ball == nullptr || !ball->IsWallCursed()) return;

	// タイマーを進めて一定間隔でエフェクトを再生
	effectTimer += deltaTime;
	if (effectTimer >= effectInterval)
	{
		effectTimer = 0.0f;

		GameObject* ballObj = rb->GetOwner();
		ParticleComponent* particle = ballObj->GetComponent<ParticleComponent>();
		if (particle == nullptr)
		{
			particle = ballObj->AddComponent<ParticleComponent>();
			particle->Load("./Assets/Effects/DegreateColorEffect.json");
		}
		if (particle)
		{
			particle->Play();  // 再生
		}
	}
}

void ChangeballColor::OnCollisionEnter(const CollisionInfo& info)
{
	if (info.other == nullptr)return;
	// 衝突相手がBallコンポーネントを持っているか確認
	Ball* ball = info.other->GetComponent<Ball>();
	if (ball == nullptr)return;
	
	cachedBallRigidbody = info.other->GetComponentShared<Rigidbody>();

	// 接触法線をキャッシュ
	ContactPoint contact = info.contacts.empty() ? ContactPoint{} : info.contacts[0];
	for (const auto& c : info.contacts)
	{
		if (c.thisCollider == GetOwner()->GetComponent<Collider>())
		{
			contact = c;
			break;
		}
	}

	cachedCollisionNormal = contact.normal;

	PerformAction(); // アクションを実行（耐久値の減少など）
}

void ChangeballColor::OnAction()
{
	if (auto rb = cachedBallRigidbody.lock())
	{
		if (Ball* ball = rb->GetOwner()->GetComponent<Ball>())
		{
			if (!ball->IsWallCursed())
			{
				ball->SetWallCurse(true);
				effectTimer = effectInterval; // 即座に1回再生させるため満タンにする

				if (auto* primitiveRenderer = ball->GetOwner()->GetComponent<PrimitiveRenderer>())
				{
					primitiveRenderer->material->SetValue("materialColor", Color(0.1f, 0.0f, 0.2f, 1.0f));
				}
			}
		}
	}
	DecreaseDurability();

	Audio::PlayOneShot(L"./Assets/Sounds/SE/changeBallColor.wav");
}


void ChangeballColor::OnDeactivate()
{
	// ガジェットがアクティブ化されたときの処理をここに実装します。
	//エフェクトを消す
	if (auto rb = cachedBallRigidbody.lock())
	{
		if (auto ball = rb->GetOwner()->GetComponent<Ball>())
		{
			ball->SetWallCurse(false);
			GameObject* ballObj = ball->GetOwner();
			if (ParticleComponent* particle = ballObj->GetComponent<ParticleComponent>())
			{
				particle->Stop(); // エフェクトを停止
				// 必要に応じてコンポーネントを削除することもできます
				 ballObj->RemoveComponent<ParticleComponent>();
			}
		}
	}
}

void ChangeballColor::OnPreviewEnter()
{

}

void ChangeballColor::OnPreviewExit()
{

}

void ChangeballColor::OnAttachment()
{
	// ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
	Gadget::OnAttachment(); // 基底クラスの処理を呼び出す

	if (ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	{
		particle->Play();
	}
}