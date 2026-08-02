#include "pch.h"
#include "GadgetSleep.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Effects/ParticleComponent.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Physics/Rigidbody.h"
#include <limits>
#include "Engine/Physics/Collider.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(GadgetSleep, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(GadgetSleep, "UserScripts", ComponentAttributes::None, {})


void GadgetSleep::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	//衝突イベントの登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnTriggerStayEvent([this](const TriggerInfo& info) { OnTriggerStay(info); });
	}

	//ガジェットタイプを妨害ガジェットに設定
	SetGadgetType(GadgetType::ObtrusiveGadget);
}

void GadgetSleep::Update(float deltaTime)
{
	
}

void GadgetSleep::OnTriggerStay(const TriggerInfo& triggerInfo)
{
	if (!triggerInfo.otherCollider || !triggerInfo.otherCollider->GetOwner())
		return;

	GameObject* obj = triggerInfo.otherCollider->GetOwner();

	// 親オブジェクトも含めてGadgetを探す
	Gadget* gadget = obj->GetComponentInChildren<Gadget>();
	if (gadget == nullptr)
	{
		if (GameObject* parent = obj->GetParent())
			gadget = parent->GetComponentInChildren<Gadget>();
	}

	if (gadget == nullptr) return;
	if (gadget == this) return;
	if (gadget->GetGadgetType() != GadgetType::AllyGadget) return;
	if (gadget->IsDisabled()) return;

	gadget->SetDisabled(true);

	for(ParticleComponent* particle : gadget->GetOwner()->GetComponents<ParticleComponent>())
	{
		particle->Play();
	}

	targetGadgets.push_back(gadget);
}

void GadgetSleep::OnPreviewEnter()
{
}

void GadgetSleep::OnPreviewExit()
{
}

void GadgetSleep::OnDeactivate()
{
	// ガジェットが非アクティブ化されたときの処理をここに実装します。
	for(ParticleComponent* particle : GetOwner()->GetComponents<ParticleComponent>())
	{
		particle->Stop();
	}

	// 自分が無効化された際に、味方の無効化状態をすべて元に戻す
	for (Gadget* gadget : targetGadgets)
	{
		if (gadget != nullptr)
		{
			gadget->SetDisabled(false);
			for(ParticleComponent* particle : gadget->GetOwner()->GetComponents<ParticleComponent>())
			{
				particle->Stop(); // かけたエフェクトを停止する
			}
		}
	}
	targetGadgets.clear(); // リストを空にする
}

void GadgetSleep::OnAttachment()
{
	// ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
	for (ParticleComponent* particle : GetOwner()->GetComponents<ParticleComponent>())
	{
		particle->Play();
	}

	Gadget::OnAttachment(); // 基底クラスの処理も呼び出す
}

void GadgetSleep::OnRoundEnd()
{
	// ラウンド終了時の処理をここに実装します。
	for(ParticleComponent* particle : GetOwner()->GetComponents<ParticleComponent>())
	{
		particle->Stop();
	}

	// ご自身が終了処理を行う際に、対象の無効化を解除する
	for (Gadget* gadget : targetGadgets)
	{
		if (gadget != nullptr)
		{
			gadget->SetDisabled(false);
			for(ParticleComponent* particle : gadget->GetOwner()->GetComponents<ParticleComponent>())
			{
				particle->Stop();
			}
		}
	}
	targetGadgets.clear(); // リストを空にする
		
	DecreaseDurability(); // 耐久値を減らす
}
