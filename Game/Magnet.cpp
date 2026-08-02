#include "pch.h"
#include "Magnet.h"
#include "Ball.h"
#include "Engine/Physics/Rigidbody.h"
#include "Engine/Physics/Collider.h"
#include <Engine/Effects/ParticleComponent.h>
#include "Engine/Audio/Audio.h"

REGISTER_COMPONENT(Magnet, "Magnet")

void Magnet::Start()
{
    if (Collider* col = GetOwner()->GetComponent<Collider>())
    {
        col->AddOnTriggerStayEvent([this](const TriggerInfo& info) { OnTriggerStay(info); });
    }

	//ガジェットタイプを味方ガジェットに設定
	SetGadgetType(GadgetType::AllyGadget);
}

void Magnet::Update(float deltaTime)
{
    if (IsDisabled()) return; // ガジェットが無効化されている場合は何もしない

	// 音のクールダウンを更新
    if (audioCooldown > 0.0f) 
    {
        audioCooldown -= deltaTime;
    }

    //磁力の持続時間をクールダウンを繰り返す
    if (isActive)
    {
        currentMagnetTime += deltaTime;
        if (currentMagnetTime >= magnetTime)
        {
            currentMagnetTime = 0.0f;
            currentCooldown = 0.0f;
            isActive = false; // クールダウンへ 
			//affectedObjects.insert(GetOwner()); // 自身を影響を受けるオブジェクトのセットに追加
        }
    }
    else
    {
        currentCooldown += deltaTime;
        if (currentCooldown >= cooldownTime)
        {
            currentCooldown = 0.0f;
            isActive = true; // 発動へ
        }
    }

    if(GetDurability() <= 0)
    {
        Deactivate(); // ガジェットが壊れたときの処理を呼び出す
	}
}

void Magnet::OnTriggerStay(const TriggerInfo& info)
{
    if (IsDisabled()) return; // ガジェットが無効化されている場合は何もしない

    if(isActive)
    {

        if (info.otherCollider == nullptr)
            return;
        Ball* ball = info.otherCollider->GetOwner()->GetComponent<Ball>();
        if (ball == nullptr)
            return;

        Rigidbody* rb = ball->GetOwner()->GetComponent<Rigidbody>();
        if (rb == nullptr)
            return;

        Vector3 dir = GetTransform()->GetPosition() - ball->GetTransform()->GetPosition();
        float distance = dir.LengthSq();
  
        if (distance < 0.01f)
            return;

        float forceMag = strength / (std::max)(distance, 1.0f);
        rb->AddForce(dir.Normalize() * forceMag, ForceMode::Acceleration);

        if (audioCooldown <= 0.0f)
        {
            Audio::PlayOneShot(L"./Assets/Sounds/SE/magnet.wav", 0.5f);
            // 1秒に1回だけ鳴るように設定
            audioCooldown = 1.0f;
        }
    }
}

void Magnet::OnRoundEnd()
{
    // ラウンド終了時の処理をここに実装します。
    DecreaseDurability(); // ガジェットの耐久値を減少させる
}

void Magnet::OnActivate()
{
    // ガジェットがアクションを実行したときの処理をここに実装します。

    
}

void Magnet::OnDeactivate()
{
    // ガジェットがアクションを実行したときの処理をここに実装します。
    if(ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
    {
        // パーティクルを停止するなどの処理をここに実装します。
        particle->Stop();
    }
}

void Magnet::OnPreviewEnter()
{
    // ピンに近づけたときのプレビュー表示開始の処理をここに実装します。
    if (ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
    {
        // パーティクルを再生するなどの処理をここに実装します。
        particle->Play();
    }
}

void Magnet::OnPreviewExit()
{
    // ピンから離れたときのプレビュー表示終了の処理をここに実装します。
    if (ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
    {
        // パーティクルを停止するなどの処理をここに実装します。
        particle->Stop();
    }
}

void Magnet::OnAttachment()
{
    // ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
    // 例えば、初期化処理や、アタッチされたオブジェクトの状態に応じた設定などを行うことができます。
    //if (ParticleComponent* particle = GetOwner()->GetParent()->GetComponentInChildren<ParticleComponent>())
    //{
    //    // パーティクルを再生するなどの処理をここに実装します。
    //    particle->Play();
    //}

	Gadget::OnAttachment(); // 基底の処理（必要に応じて）
}

void Magnet::ClearBallSet()
{
    // 管理しているボールのセットをクリアするための処理をここに実装します。
    // 例えば、磁力の影響を受けているオブジェクトのセットをクリアするなどが考えられます。
    //affectedObjects.clear();
}

void Magnet::SetDisabled(bool disabled)
{
    Gadget::SetDisabled(disabled); // 基底の処理（フラグ設定、ClearBallSet呼び出しなど）

    if (disabled)
    {
        // 磁力イベントを完全に強制終了させる
        isActive = false;
        currentMagnetTime = 0.0f;
        currentCooldown = 0.0f;
        //affectedObjects.clear();

        if (ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
        {
            particle->Stop(); // エフェクトも止める
        }
    }
}