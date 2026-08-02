#include "pch.h"
#include "ValueETC.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Physics/Collider.h"
#include "Ball.h"
#include "Engine/Effects/ParticleComponent.h"
#include "Engine/Audio/Audio.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(ValueETC, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(ValueETC, "UserScripts", ComponentAttributes::None, {})


void ValueETC::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	//衝突イベントの登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnTriggerEnterEvent([this](const TriggerInfo& triggerInfo) { OnTriggerEnter(triggerInfo); });
		collider->AddOnTriggerExitEvent([this](const TriggerInfo& triggerInfo) { OnTriggerExit(triggerInfo); });
	}

	//ガジェットタイプを妨害ガジェットに設定
	SetGadgetType(GadgetType::ObtrusiveGadget);
}

void ValueETC::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void ValueETC::OnTriggerEnter(const TriggerInfo& info)
{
	if (info.other == nullptr) return;

	Ball* ball = info.otherCollider->GetOwner()->GetComponent<Ball>();
	if (ball == nullptr)
		return;

	// すでに減らしていたら無視
	if (triggeredBalls.count(ball) > 0)
		return;

	ball->IncreaseValue(-decreaseAmount);
	triggeredBalls.insert(ball);
	Console::Log("ValueETC: Ball value decreased to " + std::to_string(ball->GetValue()));
	Audio::PlayOneShot(L"./Assets/Sounds/SE/decreaseValue.wav");

	if (ball->GetValue() <= 0) 
	{
		ball->SetValue(0); // 価値が0以下にならないようにする
	}
}

void ValueETC::OnTriggerExit(const TriggerInfo& info)
{
	if(info.other == nullptr) return;

	Ball* ball = info.otherCollider->GetOwner()->GetComponent<Ball>();
	if (ball == nullptr)
		return;

	// トリガーから出たボールを管理セットから削除
	triggeredBalls.erase(ball);
}

void ValueETC::OnActivate()
{
	// ガジェットがアクションを実行したときの処理をここに実装します。
	
}

void ValueETC::OnDeactivate()
{
	// ガジェットがアクションを実行したときの処理をここに実装します。
}

void ValueETC::OnPreviewEnter()
{
}

void ValueETC::OnPreviewExit()
{
}

void ValueETC::OnAttachment()
{
	// ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
	Gadget::OnAttachment(); // 基底クラスの処理を呼び出す

	if (ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	{
		particle->Play();
	}
}

void ValueETC::OnRoundEnd()
{
	// ラウンドの終了時の処理をここに実装します。
	DecreaseDurability(); // ガジェットの耐久値を減らす
}