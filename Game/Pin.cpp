#include "pch.h"
#include "Pin.h"
#include "Gadget.h"
#include "Engine/Effects/ParticleComponent.h"

REGISTER_COMPONENT(Pin, "Pin")


void Pin::Start()
{
	gadgetSpawnEffect = GetOwner()->AddComponent<ParticleComponent>(); // ガジェットスポーンエフェクトコンポーネントを追加
	gadgetSpawnEffect->Load("Assets/Effects/PutInGadgetEffect.json"); // ガジェットスポーンエフェクトのエフェクトファイルをロード
	obtrusiveGadgetSpawnEffect = GetOwner()->AddComponent<ParticleComponent>(); // 妨害ガジェットスポーンエフェクトコンポーネントを追加
	obtrusiveGadgetSpawnEffect->Load("Assets/Effects/PutInDebuffGadgetEffect.json"); // 妨害ガジェットスポーンエフェクトのエフェクトファイルをロード
	gadgetBreakEffect = GetOwner()->AddComponent<ParticleComponent>(); // ガジェットブレイクエフェクトコンポーネントを追加
	gadgetBreakEffect->Load("Assets/Effects/BreakGadgetEffect.json"); // ガジェットブレイクエフェクトのエフェクトファイルをロード
}

void Pin::Update(float deltaTime)
{
}

void Pin::Equip(GameObject* obj, const Vector3& spawnPosition, const Quaternion& spawnRotation, const Vector3& spawnScale)
{
	equippedObject = obj;
	obj->SetParent(GetOwner()); // 装備したオブジェクトをピンの子にする
	obj->GetTransform()->SetPosition(spawnPosition); // 装備したオブジェクトのローカル位置を設定
	obj->GetTransform()->SetRotation(spawnRotation); // 装備したオブジェクトのローカル回転を設定
	obj->GetTransform()->SetWorldScale(spawnScale); // 装備したオブジェクトのローカルスケールを設定

	// ピンのコンポーネントをすべて無効にする
	if (Gadget* gadget = GetOwner()->GetComponentInChildren<Gadget>())
	{
		gadget->OnAttachment(); // ガジェットがアタッチされたときのイベントを呼び出す

		if (gadget->GetGadgetType() == Gadget::GadgetType::AllyGadget)
		{
			if (gadgetSpawnEffect)
			{
				gadgetSpawnEffect->Play(); // ガジェットスポーンエフェクトを再生
			}
		}
		else if (gadget->GetGadgetType() == Gadget::GadgetType::ObtrusiveGadget)
		{
			if (obtrusiveGadgetSpawnEffect)
			{
				obtrusiveGadgetSpawnEffect->Play(); // 妨害ガジェットスポーンエフェクトを再生
			}
		}
	}
}

GameObject* Pin::Unequip()
{
	if (equippedObject)
	{
		equippedObject->SetParent(nullptr); // 装備を解除したオブジェクトの親を解除
		GameObject* temp = equippedObject;
		equippedObject = nullptr;

		// ピンのコンポーネントをすべて有効にする
		for (auto& component : GetOwner()->GetAllComponents())
		{
			if (component)
			{
				component->SetEnabled(true);
			}
		}

		return temp; // 装備していたオブジェクトを返す
	}
	return nullptr; // 装備していなかった場合は nullptr を返す
}

void Pin::PlayBreakEffect()
{
	if (gadgetBreakEffect)
	{
		gadgetBreakEffect->Play(); // ガジェットブレイクエフェクトを再生
	}
}

REGISTER_COMPONENT(GoldenPin, "Pin")

void GoldenPin::Start()
{
}

void GoldenPin::Update(float deltaTime)
{
}

void GoldenPin::Equip(GameObject* obj, const Vector3& spawnPosition, const Quaternion& spawnRotation, const Vector3& spawnScale)
{
	Pin::Equip(obj, spawnPosition, spawnRotation, spawnScale); // 基底クラスの装備処理を呼び出す
	// ゴールデンピン特有の装備処理があればここに追加
}

GameObject* GoldenPin::Unequip()
{
	return Pin::Unequip(); // 基底クラスの装備解除処理を呼び出す
}