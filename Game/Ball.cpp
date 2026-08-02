#include "pch.h"
#include "Ball.h"
#include "Pin.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Editor/Console.h"
#include "Engine/Physics/SphereCollider.h"
#include "Bumper.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Audio/AudioSource.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Core/Time.h"
#include "Engine/UI/Text.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Physics/Rigidbody.h"
#include "Gate.h"
#include "BallRespawnTimer.h"
#include "Flipper.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Factory/GameObjectFactory.h"
#include "Engine/Easing/EasingComponent.h"
#include "PassiveSkillContainer.h"
#include "ComboText.h"
#include "RoundManager.h"
#include "AchievementManager.h"
#include "Engine/Effects/ParticleComponent.h"

REGISTER_COMPONENT(Ball, "Ball")

void Ball::Start()
{
	value = 1; // ボールの価値を初期化

	const auto& containers = GetScene()->FindComponents<PassiveSkillContainer>();
	if (!containers.empty())
	{
		PassiveSkillContainer* passiveSkillContainer = containers.front();
		if (passiveSkillContainer)
		{
			// パッシブスキルコンテナから "InitBallValue" プロパティの修正値を取得して、ボールの価値に加算します。
			value += static_cast<int>(passiveSkillContainer->GetModifier("InitBallValue")); // "InitBallValue" は仮のプロパティ名。実際のプロパティ名に合わせて変更してください。

			// パッシブスキルコンテナから "BallScaling" プロパティの修正値を取得して、ボールのサイズを変更します。
			float scalingModifier = 1.0f + passiveSkillContainer->GetModifier("BallScaling"); // "BallScaling" は仮のプロパティ名。実際のプロパティ名に合わせて変更してください。
			if (scalingModifier < minScaleFactor) scalingModifier = minScaleFactor; // サイズが極端に小さくなるのを防止
			ScaleBallRelative(scalingModifier); // 初期スケールに基づいてサイズを変更
		}
	}

	// 衝突イベントのコールバックを登録
	if (SphereCollider* collider = GetOwner()->GetComponent<SphereCollider>())
	{
		collider->AddOnCollisionEnterEvent([this](const CollisionInfo& collisionInfo) { OnCollisionEnter(collisionInfo); });
		/*collider->AddOnCollisionStayEvent([this](const CollisionInfo& collisionInfo) { OnCollisionStay(collisionInfo); });
		collider->AddOnCollisionExitEvent([this](const CollisionInfo& collisionInfo) { OnCollisionExit(collisionInfo); });*/
	}

	//initialPosition = GetTransform()->GetPosition(); // 初期位置を保存
	isPlaySound = false; // 音の再生フラグを初期化

	isStacking = false; // ボールが静止して詰んでる状態かどうかのフラグを初期化
	stackingTimer = 0.0f; // ボールが静止して詰んでる状態のタイマーを初期化

	gate = GetScene()->GetObjectManager()->Find("Gate_Pivot")->GetComponent<Gate>();
}

void Ball::Update(float deltaTime)
{
	// Rキーで初期位置にリセット
	if (InputSystem::GetKeyTrigger('R'))
	{
		ResetToInitialPosition();
	}

	Vector3 position = GetTransform()->GetWorldPosition();
	bool isOutOfBounds = position.LengthSq() > 100.0f; // 例えば、座標の長さの二乗が100を超えたらアウトオブバウンズとする

	if (isOutOfBounds)
	{
		// デバッガーアチーブメント
		AchievementManager::AddProgressToManager(GetScene(), "DEBUGGER", 1);

		// 自身を破壊する
		GetOwner()->Destroy();
	}

#if 1

	float lengthSq = (position - previousPosition).LengthSq();

	previousPosition = position;

	// ボールがほとんど動いていない（スタッキング状態）かどうかをチェック
	Rigidbody* rb = GetOwner()->GetComponent<Rigidbody>();
	if (!rb) return; // Rigidbodyがない場合は処理しない

	if (gate)
	{
		if (!gate->IsClosed())
		{
			return; // ゲートが開いている場合はスタッキングチェックを行わない
		}
	}

	if (rb->IsSleeping()/* && lengthSq < 0.004f*/) // 速度の二乗が0.04未満ならほとんど動いていないとみなす
	{
		if (!isStacking)
		{
			isStacking = true; // スタッキング状態に入る
			stackingTimer = 0.0f; // タイマーをリセット
		}
		else
		{
			Console::Log("Ball is stacking. Timer: " + std::to_string(stackingTimer));
			stackingTimer += deltaTime; // スタッキング状態の時間を更新
			if (stackingTimer >= stackingThreshold)
			{
				if (GameObject* shakeButtonObj = GetScene()->GetObjectManager()->Find("ShakeButton"))
				{
					shakeButtonObj->SetActive(true); // シェイクボタンを表示
					Console::Log("Ball has been stacking for " + std::to_string(stackingTimer) + " seconds. Showing shake button.");
				}
			}
		}
	}
	else
	{
		isStacking = false; // 動いているのでスタッキング状態をリセット
		stackingTimer = 0.0f; // タイマーもリセット
	}
#endif // 0

}

//void Ball::DrawProperty()
//{
//#ifdef USE_IMGUI
//	Component::DrawProperty(); // 基底クラスの描画を呼び出す 
//
//	if (ImGui::Button("Shake"))
//	{
//		if (Rigidbody* rb = GetOwner()->GetComponent<Rigidbody>())
//		{
//			Vector3 randomForce = Vector3(
//				((rand() / (float)RAND_MAX) - 0.5f) * 2.0f, // -1.0から1.0のランダムな値
//				0.0f,
//				((rand() / (float)RAND_MAX) - 0.5f) * 2.0f
//			).Normalize() * 0.01f; // ランダムな方向に小さな力を与える
//			rb->AddForce(randomForce, ForceMode::Impulse);
//			Console::Log("Applying shake force to ball: " + std::to_string(randomForce.x) + ", " + std::to_string(randomForce.y) + ", " + std::to_string(randomForce.z));
//		}
//	}
//
//#endif // USE_IMGUI
//
//}

void Ball::OnDestroy()
{
	// ボールが破壊されたときの処理
	//エフェクトを停止
	ParticleComponent* particle = GetOwner()->GetComponent<ParticleComponent>();
	if (particle)
	{
		particle->Stop();
	}
	
}

void Ball::OnCollisionEnter(const CollisionInfo& collisionInfo)
{
	if (collisionInfo.other == nullptr) return; // 衝突相手が存在しない場合は処理しない
	GoldenPin* goldenPin = collisionInfo.other->GetComponent<GoldenPin>();
	Pin* pin = collisionInfo.other->GetComponent<Pin>();

	//ピンに当たったら耐久値を減少させる


	if (goldenPin)
	{
		value += 5;
		durability--; // ゴールデンピンに当たったら耐久値を減少
		Audio::PlayOneShot(L"./Assets/Sounds/SE/hitPin.wav");
		Console::Log("Golden Pin hit! Value is now " + std::to_string(value));
	}
	else if (pin)
	{
		IncreaseValue(1); // ピンに当たったら価値を増加
		durability--; // ピンに当たったら耐久値を減少

		// TODO: コンボの加算がピンにヒットなら、ここに処理を追加する。
		RoundManager* roundManager = GetScene()->GetObjectManager()->Find("RoundManager")->GetComponent<RoundManager>();
		if (roundManager)
		{
			ComboText* comboText = GetScene()->FindComponentById<ComboText>(roundManager->comboTextReference);
			if (comboText)
			{
				comboText->AddComboCount(1); // コンボ数を1増加
			}
		}
		
#if 1
		if (Rigidbody* rb = GetOwner()->GetComponent<Rigidbody>())
		{
			if (collisionInfo.contacts.size() > 0)
			{
				if (collisionInfo.contacts[0].normal.x != 0.0f)
				{
					// 横方向(X軸)に、衝突法線のX成分を正規化してインパルスを与える
					Vector3 impulseDir = Vector3(collisionInfo.contacts[0].normal.x, 0, 0).Normalize(); // 衝突法線のX成分を正規化してベクトルを生成
					float stackingImpulse = 0.0001f; // インパルスの強さ
					rb->AddForce(impulseDir * stackingImpulse, ForceMode::Impulse); // インパルスを与える
					Console::Log("Applying impulse to ball: " + std::to_string(impulseDir.x * stackingImpulse));
				}
			}
		}
#endif // 0


		Audio::PlayOneShot(L"./Assets/Sounds/SE/hitPin.wav");
		Audio::PlayOneShot(L"./Assets/Sounds/SE/IncreaseValue.wav");
		Console::Log("Ball value increased to " + std::to_string(value));
	}

	bool isHitWall = false;

	if (collisionInfo.other->GetName().find("pinball_frame") != std::string::npos)
	{
		Audio::PlayOneShot(L"./Assets/Sounds/SE/hitWall.wav");
		isHitWall = true;
	}

	//フリッパーに当たったらhitWallを再生
	if (collisionInfo.other->GetComponent<Flipper>() != nullptr)
	{
		Audio::PlayOneShot(L"./Assets/Sounds/SE/hitWall.wav");
	}
	if(collisionInfo.other->GetName().find("Stopper") != std::string::npos)
	{
		Audio::PlayOneShot(L"./Assets/Sounds/SE/hitWall.wav");
	}

	// 「壁に当たったら価値が下がる」状態(フラグ)がONなら減る
	if (isHitWall && isCurseByWall)
	{
		IncreaseValue(-1);
		Audio::PlayOneShot(L"./Assets/Sounds/SE/decreaseValue.wav");
		Console::Log("Hit wall! Value decreased.");
	}
}

void Ball::OnCollisionStay(const CollisionInfo& collisionInfo)
{
	// ピンに当たっている間
	if (collisionInfo.other->GetComponent<Pin>())
	{
		//collisionTimer += Time::DeltaTime(); // 衝突時間を更新

		//ピンに当たっている時間が長かったらタイムスケールを早くする
		//if (collisionTimer >= 3.0f)
		//{
		//	// Console::Log("Collision timer: " + std::to_string(collisionTimer)); // 頻繁に出るためコメント化推奨
		//	Time::timeScale = 4.0f; // タイムスケールを4倍にする
		//}
	}
}

void Ball::OnCollisionExit(const CollisionInfo& collisionInfo)
{
	// ピンから離れた時
	if (collisionInfo.other->GetComponent<Pin>())
	{
		// タイムスケールを元に戻し、タイマーをリセット
		Time::timeScale = 1.0f;
		//collisionTimer = 0.0f;
	}
}

void Ball::IncreaseValue(int amount)
{
	value += amount;

	// 価値が増加したとき、ボールのスクリーン座標にテキストを表示する処理を追加
	// TODO: 価値が減少したときの表示も追加する場合は、amountの正負で表示内容や色を変えるなどの工夫が必要。
	// プレハブにテキストオブジェクトを用意しておいて、そこに表示する方法もあるが、今回は動的に生成して表示する方法で実装してみる。
	{
		// テキストオブジェクトを生成して表示
		{
			GameObject* valueTextObj = GameObjectFactory::CreateText(SceneManager::GetLoadingSceneOrCurrentScene(), "ValueText");
			Text* valueText = valueTextObj->GetComponent<Text>();
			if (valueText)
			{
				std::wstring displayText = amount > 0 ? L"+" + std::to_wstring(amount) : std::to_wstring(amount);
				valueText->SetText(displayText); // 増加量を表示
				Color textColor = amount > 0 ? Color::Yellow : Color::Red; // 増加なら黄色、減少なら赤色
				valueText->SetColor(textColor); // 色を設定
				valueText->SetFontSize(64); // フォントサイズを設定
				valueText->SetHorizontalOverflow(Text::HorizontalOverflow::Overflow); // はみ出しを許可
				valueText->SetAlignment(Text::Alignment::MiddleCenter); // 中央揃え

				// ボールのスクリーン座標を取得してテキストの位置を設定
				valueText->GetRectTransform()->SetAnchoredPositionByTransform(GetTransform()); // ボールの上に表示
				valueText->GetRectTransform()->Update(0.0f); // 位置を最新に更新
				float startY = valueText->GetRectTransform()->GetAnchoredPosition().y;
				float endY = startY - 50.0f; // 上に50ピクセル移動する目標位置

				EasingComponent* easing = valueTextObj->AddComponent<EasingComponent>();
				// 上に移動しながらフェードアウトするイージングを追加
				EasingHandler moveUpHandler;
				moveUpHandler.AddEasing(EaseType::InCubic, startY, endY, 1.0f); // 1秒かけて上に移動
				PropertyAccessor<float> posYAccessor{
					.getter = [valueText]() { return valueText->GetRectTransform()->GetAnchoredPosition().y; },
					.setter = [valueText](float y) { Vector2 pos = valueText->GetRectTransform()->GetAnchoredPosition(); pos.y = y; valueText->GetRectTransform()->SetAnchoredPosition(pos); }
				};
				easing->StartHandler(moveUpHandler, posYAccessor);

				EasingHandler fadeOutHandler;
				fadeOutHandler.AddEasing(EaseType::Linear, 1.0f, 0.0f, 1.0f); // 1秒かけてフェードアウト
				PropertyAccessor<float> alphaAccessor{
					.getter = [valueText]() { return valueText->GetColor().a; },
					.setter = [valueText](float a) { Color color = valueText->GetColor(); color.a = a; valueText->SetColor(color); }
				};
				easing->StartHandler(fadeOutHandler, alphaAccessor);

				// 一定時間後にテキストオブジェクトを破棄する処理を追加
				valueTextObj->Destroy(1.0f); // 1秒後に破棄
			}
		}

	}
}

void Ball::ResetToInitialPosition()
{
	//GetTransform()->SetPosition(initialPosition);
	value = 1;  // 価値もリセット
	isCurseByWall = false; // 壁に当たったフラグをリセット
	Rigidbody* rigidbody = GetOwner()->GetComponent<Rigidbody>();
	if (rigidbody)
	{
		rigidbody->SetVelocity(Vector3::Zero); // ボールの速度もリセット
		rigidbody->SetAngularVelocity(Vector3::Zero); // ボールの回転もリセット
	}

	Gate* gate = GetScene()->GetObjectManager()->Find("Gate_Pivot")->GetComponent<Gate>();
	if (gate)
	{
		gate->OpenGate(); // ゲートを開く処理を呼び出す
	}
	ResetDurability(); // 耐久値をリセット
}

void Ball::ScaleBall(float scaleFactor)
{
	if (scaleFactor < minScaleFactor)
	{
		Console::LogError("Invalid scale factor provided to ScaleBall. Scale factor must be greater than 0. Provided value: " + std::to_string(scaleFactor));
		return; // 無効なスケールファクターの場合は処理を中断する
	}
	if (scaleFactor > maxScaleFactor)
	{
		Console::LogError("Invalid scale factor provided to ScaleBall. Scale factor must be less than or equal to " + std::to_string(maxScaleFactor) + ". Provided value: " + std::to_string(scaleFactor));
		return; // 無効なスケールファクターの場合は処理を中断する
	}
	GetTransform()->SetScale(initialScale * scaleFactor);
	UpdateBallPhysicsMaterialData();
}

void Ball::ScaleBallRelative(float scaleFactor)
{
	float currentScaleRatio = GetRelativeScale();
	float newScale = GetTransform()->GetScale().x * scaleFactor; // 現在のスケールに scaleFactor を掛けた新しいスケールを計算
	float newScaleRatio = newScale / initialScale; // 現在のスケール比にさらに scaleFactor を掛けた新しいスケール比を計算
	if (newScaleRatio < minScaleFactor)
	{
		Console::LogError("Invalid scale factor provided to ScaleBallRelative. Resulting scale ratio must be greater than 0. Provided value: " + std::to_string(scaleFactor) + ", resulting scale ratio: " + std::to_string(newScaleRatio));
		return; // 無効なスケールファクターの場合は処理を中断する
	}
	if (newScaleRatio > maxScaleFactor)
	{
		Console::LogError("Invalid scale factor provided to ScaleBallRelative. Resulting scale ratio must be less than or equal to " + std::to_string(maxScaleFactor) + ". Provided value: " + std::to_string(scaleFactor) + ", resulting scale ratio: " + std::to_string(newScaleRatio));
		return; // 無効なスケールファクターの場合は処理を中断する
	}
	GetTransform()->SetScale(newScale);
	UpdateBallPhysicsMaterialData();
}

float Ball::GetRelativeScale() const
{
	float scaleRatio = GetTransform()->GetScale().x / initialScale; // x軸のスケール比を使用（球体なので他の軸も同じはず）
	if (!std::isfinite(scaleRatio) || scaleRatio <= 0.0f)
	{
		Console::LogError("Invalid scale ratio calculated for Ball. Scale ratio: " + std::to_string(scaleRatio));
		return minScaleFactor; // 無効なスケール比の場合は最小値を返すなどの対処をする
	}
	return scaleRatio;
}

void Ball::UpdateBallPhysicsMaterialData()
{
	// 初期スケールから現在のスケールへの比率を計算
	float scaleRatio = GetRelativeScale(); // 現在のスケール比を取得

	// スケール比に基づいて質量を調整する（スケールが大きくなるほど重くなるように）
	if (Rigidbody* rigidbody = GetOwner()->GetComponent<Rigidbody>())
    {
        // 毎回 initialMass を基準に計算する（複利防止）
        float newMass = initialMass * scaleRatio * scaleRatio * scaleRatio;
        
        // 半径もスケールに追従
		float newRadius = initialScale * scaleRatio;
        
        // InertiaTensorも同時更新 (実球: I = 2/5 * m * r^2)
        float I = 0.4f * newMass * newRadius * newRadius;

		newMass = max(newMass, 1e-4f); // 質量が極端に小さくなるのを防止
		I = max(I, 1e-8f); // 慣性モーメントが極端に小さくなるのを防止

        rigidbody->SetMass(newMass);
        rigidbody->SetInertiaTensor(Vector3(I, I, I));
    }
}
