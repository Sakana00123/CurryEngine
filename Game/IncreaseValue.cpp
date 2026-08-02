#include "pch.h"
#include "IncreaseValue.h"
#include "Ball.h"
#include "Engine/Physics/Collider.h"
#include "Engine/Effects/ParticleComponent.h"
#include "Engine/Audio/Audio.h"

REGISTER_COMPONENT(IncreaseValue, "UserScripts")

void IncreaseValue::Start()
{
    if (Collider* col = GetOwner()->GetComponent<Collider>())
    {
        //col->AddOnTriggerStayEvent([this](const TriggerInfo& info) { OnTriggerStay(info); });
        col->AddOnTriggerEnterEvent([this](const TriggerInfo& info) { OnTriggerEnter(info); });
        col->AddOnTriggerExitEvent([this](const TriggerInfo& info) { OnTriggerExit(info); });
    }

	//ガジェットタイプを味方ガジェットに設定
	SetGadgetType(GadgetType::AllyGadget);
}

void IncreaseValue::Update(float deltaTime)
{
    if (IsDisabled()) return; // ガジェットが無効化されている場合は何もしない

    // トリガー内にボールがいなければタイマーリセット
    if (ballsInRange.empty())
    {
        increaseTimer = 0.0f;
        ballsInRange.clear();
        return;
    }

    increaseTimer += deltaTime;

    if (increaseTimer >= 1.0f / increaseSpeed)
    {
        increaseTimer = 0.0f;
        for (Ball* ball : ballsInRange)
        {
            if (ball == nullptr)
                continue;
            ball->IncreaseValue(1);
            Audio::PlayOneShot(L"./Assets/Sounds/SE/IncreaseValue.wav");
			
            Console::Log("Increased ball value by 1. New value: " + std::to_string(ball->GetValue()));
        }

        DecreaseDurability(); // ガジェットの耐久値を減少させる
        Audio::PlayOneShot(L"./Assets/Sounds/SE/IncreasevalueAura.wav");
    }

    if (GetDurability() <= 0)
    {
        Deactivate(); // ガジェットが壊れたときの処理を呼び出す
    }

    // 毎フレームリセットしてStayで再登録する
    //ballsInRange.clear();
}

//void IncreaseValue::OnTriggerStay(const TriggerInfo& info)
//{
//    if (IsDisabled()) return; // ガジェットが無効化されている場合は何もしない
//
//    if (!info.otherCollider || !info.otherCollider->GetOwner())
//		return;
//    Ball* ball = info.otherCollider->GetOwner()->GetComponent<Ball>();
//    if (ball == nullptr)
//        return;
//
//    ballsInRange.insert(ball);
//}

void IncreaseValue::OnTriggerEnter(const TriggerInfo& info)
{
    //if (IsDisabled()) return; // ガジェットが無効化されている場合は何もしない
    if (!info.otherCollider || !info.otherCollider->GetOwner())
        return;
    Ball* ball = info.otherCollider->GetOwner()->GetComponent<Ball>();
    if (ball == nullptr)
        return;
    ballsInRange.insert(ball);
}

void IncreaseValue::OnTriggerExit(const TriggerInfo& info)
{
    //if (IsDisabled()) return; // ガジェットが無効化されている場合は何もしない
    if (!info.otherCollider || !info.otherCollider->GetOwner())
        return;
    Ball* ball = info.otherCollider->GetOwner()->GetComponent<Ball>();
    if (ball == nullptr)
        return;
    ballsInRange.erase(ball);
}

void IncreaseValue::OnRoundEnd()
{
    //// ラウンド終了時に値をリセット
    //for (Ball* ball : ballsInRange)
    //{
    //    if (ball == nullptr)
    //        continue;
    //    ball->ResetValue();
    //    Console::Log("Ball value reset to 1 at round end.");
    //}
    //ballsInRange.clear();

	
}

void IncreaseValue::OnActivate()
{
    // ガジェットがアクティブ化されたときの処理
}

void IncreaseValue::OnDeactivate()
{
    // ガジェットが非アクティブ化されたときの処理
	ballsInRange.clear(); // Stay中のボールのセットをクリアして、再度同じボールがトリガーに入ったときに増加処理が行われるようにします。
}

void IncreaseValue::OnPreviewEnter()
{
    // ピンに近づけたときの処理
    if (ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
    {
        // パーティクルを再生するなどの処理をここに実装します。
        particle->Play();
    }
}

void IncreaseValue::OnPreviewExit()
{
    // ピンから離れたときの処理
    if (ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
    {
        // パーティクルを停止するなどの処理をここに実装します。
        particle->Stop();
    }
}

void IncreaseValue::OnAttachment()
{
    // ガジェットがオブジェクトにアタッチされたときの処理
	Gadget::OnAttachment(); // 基底クラスの処理を呼び出す
}

void IncreaseValue::ClearBallSet()
{
    ballsInRange.clear();
	Console::Log("Cleared ball set in IncreaseValue gadget.");
}

void IncreaseValue::SetDisabled(bool disabled)
{
    Gadget::SetDisabled(disabled); // 基底クラスの処理を呼び出す
    if (disabled)
    {
        // ガジェットが無効化されたときの追加処理
        ballsInRange.clear(); // Stay中のボールのセットをクリアして、再度同じボールがトリガーに入ったときに増加処理が行われるようにします。
        Console::Log("IncreaseValue gadget has been disabled.");
    }
}