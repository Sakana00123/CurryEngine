#include "pch.h"
#include "UpTargetValue.h"
#include "Engine/Scenes/Scene.h"
#include "Ball.h"
#include "PreserveValue.h"
#include "Engine/Physics/Collider.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(UpTargetValue, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(UpTargetValue, "UserScripts", ComponentAttributes::None, {})


void UpTargetValue::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	//衝突イベントの登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnCollisionEnterEvent([this](const CollisionInfo& collisionInfo) { OnCollisionEnter(collisionInfo); });
	}

	//ガジェットタイプを妨害ガジェットに設定
	SetGadgetType(GadgetType::AllyGadget);
}

void UpTargetValue::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void UpTargetValue::OnCollisionEnter(const CollisionInfo& collisionInfo)
{
	if (collisionInfo.other == nullptr) return;

	if (collisionInfo.other->GetComponent<Ball>() == nullptr) return;

	cachedBallRigidbody = collisionInfo.other->GetComponentShared<Rigidbody>();

	// 接触法線をキャッシュ
	ContactPoint contact = collisionInfo.contacts.empty() ? ContactPoint{} : collisionInfo.contacts[0];
	for (const auto& c : collisionInfo.contacts)
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

void UpTargetValue::OnAction()
{
	// ガジェットがアクションを実行したときの処理をここに実装します。

	if(auto rb = cachedBallRigidbody.lock())
	{
		// Rigidbodyが有効な場合の処理をここに実装します。
		for(auto* preserveValue : GetScene()->FindComponents<PreserveValue>())
		{
			if (preserveValue)
			{
				// 目標金額を現在の目標金額から10%増加させる
				preserveValue->SetTargetValue(static_cast<int>(preserveValue->GetTargetValue() * 1.1f));
				Console::Log("Target value increased: " + std::to_string(preserveValue->GetTargetValue()));
			}
		}
	}

	DecreaseDurability(); // ガジェットの耐久値を減らす
}

void UpTargetValue::OnAttachment()
{
	// ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
	Gadget::OnAttachment(); // 基底クラスの処理も呼び出す
}
