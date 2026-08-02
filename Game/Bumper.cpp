#include "pch.h"
#include "Bumper.h"
#include "Ball.h"
#include "Engine/Core/Math/Vector3.h"
#include "Engine/Editor/Console.h"
#include "Engine/Audio/Audio.h"
#include "Pin.h"
#include "Engine/Physics/Collider.h"

REGISTER_COMPONENT(Bumper, "Game")

void Bumper::Start()
{
	// 衝突イベントのコールバックを登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnCollisionEnterEvent([this](const CollisionInfo& collisionInfo) { OnCollisionEnter(collisionInfo); });

		//高速衝突でEnterが取れなかった場合の補完
		collider->AddOnCollisionStayEvent([this](const CollisionInfo& info) {OnCollisionStay(info); });
	}

	bounceCount = 0; // バウンド回数をリセット
	originalScale = GetTransform()->GetScale(); // 元のスケールを保存
	targetScale = originalScale * 1.5f; // バウンド時の目標スケールを設定

	//ガジェットタイプを味方ガジェットに設定
	SetGadgetType(GadgetType::AllyGadget);
}

void Bumper::Update(float deltaTime)
{
	// スケールをイージングで更新
	easingHandler.Update(currentScaleFactor, deltaTime);

	// 現在のスケールをオブジェクトに適用
	GetTransform()->SetScale(originalScale * currentScaleFactor);

	//Stayクールダウンを減らす
	if (stayCollisionCooldown > 0.0f)
		stayCollisionCooldown -= deltaTime;
}

void Bumper::OnCollisionEnter(const CollisionInfo& collisionInfo)
{
	if (IsDisabled()) return; // ガジェットが無効化されている場合は何もしない

	// null チェック
	if (collisionInfo.other == nullptr)return;
		
	// 衝突相手がBallコンポーネントを持っているか確認
	Ball* ball = collisionInfo.other->GetComponent<Ball>();
	if (ball == nullptr)return;
		
	cachedBallRigidbody = collisionInfo.other->GetComponentShared<Rigidbody>(); // ボールのRigidbodyをキャッシュ

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
	stayCollisionCooldown = 0.15f; // Stayイベントの連続発火を防止するためのクールダウンを設定
}

void Bumper::OnCollisionStay(const CollisionInfo& collisionInfo)
{
	if (stayCollisionCooldown > 0.0f)
		return;


	OnCollisionEnter(collisionInfo);
	stayCollisionCooldown = 0.15f;
}

void Bumper::OnAction()
{
	// ここにアクションを実行するためのコードを追加します。
	if (auto ballRigidbody = cachedBallRigidbody.lock())
	{
		// ボールの現在速度を取得
		Vector3 currentVel = ballRigidbody->GetVelocity();

		// 現在の速さを保持しつつ反発方向に変換（最低速度保証付き）
		float currentSpeed = currentVel.Length();
		float bounceSpeed = max(currentSpeed, 3.0f); // 最低3m/s は保証

		Vector3 bounceDirection = -cachedCollisionNormal;
		bounceDirection.y = 0;
		bounceDirection = bounceDirection.Normalize();

		// 現在速度との差分をImpulseとして与える
		Vector3 targetVelocity = bounceDirection * bounceSpeed;
		Vector3 velocityDelta = targetVelocity - currentVel;
		velocityDelta.y = 0;

		ballRigidbody->AddForce(velocityDelta * 0.02f, ForceMode::Impulse);

		// スケールアニメーションの実行
		easingHandler.Clear();
		easingHandler.AddEasing(EaseType::OutExp, currentScaleFactor, 1.1f, 0.1f);
		easingHandler.AddEasing(EaseType::OutBounce, 1.1f, currentScaleFactor, 0.55f);
		easingHandler.SetCompletedFunction([this]() { currentScaleFactor = 1.0f; });

		Audio::PlayOneShot(L"./Assets/Sounds/SE/hitBumper.wav");
	}

	// 例: ガジェットの耐久値を減らす
	DecreaseDurability();
}

void Bumper::OnAttachment()
{
	// ガジェットがオブジェクトにアタッチされたときの処理をここに追加します。
	// 例えば、アタッチされたときに特定のエフェクトを再生するなどの処理が考えられます。
	Gadget::OnAttachment(); // 基底クラスの処理も呼び出す
}
