#include "pch.h"
#include "DownSpeedArea.h"
#include "Engine/Physics/Rigidbody.h"
#include "Engine/Physics/Collider.h"
#include "Engine/Scenes/Scene.h"
#include "Ball.h"
#include "Engine/Physics/Physics.h"
#include "Engine/Effects/ParticleComponent.h"
#include "Engine/Audio/Audio.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(DownSpeedArea, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(DownSpeedArea, "UserScripts", ComponentAttributes::None, {})


void DownSpeedArea::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。

	//衝突イベントの登録
	Collider* collider = GetOwner()->GetComponent<Collider>();
	if (collider != nullptr)
	{
		collider->AddOnTriggerEnterEvent([this](const TriggerInfo& collisionInfo) { OnTriggerEnter(collisionInfo); });
		//collider->AddOnTriggerStayEvent([this](const TriggerInfo& collisionInfo) { OnTriggerStay(collisionInfo); });
	}

	//ガジェットタイプを妨害ガジェットに設定
	SetGadgetType(GadgetType::ObtrusiveGadget);

	extraGravityScale = 5.0f;
	enterDownSpeedRate = 2.0f;

}

void DownSpeedArea::Update(float deltaTime)
{
	if(GetDurability() <= 0)
	{
		Deactivate();
	}
}

void DownSpeedArea::OnTriggerEnter(const TriggerInfo& collisionInfo)
{
	// トリガーに入ったオブジェクトがボールかどうかを確認
	if (!collisionInfo.otherCollider)
		return;

	GameObject* otherObject = collisionInfo.otherCollider->GetOwner();
	//if (triggeredObjects.find(otherObject) != triggeredObjects.end())
	//{
	//	// 既に処理済みのオブジェクトの場合は何もしない
	//	return;
	//}

	// 処理済みとしてセットに追加
	//triggeredObjects.insert(otherObject);

	Ball* ball = collisionInfo.otherCollider->GetOwner()->GetComponent<Ball>();
	if (ball == nullptr) return;

	// ボールの速度を減速させる
	Rigidbody* rb = ball->GetOwner()->GetComponent<Rigidbody>();
	if (rb == nullptr) return;

	Vector3 velocity = rb->GetVelocity();
	rb->SetVelocity(velocity * enterDownSpeedRate);

	// 追加の重力を適用
	Vector3 extraGravity(0.0f, Physics::GetGravity().y * extraGravityScale, 0.0f);
	rb->AddForce(extraGravity, ForceMode::Acceleration);


	// 音を再生
	Audio::PlayOneShot(L"./Assets/Sounds/SE/downSpeed.wav", 0.5f);


	DecreaseDurability();
}

void DownSpeedArea::OnTriggerStay(const TriggerInfo& collisionInfo)
{
	if (!collisionInfo.otherCollider || !collisionInfo.otherCollider->GetOwner())
		return;
	// トリガー内にいるオブジェクトがボールかどうかを確認
	Ball* ball = collisionInfo.otherCollider->GetOwner()->GetComponent<Ball>();
	if (ball == nullptr) return;

	// ボールの速度をさらに減速させる
	Rigidbody* rb = ball->GetOwner()->GetComponent<Rigidbody>();
	if (rb == nullptr) return;

	Vector3 velocity = rb->GetVelocity();
	rb->SetVelocity(velocity * stayDownSpeedRate);

	
}

void DownSpeedArea::OnPreviewEnter()
{
}

void DownSpeedArea::OnPreviewExit()
{
}

void DownSpeedArea::OnActivate()
{
	// ガジェットがアクティブ化されたときの処理をここに実装します。
	/*if(ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	{
		particle->Play();
	}*/
}

void DownSpeedArea::OnDeactivate()
{
	//トリガーされたオブジェクトのセットをクリアして、再度同じオブジェクトがトリガーに入ったときに減速処理が行われるようにします。
	//triggeredObjects.clear();

	// ガジェットが非アクティブ化されたときの処理をここに実装します。
	/*if (ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	{
		particle->Stop();
	}*/
	Gadget::OnDeactivate(); // 基底クラスの OnDeactivate を呼び出すことで、ガジェットの非アクティブ化処理も行います。
}

void DownSpeedArea::OnBreak()
{
	//triggeredObjects.clear(); // ガジェットが壊れたときに、トリガーされたオブジェクトのセットをクリアして、再度同じオブジェクトがトリガーに入ったときに減速処理が行われるようにします。

	// ガジェットが壊れたときの処理をここに実装します。
	// ガジェットが非アクティブ化されたときの処理をここに実装します。
	if (ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	{
		particle->Stop();
	}
	Gadget::OnBreak(); // 基底クラスの OnBreak を呼び出すことで、ガジェットの壊れたときの処理も行います。
}

void DownSpeedArea::ClearBallSet()
{
	//triggeredObjects.clear(); // 管理しているボールのセットをクリアします。
}

void DownSpeedArea::OnAttachment()
{
	// ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
	Gadget::OnAttachment(); // 基底の処理（必要に応じて）

	if (ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	{
		particle->Play();
	}
}