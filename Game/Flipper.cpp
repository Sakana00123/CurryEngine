#include "pch.h"
#include "Flipper.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Core/GameObject.h"
#include "Pin.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Physics/Rigidbody.h"
#include "Ball.h"
#include "PassiveSkillContainer.h"
#include "Engine/Physics/Collider.h"

REGISTER_COMPONENT(Flipper, "Flipper")

void Flipper::Start()
{
	// フリッパーの初期角度を保存
	startAngle = GetTransform()->GetEulerAngles().y;

	// ガジェットのタイプを味方ガジェットに設定
	SetGadgetType(GadgetType::AllyGadget);

	// 衝突イベントのコールバックを登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnCollisionEnterEvent([this](const CollisionInfo& collisionInfo) { OnCollisionEnter(collisionInfo); });

	}


}

void Flipper::Update(float deltaTime)
{
	if(IsDisabled())
		return; // 無効化されている場合は何もしない

	// 入力に応じてフリッパーを回転させる
	bool trigger = false;
	trigger |= InputSystem::GetInputState("Space", InputStateMask::Trigger);
	trigger |= InputSystem::GetKeyTrigger(VK_RBUTTON);
	if (trigger)
	{
		if (flipDirection == 0) // 停止状態から回転開始
			flipDirection = 1;
		
	}
	// フリッパーの回転を更新
	switch (flipDirection)
	{
		case 1: // 回転中
		{
			// 回転進捗を更新
			currentFlipRate += flipForce * deltaTime / flipAngle;
			if (currentFlipRate >= 1.0f)
			{
				currentFlipRate = 1.0f;
				flipDirection = -1; // 反転開始
				Audio::PlayOneShot(L"./Assets/Sounds/SE/Flipper.wav");
			}
			break;
		}
		case -1: // 反転中
		{
			// 回転進捗を更新
			currentFlipRate -= flipForce * deltaTime / flipAngle;
			if (currentFlipRate <= 0.0f)
			{
				currentFlipRate = 0.0f;
				flipDirection = 0; // 停止
			}
			break;
		}
	default:
		return; // 停止中は何もしない
	}

	// フリッパーの角度を計算して適用
	float targetAngle = startAngle + (isRightFlipper ? 1 : -1) * flipAngle * currentFlipRate;
	Vector3 eulerAngles = GetTransform()->GetEulerAngles();
	eulerAngles.y = targetAngle;
	GetTransform()->SetRotation(eulerAngles);
}

void Flipper::OnRoundEnd()
{
	// ラウンド終了時に耐久値を減らす
	DecreaseDurability();
}

void Flipper::OnCollisionEnter(const CollisionInfo& collision)
{
	if(collision.other == nullptr)
		return;

	//衝突相手がボールの場合、オートフリッパーを発動
	// 衝突相手がBallコンポーネントを持っているか確認
	Ball* ball = collision.other->GetComponent<Ball>();
	if (ball == nullptr)return;

	// オートフリッパーが有効で、フリッパーが停止状態の場合に発動
	bool isAutoFlipper = false;
	const auto& containers = GetScene()->FindComponents<PassiveSkillContainer>();
	if (!containers.empty())
	{
		PassiveSkillContainer* passiveSkillContainer = containers.front();
		if (passiveSkillContainer)
		{
			// パッシブスキルコンテナから "AutoFlipper" プロパティの修正値を取得して、オートフリッパーの有効状態を決定します。
			isAutoFlipper = static_cast<bool>(passiveSkillContainer->GetModifier("AutoFlipper")); // "AutoFlipper" は仮のプロパティ名。実際のプロパティ名に合わせて変更してください。
		}
	}

	if (isAutoFlipper && flipDirection == 0)
	{
		flipDirection = 1; // 回転開始
	}
}

void Flipper::OnAttachment()
{
	// フリッパーがオブジェクトにアタッチされたときの処理
	// 例えば、フリッパーの初期設定や、必要なコンポーネントの取得などを行うことができます。
	Gadget::OnAttachment(); // 基底クラスの処理も呼び出す
}
