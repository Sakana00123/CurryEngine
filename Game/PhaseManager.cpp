#include "pch.h"
#include "PhaseManager.h"
#include "ShopManager.h"
#include "Inventory.h"
#include "Engine/Scenes/Scene.h"
#include "RoundManager.h"
#include "Flipper.h"
#include "ResultManager.h"
#include "RankingManager.h"
#include "AchievementManager.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(PhaseManager, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(PhaseManager, "UserScripts", ComponentAttributes::None, {})


void PhaseManager::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	//SetPhase(currentPhase); // シーン開始時に現在のフェーズに遷移する
}

void PhaseManager::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void PhaseManager::SetPhase(int phase)
{
	switch (currentPhase)
	{
	case Playing:
		// プレイ中のフェーズから抜ける処理をここに実装します。
		if (auto roundManager = GetScene()->GetObjectManager()->Find("RoundManager")->GetComponent<RoundManager>())
		{
			roundManager->EndRound();
			if (phase == Shop)
			{
				roundManager->OnRoundClear();
			}
		}
		break;
	case Shop:
		// ショップのフェーズから抜ける処理をここに実装します。
	{
		if (auto shopManager = GetScene()->GetObjectManager()->Find("ShopManager")->GetComponent<ShopManager>())
		{
			shopManager->Exit();
		}
		break;
	}
	case Placement:
		// 配置のフェーズから抜ける処理をここに実装します。
	{
		if (auto inventory = GetScene()->GetObjectManager()->Find("Inventory")->GetComponent<Inventory>())
		{
			inventory->Exit();
		}
		break;
	}
	case Result:
		// 結果のフェーズから抜ける処理をここに実装します。
	{
		if (ResultManager* resultManager = GetScene()->FindComponentById<ResultManager>(resultManagerReference))
		{
			resultManager->HideResult();
		}
		if (auto roundManager = GetScene()->GetObjectManager()->Find("RoundManager")->GetComponent<RoundManager>())
		{
			roundManager->EndRound();
			if (phase == Shop)
			{
				roundManager->OnRoundClear();
			}
		}
		break;
	}
	default:
		break;
	}

	// フェーズを直接設定する処理をここに実装します。
	currentPhase = phase;
	// 指定されたフェーズに遷移することができる
	switch (currentPhase)
	{
	case Playing:
		// プレイ中のフェーズに入る処理をここに実装します。
		if (auto roundManager = GetScene()->GetObjectManager()->Find("RoundManager")->GetComponent<RoundManager>())
		{
			roundManager->StartRound();
		}
		break;

	case Shop:
		// ショップのフェーズに入る処理をここに実装します。
	{
		if (auto shopManager = GetScene()->GetObjectManager()->Find("ShopManager")->GetComponent<ShopManager>())
		{
			shopManager->Enter();
		}
		break;
	}

	case Placement:
		// 配置のフェーズに入る処理をここに実装します。
	{
		if (auto inventory = GetScene()->GetObjectManager()->Find("Inventory")->GetComponent<Inventory>())
		{
			inventory->Enter();
		}
		break;
	}
	case Result:
		// 結果のフェーズに入る処理をここに実装します。
	{
		if (RoundManager* roundManager = GetScene()->GetObjectManager()->Find("RoundManager")->GetComponent<RoundManager>())
		{
			roundManager->EndGame();
		}

		if (RankingManager* rankingManager = GetScene()->GetObjectManager()->Find("RankingManager")->GetComponent<RankingManager>())
		{
			if (rankingManager->IsEndlessMode())
			{
				AchievementManager::AddProgressToManager(GetScene(), "ENDLESS", 1);
			}
		}

		if (auto resultManager = GetScene()->FindComponentById<ResultManager>(resultManagerReference))
		{
			resultManager->ShowResult();
		}
		break;
	}
	default:
		break;
	}
}

void PhaseManager::AdvancePhase()
{
	int nextPhase = currentPhase + 1;
	if (nextPhase > 2) nextPhase = 0;
	SetPhase(nextPhase);
}

//void PhaseManager::DrawProperty()
//{
//	// エディタでプロパティを描画するための初期化処理を実装します。
//#ifdef USE_IMGUI
//	Component::DrawProperty(); // 基底クラスのプロパティ描画を呼び出す (必要に応じて追加)
//	// ここに PhaseManager 特有のプロパティ描画コードを追加します。
//	ImGui::Separator();
//	if (ImGui::Button("Advance Phase"))
//	{
//		AdvancePhase();
//	}
//	// --- 追加: 現在のフェーズを文字列に変換して表示する ---
//	const char* phaseName = "Unknown";
//	switch (currentPhase)
//	{
//	case Playing:
//		phaseName = "Playing";
//		break;
//	case Shop:
//		phaseName = "Shop";
//		break;
//	case Placement:
//		phaseName = "Placement";
//		break;
//	default:
//		break;
//	}
//
//	ImGui::Text("Current Phase: %s (%d)", phaseName, currentPhase);
//#endif	
//}
