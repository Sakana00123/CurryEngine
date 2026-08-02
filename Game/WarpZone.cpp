#include "pch.h"
#include "WarpZone.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Physics/Collider.h"
#include "Engine/Physics/Rigidbody.h"
#include "Engine/Editor/Console.h"
#include "Ball.h"
#include <random>
#include "Engine/Effects/ParticleComponent.h"
#include "Engine/Audio/Audio.h"

REGISTER_COMPONENT(WarpZone, "UserScripts")

void WarpZone::Start()
{
	// 衝突イベントの登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnTriggerEnterEvent([this](const TriggerInfo& info) { OnTriggerEnter(info); });
	}

	//ガジェットタイプを妨害ガジェットに設定
	SetGadgetType(GadgetType::ObtrusiveGadget);
	warpEffectDuration = 1.5f;
	warpEffectWaitTime = 1.5f;
}

void WarpZone::Update(float deltaTime)
{
	if (currentCooldown > 0.0f)
	{
		currentCooldown -= deltaTime;
		if (currentCooldown < 0.0f)
			currentCooldown = 0.0f;
	}

	if (!isWarping) return;

	auto rb = warpingBallRb.lock();
	if (!rb)
	{
		isWarping = false;
		return;
	}

	GameObject* ballObj = rb->GetOwner();
	effectTimer += deltaTime;
	float halfDuration = warpEffectDuration * 0.5f;
	float phase2Start = halfDuration + warpEffectWaitTime;// フェーズ1とフェーズ2の切り替えタイミングを考慮して、フェーズ2の開始を遅らせる
	float totalDuration = phase2Start + halfDuration; // フェーズ1 + 待機時間 + フェーズ2

	if (effectTimer < halfDuration)
	{
		// --- フェーズ1: ワープアウト（ゾーン中心を起点に回転しながら縮小） ---
		float t = effectTimer / halfDuration; // 0→1

		Vector3 zoneCenter = GetOwner()->GetTransform()->GetWorldPosition();

		// 回転角度（2π * 回転数 * 進行度）
		float angle = t * -2.0f * 3.14159f * 2.0f; // 1フェーズで2回転
		float radius = ballInitialScale.x * (1.0f - t); // 半径を縮小

		Vector3 newPos;
		newPos.x = zoneCenter.x + std::cos(angle) * radius;
		newPos.y = zoneCenter.y;
		newPos.z = zoneCenter.z + std::sin(angle) * radius;
		ballObj->GetTransform()->SetPosition(newPos);

		// スケール縮小（1→0）
		float scale = 1.0f - t;
		ballObj->GetTransform()->SetScale(ballInitialScale * scale);
	}
	else if (effectTimer < phase2Start)
	{
		// --- 待機フェーズ: ワープ先に瞬間移動 + 出口エフェクト再生 ---
		// 最初の1フレームだけ実行
		if ((effectTimer - deltaTime) < halfDuration)
		{
			//ワープ先を決定して瞬間移動
			// ワープ先を決定
			int randIndex = (rand() % warpDestinationCount) + 1;
			std::string targetName = warpDestinationBaseName + std::to_string(randIndex);
			GameObject* destObj = GetScene()->GetObjectManager()->Find(targetName);
			if (destObj)
			{
				warpDestPos = destObj->GetTransform()->GetWorldPosition();
				rb->SetGlobalPose(warpDestPos, ballObj->GetTransform()->GetWorldRotation());
				ballObj->GetTransform()->SetScale(Vector3::Zero);

				//出口オブジェクトにコンポーネントを追加してエフェクト再生
				ParticleComponent* particle = destObj->GetComponent<ParticleComponent>();
				if(particle == nullptr)
				{
					particle = destObj->AddComponent<ParticleComponent>();
				}

				// 入り口ではなく、出口（destObj）のParticleComponentを取得して再生
				if (ParticleComponent* particle = destObj->GetComponent<ParticleComponent>())
				{
					particle->Load("Assets/Effects/BlackHoleExitEffect.json");
					particle->Play();

					// 演出終了時に止めるために出口オブジェクトをキャッシュ
					warpDestObj = destObj; //  weak_ptr or生ポインタをキャッシュ
				}
			}
		}
		// 待機中はスケール0・位置固定のまま何もしない
	}
	else if (effectTimer < totalDuration)
	{
		// --- フェーズ2: 回転しながら拡大 ---
		float t = (effectTimer - phase2Start) / halfDuration; // 0→1

		if ((effectTimer - deltaTime) < phase2Start)
		{
			Audio::PlayOneShot(L"Assets/Sounds/SE/exitWarp.wav", 0.5f);
		}

		float angle = t * -2.0f * 3.14159f * 2.0f;
		float radius = ballInitialScale.x * t;

		Vector3 newPos;
		newPos.x = warpDestPos.x + std::cos(angle) * radius;
		newPos.y = warpDestPos.y;
		newPos.z = warpDestPos.z + std::sin(angle) * radius;
		ballObj->GetTransform()->SetPosition(newPos);

		float scale = t;
		ballObj->GetTransform()->SetScale(ballInitialScale * scale);
	}
	else
	{
		// 出口エフェクトを止める（出口オブジェクトのパーティクル）
		if (warpDestObj != nullptr)
		{
			if (ParticleComponent* particle = warpDestObj->GetComponentInChildren<ParticleComponent>())
			{
				particle->Stop();
			}
			warpDestObj = nullptr;
		}

		ballObj->GetTransform()->SetScale(ballInitialScale);
		rb->SetGlobalPose(warpDestPos, ballObj->GetTransform()->GetWorldRotation());
		rb->SetKinematic(false);

		float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
		Vector3 randomDir;
		randomDir.x = std::cos(angle);
		randomDir.y = 0.0f;
		randomDir.z = std::sin(angle);
		randomDir = randomDir.Normalize();
		rb->SetVelocity(Vector3::Zero);
		rb->AddForce(randomDir * 1.0f, ForceMode::VelocityChange);


		// 耐久値を減らす
		DecreaseDurability();

		isWarping = false;
		Console::Log("WarpZone: ワープ演出完了");
	}
}

void WarpZone::OnTriggerEnter(const TriggerInfo& info)
{
	if (currentCooldown > 0.0f)
		return; // クールダウン中はワープしない
	if (isWarping) return;

	if (info.other == nullptr) return;

	// 衝突したオブジェクトがボールかどうかを確認
	if (info.other->GetComponent<Ball>() == nullptr) return;

	// ボールのRigidbodyを保持してアクション実行へ投げる
	cachedBallRigidbody = info.other->GetComponentShared<Rigidbody>();

	Audio::PlayOneShot(L"Assets/Sounds/SE/enterWarp.wav", 0.5f);

	PerformAction();

}

void WarpZone::OnAction()
{
	if (warpDestinationCount <= 0)
	{
		Console::Log("WarpZone: ワープ先の数が0以下に設定されています。");
		return;
	}

	auto rb = cachedBallRigidbody.lock();
	if (!rb) return;

	// ワープ先を決定
	int randIndex = (rand() % warpDestinationCount) + 1;
	std::string targetName = warpDestinationBaseName + std::to_string(randIndex);
	GameObject* destObj = GetScene()->GetObjectManager()->Find(targetName);
	if (destObj == nullptr)
	{
		Console::Log("WarpZone: ワープ先 " + targetName + " が見つかりませんでした。");
		return;
	}

	warpDestPos = destObj->GetTransform()->GetWorldPosition();
	warpingBallRb = cachedBallRigidbody;

	// 演出中は物理を止める
	rb->SetVelocity(Vector3::Zero);
	rb->SetAngularVelocity(Vector3::Zero);
	rb->SetKinematic(true);

	// 演出開始
	ballInitialScale = rb->GetOwner()->GetTransform()->GetScale();
	effectTimer = 0.0f;
	isWarping = true;

	currentCooldown = warpCooldownTime;
	Console::Log("WarpZone: " + targetName + " へワープ開始");

}

void WarpZone::OnRoundEnd()
{
	currentCooldown = 0.0f; // ラウンド終了時にクールダウンをリセット
}

void WarpZone::OnActivate()
{
	if(ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	{
		particle->Play();
	}
}

void WarpZone::OnDeactivate()
{
	// 演出中だった場合は強制終了
	if (isWarping)
	{
		if (auto rb = warpingBallRb.lock())
		{
			rb->GetOwner()->GetTransform()->SetScale(ballInitialScale);
			rb->SetKinematic(false);
		}
		// 出口エフェクトも止める
		if (warpDestObj != nullptr)
		{
			if (ParticleComponent* particle = warpDestObj->GetComponent<ParticleComponent>())
			{
				particle->Stop();
			}
			warpDestObj = nullptr;
		}
		isWarping = false;
	}

	if(ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	{
		particle->Stop();
	}
}

void WarpZone::OnAttachment()
{
	// ワープゾーンは常にアクティブ状態で存在するため、特に何もしない
	Gadget::OnAttachment(); // 基底クラスの処理を呼び出す

	if (ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>())
	{
		particle->Play();
	}
}