#include "pch.h"
#include "GravityChange.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Physics/Rigidbody.h"
#include "Ball.h"
#include "Engine/Physics/Physics.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Effects/ParticleComponent.h"
#include "Engine/Physics/Collider.h"


// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(GravityChange, "Gravity")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(GravityChange, "UserScripts", ComponentAttributes::None, {})

void GravityChange::Start()
{
    // コンポーネントが開始されたときの処理をここに実装します。
    // 衝突イベントの登録
    if (Collider* collider = GetOwner()->GetComponent<Collider>())
    {
        collider->AddOnTriggerEnterEvent([this](const TriggerInfo& info) { OnTriggerEnter(info); });
    }

    // 元の重力を保存
    originalGravity = Physics::GetGravity();

    // 重力変化のタイマーをリセット
    gravityChangeTimer = 0.0f;

    // このオブジェクトに関連付けられたエフェクトをキャッシュ
    particleEffect = GetOwner()->GetComponent<ParticleComponent>();

	//ガジェットのタイプを味方ガジェットに設定
	SetGadgetType(GadgetType::AllyGadget);
}

void GravityChange::Update(float deltaTime)
{
	if (IsDisabled()) return; // ガジェットが無効化されている場合は何もしない


    // 毎フレームの更新処理をここに実装します。
    Vector3 customGravity(0.0f, 0.0f, originalGravity.y * -1.0f);

    for (auto it = modifiedBalls.begin(); it != modifiedBalls.end();)
    {
        it->timer += deltaTime;
        if (it->timer >= gravityChangeDuration)
        {
            // 重力を元に戻す
            if (!it->rb.expired())
            {
                it->rb.lock()->SetUseGravity(true);
            }
            it = modifiedBalls.erase(it); // リストから削除
        }
        else
        {
            if (!it->rb.expired())
            {
                it->rb.lock()->AddForce(customGravity, ForceMode::Acceleration); // カスタム重力を適用
            }
            ++it;
        }
    }

    if (GetDurability() <= 0)
    {
        Deactivate(); // 耐久値が0以下になったら壊れる処理を呼び出す
    }
}

void GravityChange::OnTriggerEnter(const TriggerInfo& triggerInfo)
{
    if (IsDisabled()) return; // ガジェットが無効化されている場合は何もしない

    if (triggerInfo.other == nullptr) return;

    // トリガーしたオブジェクトがボールかどうかを確認
    Ball* ball = triggerInfo.other->GetComponent<Ball>();
    if (ball == nullptr) return;

    auto rb = triggerInfo.other->GetComponentShared<Rigidbody>();
    if (!rb) return;

    // 既に変更中のリストに入っているかチェック
    bool isAlreadyModified = false;
    for (auto& modifiedBall : modifiedBalls)
    {
        if (modifiedBall.rb.lock() == rb)
        {
            modifiedBall.timer = 0.0f; // タイマーをリセット
            isAlreadyModified = true;
            break;
        }
    }

    // リストになければ追加して、本来の重力を無効化
    if (!isAlreadyModified)
    {
        rb->SetUseGravity(false);
        modifiedBalls.push_back({ rb, 0.0f });
    }

    // アクションを実行（耐久値の減少など）
    DecreaseDurability();
    Audio::PlayOneShot(L"./Assets/Sounds/SE/gravityChange.wav");
}

void GravityChange::OnBreak()
{
    // ガジェットが壊れたときの処理をここに実装します。
    ResetGravity(); // ガジェットが壊れたときに重力を元に戻す

    // このオブジェクトのエフェクトを停止
    if (particleEffect)
    {
        particleEffect->Stop();
    }

    Gadget::OnBreak(); // ガジェットを非アクティブ化
}

void GravityChange::OnDeactivate()
{
    // ガジェットが非アクティブ化されたときの処理をここに実装します。
    ResetGravity(); // ガジェットが非アクティブ化されたときに重力を元に戻す

    // このオブジェクトのエフェクトを停止
    /*if (particleEffect)
    {
        particleEffect->Stop();
    }*/

    Gadget::OnDeactivate(); // ガジェットを非アクティブ化
}

void GravityChange::OnActivate()
{
    // ガジェットがアクティブ化されたときの処理をここに実装します。
    
}

void GravityChange::OnPreviewEnter()
{
    // ピンに近づけたときのプレビュー表示開始の処理をここに実装します。
	if (particleEffect = GetOwner()->GetComponent<ParticleComponent>()) // エフェクトコンポーネントが存在する場合
    {
        // パーティクルを再生するなどの処理をここに実装します。
        particleEffect->Play();
    }
}

void GravityChange::OnPreviewExit()
{
    // ピンから離れたときのプレビュー表示終了の処理をここに実装します。
    if (particleEffect)
    {
        // パーティクルを停止するなどの処理をここに実装します。
        particleEffect->Stop();
    }
}

void GravityChange::OnAttachment()
{
    // ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
    // 例えば、ガジェットがアタッチされたオブジェクトの位置にエフェクトを配置するなどの処理をここに実装します。
	Gadget::OnAttachment(); // 基底クラスの OnAttachment を呼び出す
}

void GravityChange::ResetGravity()
{
    // 重力を元に戻す処理をここに実装します。
    for (auto& modifiedBall : modifiedBalls)
    {
        if (!modifiedBall.rb.expired())
        {
            modifiedBall.rb.lock()->SetUseGravity(true);
        }
    }
    modifiedBalls.clear(); // 変更されたボールのリストをクリア
}

void GravityChange::ClearBallSet()
{
    // ガジェットが管理しているボールのセットをクリアするための関数をここに実装します。
    modifiedBalls.clear();
}

void GravityChange::SetDisabled(bool disabled)
{
    Gadget::SetDisabled(disabled); // 基底クラスの SetDisabled を呼び出す
    if (disabled)
    {
        ResetGravity(); // ガジェットが無効化されたときに重力を元に戻す
        // このオブジェクトのエフェクトを停止
        /*if (particleEffect)
        {
            particleEffect->Stop();
        }*/
    }
}
