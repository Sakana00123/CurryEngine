#include "pch.h"
#include "BallSplitter.h"
#include "Ball.h"
#include "Engine/Physics/Collider.h"
#include "Engine/Physics/RigidBody.h"
#include "Pin.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Effects/ParticleComponent.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(BallSplitter, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(BallSplitter, "UserScripts", ComponentAttributes::None, {})


void BallSplitter::Start()
{
    // コンポーネントが開始されたときの処理をここに実装します。

    // 衝突イベントのコールバックを登録
    if (Collider* collider = GetOwner()->GetComponent<Collider>())
    {
        collider->AddOnCollisionEnterEvent([this](const CollisionInfo& collisionInfo) { OnCollisionEnter(collisionInfo); });
    }

	// ガジェットのタイプを味方ガジェットに設定
	SetGadgetType(GadgetType::AllyGadget);

}

void BallSplitter::Update(float deltaTime)
{
    // 毎フレームの更新処理をここに実装します。

    if(splitCooldown > 0.0f)
    {
        splitCooldown -= deltaTime;
	}

    //少しホバーさせながらモデルを回転させる
	GetOwner()->GetTransform()->Rotate(Vector3(0, 180 * deltaTime, 0)); // Y軸を中心に回転

}

void BallSplitter::OnCollisionEnter(const CollisionInfo& info)
{
    if (IsDisabled()) return; // ガジェットが無効化されている場合は何もしない

    if (info.other == nullptr)
        return;

    // ボール以外の衝突では複製しない
	Ball* ballComp = info.other->GetComponent<Ball>();
    if (ballComp == nullptr)
        return;
	float relativeScale = ballComp->GetRelativeScale();
	if (ballComp->GetRelativeScale() <= Ball::minScaleFactor)
		return; // あまりに小さいボールは分裂させない（既存処理）

	// 衝突したボールの相対スケールをキャッシュしておく（OnActionで使用）
	relativeScaleCache = relativeScale;

	valueCache = ballComp->GetValue(); // ボールの価値をキャッシュしておく（OnActionで使用）

	// 座標をキャッシュしておく（OnActionで使用）
    spawnPosCache = info.other->GetTransform()->GetPosition();

	// Rigidbodyをキャッシュしておく（OnActionで使用）
	ballRbCache = info.other->GetComponentShared<Rigidbody>();

    // アクションを実行して分裂処理を行う
	PerformAction();
}


void BallSplitter::OnAction()
{
    // ガジェットがアクションを実行したときの処理をここに実装します。

    //クールダウン中は無視
    if (splitCooldown > 0.0f)
        return;

    splitCooldown = splitCooldownTime;  //クールダウン開始

    // 当たってきたボールの位置に新しいボールを複製
	Vector3 spawnPos = spawnPosCache; // Rigidbodyから正確な位置を取得
    GameObject* newBall = Instantiate(ballPrefabPath, spawnPos);

#if 1
    // 複製されたボールに元のボールの速度を引き継ぐ
    if (newBall != nullptr)
    {
        // 念のためTransformに直接位置をセットする
        //newBall->GetTransform()->SetPosition(spawnPos);
#if 1
        auto ballRbShared = ballRbCache.lock();
        if (ballRbShared == nullptr) return;
        Rigidbody* ballRb = ballRbShared.get();
        Rigidbody* newRb = newBall->GetComponent<Rigidbody>();
        if (newRb != nullptr && ballRb != nullptr)
        {
            // Rigidbody側に直接座標を送り込んで物理演算の位置を即座に同期させる
            newRb->SetGlobalPose(spawnPos, newBall->GetTransform()->GetRotation());

            // 新しいボールのスケールを元のボールの90%に縮小する倍率
            float scaleReductionRate = max(Ball::minScaleFactor, relativeScaleCache - 0.2f);
            
            int newValue = static_cast<int>(valueCache * 0.5f); // 新しいボールの価値を元のボールの半分にする

            if (Ball* newBallComp = newBall->GetComponent<Ball>())
            {
				newBallComp->ScaleBall(scaleReductionRate);
				newBallComp->SetValue(newValue);
            }

            //元のボールも小さくする
            if (GameObject* originalBallObj = ballRb->GetOwner()) 
            {
                if (Ball* originalBall = originalBallObj->GetComponent<Ball>())
                {
					originalBall->ScaleBall(scaleReductionRate); // 元のボールも新しいボールと同じ倍率で縮小する
					originalBall->SetValue(newValue); // 元のボールの価値も新しいボールと同じにする
                }
            }
        }
#endif // 0

    }
#endif // 0

	// 耐久値を減らす
    DecreaseDurability();

    //ランダムでどちらかの音を流す
    if (rand() % 2 == 0)
    {
        Audio::PlayOneShot(L"./Assets/Sounds/SE/ballSplit.wav");
    }
    else
    {
        Audio::PlayOneShot(L"./Assets/Sounds/SE/ballSplit2.wav");
    }
}

void BallSplitter::OnAttachment()
{
    // ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
    if(ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
    {
        particle->Play(); // 再生
	}

	Gadget::OnAttachment(); // 基底クラスの処理も呼び出す
}

void BallSplitter::OnBreak()
{
    // ガジェットが壊れたときの処理をここに実装します。
    if(ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
    {
        particle->Stop(); // 停止
    }

	Gadget::OnBreak(); // 基底クラスの処理も呼び出す
}

void BallSplitter::OnPreviewEnter()
{
    // ピンに近づけたときの処理をここに実装します。
    if (ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
    {
        particle->Play();
    }
}

void BallSplitter::OnPreviewExit()
{
    // ピンから離れたときの処理をここに実装します。
    if (ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
    {
        particle->Stop();
    }
}