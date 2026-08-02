#include "pch.h"
#include "TakeAwayValue.h"
#include "Engine/Scenes/Scene.h"
#include "Ball.h"
#include "Engine/Physics/Collider.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(TakeAwayValue, "TakeAwayValue")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(TakeAwayValue, "UserScripts", ComponentAttributes::None, {})


void TakeAwayValue::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	//衝突イベントの登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnCollisionEnterEvent([this](const CollisionInfo& collisionInfo) { OnColliderEnter(collisionInfo); });
	}

	//ガジェットのタイプを妨害ガジェットに設定
	SetGadgetType(GadgetType::ObtrusiveGadget);
}

void TakeAwayValue::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void TakeAwayValue::OnColliderEnter(const CollisionInfo& collisionInfo)
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

void TakeAwayValue::OnAction()
{
	// ガジェットがアクションを実行したときの処理をここに実装します。
	//ボールの現在の価値を減らす
	if (auto rb = cachedBallRigidbody.lock())
	{
		if (auto ball = rb->GetOwner()->GetComponent<Ball>())
		{
			int currentValue = ball->GetValue();
			int decreaseAmount = static_cast<int>(currentValue * 0.1f); // 例: 現在の価値の10%を減少
			ball->IncreaseValue(decreaseAmount);
			Console::Log("Ball value decreased: " + std::to_string(ball->GetValue()));
		}
	}

	DecreaseDurability(); // ガジェットの耐久値を減らす
}

void TakeAwayValue::OnAttachment()
{
	// ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
	Gadget::OnAttachment(); // 基底クラスの処理を呼び出す
}
