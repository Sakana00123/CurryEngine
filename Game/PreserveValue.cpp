#include "pch.h"
#include "PreserveValue.h"
#include "Ball.h"
#include "MagnificationZone.h"
#include "Engine/Core/GameObject.h"
#include <Engine/UI/Text.h>
#include "Engine/Scenes/Scene.h"

#include "AchievementManager.h"
#include "ItemShop.h"
#include "TutorialSystem.h"


// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(PreserveValue, "Game")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(PreserveValue, "UserScripts", ComponentAttributes::None, {})


void PreserveValue::Start()
{
	preservedValue = 0; // 保存された価値を初期化
}

void PreserveValue::Update(float deltaTime)
{

	// 毎フレームの更新処理をここに実装します。
	if (totalValue >= targetValue * 1.5f)
	{
		AchievementManager::AddProgressToManager(GetScene(), "MONEY_MULTI_1_5", 1);
	}
	if (totalValue >= targetValue * 2.f)
	{
		AchievementManager::AddProgressToManager(GetScene(), "MONEY_MULTI_2", 1);
	}
	if (totalValue >= targetValue * 3.f)
	{
		AchievementManager::AddProgressToManager(GetScene(), "MONEY_MULTI_3", 1);
	}
	if (totalValue >= targetValue * 5.f)
	{
		AchievementManager::AddProgressToManager(GetScene(), "MONEY_MULTI_5", 1);
	}
	if (totalValue >= targetValue * 10.f)
	{
		AchievementManager::AddProgressToManager(GetScene(), "MONEY_MULTI_10", 1);
	}


	AchievementManager::ReplaceProgressToManager(GetScene(), "MONEY_OVER_10000", totalValue);

	AchievementManager::ReplaceProgressToManager(GetScene(), "MONEY_OVER_100000", totalValue);

	AchievementManager::ReplaceProgressToManager(GetScene(), "MONEY_OVER_1000000", totalValue);

	AchievementManager::ReplaceProgressToManager(GetScene(), "MONEY_OVER_10000000", totalValue);

	AchievementManager::ReplaceProgressToManager(GetScene(), "MONEY_OVER_100000000", totalValue);


	static int prevTotalValue = -1;

	if (totalValue != prevTotalValue)
	{
		prevTotalValue = totalValue;

		if (isPowerOfTwo(totalValue))
		{
			AchievementManager::AddProgressToManager(GetScene(), "Natural_Break_1", 1);
			AchievementManager::AddProgressToManager(GetScene(), "Natural_Break_2", 1);
			AchievementManager::AddProgressToManager(GetScene(), "Natural_Break_3", 1);
			AchievementManager::AddProgressToManager(GetScene(), "Natural_Break_4", 1);
			AchievementManager::AddProgressToManager(GetScene(), "Natural_Break_5", 1);
			AchievementManager::AddProgressToManager(GetScene(), "Natural_Break_6", 1);
			AchievementManager::AddProgressToManager(GetScene(), "Natural_Break_7", 1);
			AchievementManager::AddProgressToManager(GetScene(), "Natural_Break_8", 1);
			AchievementManager::AddProgressToManager(GetScene(), "Natural_Break_9", 1);
			AchievementManager::AddProgressToManager(GetScene(), "Natural_Break_10", 1);
		}
	}


}

void PreserveValue::SaveBallValue(int value)
{
	preservedValue += value;
	Console::Log("Ball value preserved: " + std::to_string(preservedValue));
}

// ** @brief 金額を減らす関数 */
void PreserveValue::DecreaseTotalValue(int amount)
{
	totalValue -= amount;
	if (totalValue < 0)
		totalValue = 0; // 負の値にならないようにする
	Console::Log("Total value decreased: " + std::to_string(totalValue));
	decreaseCounter++;
	SetWasUseCoinLastShop(true); // ショップでコインを使用したことを記録するフラグを立てる
	UpdateUIText();

	if (TutorialSystem::IsTutorialMode())
	{
		if (TutorialSystem::GetInstance()->GetCurrentTutorialStep() == TutorialSystem::TutorialStep::PurchaseAll)
		{
			// TODO: チュートリアルの「全部購入させるステップ」で金額を減らす操作が行われたことを検知する処理
			if (decreaseCounter >= 2) // 2回以上減らしたら次のステップへ進む
			{
				TutorialSystem::GetInstance()->AdvanceTutorialStep(); // チュートリアルの次のステップへ進む
			}
		}
	}

}

// ** @brief 金額を保存しておく関数 */
void PreserveValue::SaveTotalValue(int value, bool isBall)
{
	totalValue += value;

	// 目標金額に達しているかをチェックし、達成していれば処理を行う
	if (IsTargetValueReached())
	{
		OnTargetValueAchieved();
	}

	AchievementManager::AddProgressToManager(GetScene(), "TOTAL_EARN_1000", value);
	AchievementManager::AddProgressToManager(GetScene(), "TOTAL_EARN_5000", value);
	AchievementManager::AddProgressToManager(GetScene(), "TOTAL_EARN_10000", value);
	AchievementManager::AddProgressToManager(GetScene(), "TOTAL_EARN_50000", value);
	AchievementManager::AddProgressToManager(GetScene(), "TOTAL_EARN_100000", value);
	AchievementManager::AddProgressToManager(GetScene(), "TOTAL_EARN_500000", value);
	AchievementManager::AddProgressToManager(GetScene(), "TOTAL_EARN_1000000", value);
	AchievementManager::AddProgressToManager(GetScene(), "TOTAL_EARN_5000000", value);
	AchievementManager::AddProgressToManager(GetScene(), "TOTAL_EARN_10000000", value);
	AchievementManager::AddProgressToManager(GetScene(), "TOTAL_EARN_50000000", value);
	AchievementManager::AddProgressToManager(GetScene(), "TOTAL_EARN_100000000", value);
	int val = 0;
	static int prevValue = -1;
	if (prevValue < value)
	{

		val = value;
	}
	else
	{
		val = prevValue;
	}

	if (isBall)
	{
		AchievementManager::ReplaceProgressToManager(GetScene(), "SHOT_EARN_10", val);
		AchievementManager::ReplaceProgressToManager(GetScene(), "SHOT_EARN_50", val);
		AchievementManager::ReplaceProgressToManager(GetScene(), "SHOT_EARN_100", val);
		AchievementManager::ReplaceProgressToManager(GetScene(), "SHOT_EARN_300", val);
		AchievementManager::ReplaceProgressToManager(GetScene(), "SHOT_EARN_500", val);
		AchievementManager::ReplaceProgressToManager(GetScene(), "SHOT_EARN_1000", val);
		AchievementManager::ReplaceProgressToManager(GetScene(), "SHOT_EARN_3000", val);
		AchievementManager::ReplaceProgressToManager(GetScene(), "SHOT_EARN_5000", val);
		AchievementManager::ReplaceProgressToManager(GetScene(), "SHOT_EARN_10000", val);
	}



	Console::Log("Total value preserved: " + std::to_string(totalValue));
	UpdateUIText();

	// ItemShopのインタラクションを更新
	for (auto* itemShop : GetScene()->FindComponents<ItemShop>())
	{
		if (itemShop) itemShop->UpdateInteraction();
	}
}


// ** @brief 全体の金額をリセットする関数 */
void PreserveValue::ResetTotalValue()
{
	totalValue = 0;
	Console::Log("Total value reset.");
	UpdateUIText();
}

// ** @brief 現在の金額をリセットする関数 */
void PreserveValue::ResetPreservedValue()
{
	preservedValue = 0;
	Console::Log("Preserved value reset.");
}

void PreserveValue::SetTargetValue(int value)
{
	targetValue = value; Console::Log("Target value set to: " + std::to_string(targetValue));
	UpdateUIText();
}

void PreserveValue::OnTargetValueAchieved()
{
	Console::Log("Target value achieved!");
	// 目標金額達成時の処理をここに実装します。例: アチーブメントのアンロック、エフェクトの再生など。
	// TODO: 目標金額達成時の処理を実装する(フィードバック用のUIの更新やエフェクトの再生など)

}

void PreserveValue::UpdateUIText()
{
	// textReference を使って Text コンポーネントを取得し、UI を更新する処理をここに実装します。
	Text* text = GetScene()->FindComponentById<Text>(textReference);
	if (text) {
		text->SetText(L"所持金: " + std::to_wstring(totalValue));
	}
	Text* text2 = GetScene()->FindComponentById<Text>(textReference2);
	if (text2) {
		text2->SetText(L"所持金\n" + std::to_wstring(totalValue));
	}

	Text* totalText = GetScene()->FindComponentById<Text>(totalTextReference);
	if (totalText) {
		totalText->SetText(L"目標金額: " + std::to_wstring(targetValue));
	}
	Text* totalText2 = GetScene()->FindComponentById<Text>(totalTextReference2);
	if (totalText2) {
		totalText2->SetText(L"目標金額\n" + std::to_wstring(targetValue));
	}
}