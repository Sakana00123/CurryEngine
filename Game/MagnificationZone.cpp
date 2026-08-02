#include "pch.h"
#include "MagnificationZone.h"
#include "Bumper.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Editor/Console.h"
#include "Engine/Physics/Collider.h"
#include "PreserveValue.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Scenes/SceneManager.h"
#include "RoundManager.h"
#include "TestAttachment.h"
#include "PhaseManager.h"
#include "AttachManager.h"

#include "Engine/Scenes/Scene.h"
#include "Engine/Factory/GameObjectFactory.h"
#include "Engine/Easing/EasingComponent.h"
#include <Engine/UI/Text.h>
#include <Engine\Effects\ParticleComponent.h>


REGISTER_COMPONENT(MagnificationZone, "Game")

void MagnificationZone::Start()
{
	// トリガーイベントのコールバックを登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnTriggerEnterEvent([this](const TriggerInfo& triggerInfo) { OnTriggerEnter(triggerInfo); });
	}

	//collisionCount = 0;
	//currentBall = nullptr;
	resetTimer = 0.0f;
	isWaitingForReset = false;

}

void MagnificationZone::Update(float deltaTime)
{
}

void MagnificationZone::OnDestroy()
{
	currentBall = nullptr;
}

void MagnificationZone::OnTriggerEnter(const TriggerInfo& triggerInfo)
{
	// null チェック
	if (triggerInfo.other == nullptr)
		return;

	// 衝突相手がBallコンポーネントを持っているか確認
	Ball* ball = triggerInfo.other->GetComponent<Ball>();
	if (ball == nullptr)
		return;

	// 倍率を適用
	ball->MultiplicationValue(static_cast<int>(magnification));

	// 通過直後にtotalValueを更新してUIに反映
	PreserveValue* preserveValue = GetScene()->objectManager->Find("PreserveValue")->GetComponent<PreserveValue>();
	if (preserveValue)
	{
		preserveValue->SaveTotalValue(ball->GetValue()); // 即座にtotalValueに加算
		preserveValue->UpdateUIText();
	}

	// 倍率ゾーンにはいったら効果音再生
	Audio::PlayOneShot(L"./Assets/Sounds/SE/enteredMultiplier.wav", 0.5f);

	{
		// 値の変化を表示するエフェクトを追加する例
		// TODO: 価値が減少したときの表示も追加する場合は、amountの正負で表示内容や色を変えるなどの工夫が必要。
		// プレハブにテキストオブジェクトを用意しておいて、そこに表示する方法もあるが、今回は動的に生成して表示する方法で実装してみる。
		int amount = ball->GetValue();
		{
			// テキストオブジェクトを生成して表示
			{
				GameObject* valueTextObj = GameObjectFactory::CreateText(SceneManager::GetLoadingSceneOrCurrentScene(), "ValueText");
				Text* valueText = valueTextObj->GetComponent<Text>();
				if (valueText)
				{
					std::wstring displayText = amount > 0 ? L"+" + std::to_wstring(amount) : std::to_wstring(amount);
					valueText->SetText(displayText); // 増加量を表示
					Color textColor = amount > 0 ? Color::Orange : Color::Red; // 増加ならオレンジ、減少なら赤色
					valueText->SetColor(textColor); // 色を設定
					valueText->SetFontSize(72); // フォントサイズを設定
					valueText->SetHorizontalOverflow(Text::HorizontalOverflow::Overflow); // はみ出しを許可
					valueText->SetAlignment(Text::Alignment::MiddleCenter); // 中央揃え
					

					// ボールのスクリーン座標を取得してテキストの位置を設定
					valueText->GetRectTransform()->SetAnchoredPositionByTransform(GetTransform()); // ボールの上に表示
					valueText->GetRectTransform()->Update(0.0f); // 位置を最新に更新
					float startY = valueText->GetRectTransform()->GetAnchoredPosition().y;
					float endY = startY - 50.0f; // 上に50ピクセル移動する目標位置

					EasingComponent* easing = valueTextObj->AddComponent<EasingComponent>();
					// イージングハンドラーを作成して、テキストのY座標を上に移動させるアニメーションと、同時にフェードアウトするアニメーションを設定
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


			// エフェクトプレハブを生成して表示
			GameObject* effectObj = Instantiate("./Assets/Prefabs/CoinEffect.prefab", ball->GetTransform()->GetPosition());
			if (auto* particleComponent = effectObj->GetComponent<ParticleComponent>())
			{
				particleComponent->Play();
				// エフェクトオブジェクトを一定時間後に破棄する処理を追加
				effectObj->Destroy(2.0f); // 2秒後に破棄
			}
		}
	}



	// ログ出力
	Console::Log("Ball value multiplied by magnification zone to " + std::to_string(ball->GetValue()));
	Console::Log("Ball entered magnification zone! Lifespan count: " + std::to_string(collisionCount));

	// ボールを削除
	ball->GetOwner()->Destroy();

}

void MagnificationZone::SetMagnification(float newMagnification, bool isTopRate)
{
	magnification = newMagnification;

	if (Text* multiplicationText = GetScene()->FindComponentById<Text>(multiplicationTextRef))
	{
		float displayMagnification = magnification;
		std::wstring displayText = std::format(L"x{:.1f}", displayMagnification);
		multiplicationText->SetText(displayText);
		multiplicationText->SetColor(isTopRate ? Color::Yellow : Color::White); // トップレートなら黄色、それ以外は白色で表示
	}
}