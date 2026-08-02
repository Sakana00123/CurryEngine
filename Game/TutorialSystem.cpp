#include "pch.h"
#include "TutorialSystem.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#include "PhaseManager.h"
#include "SceneTransitionButton.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(TutorialSystem, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(TutorialSystem, "UserScripts", ComponentAttributes::None, {})


void TutorialSystem::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	if (isTutorialMode)
	{
		AdvanceTutorialStep();
	}
}

void TutorialSystem::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void TutorialSystem::OnDestroy()
{
	// コンポーネントが破棄されるときの処理をここに実装します。
#ifndef _DEBUG
	SetTutorialMode(false); // チュートリアルモードを無効にする  
#endif // _DEBUG

}


void TutorialSystem::SetTutorialMode(bool enabled)
{
	isTutorialMode = enabled;
}

TutorialSystem* TutorialSystem::GetInstance()
{
	if (auto tutorialSystemObj = SceneManager::GetCurrentScene()->GetObjectManager()->Find("TutorialSystem"))
	{
		if (auto tutorialSystemComp = tutorialSystemObj->GetComponent<TutorialSystem>())
		{
			return tutorialSystemComp;
		}
		else
		{
			LOG_ERROR("TutorialSystem component not found on the tutorial UI object.");
			return nullptr;
		}
	}
	else
	{
		LOG_ERROR("Tutorial UI object not found for the given reference.");
		return nullptr;
	}
}

void TutorialSystem::AdvanceTutorialStep()
{
	// 進める前の現在のステップに応じた処理をここに実装します。後処理など。
	switch(static_cast<TutorialStep>(currentStep))
	{
		case TutorialStep::Welcome:
		{
			if (auto welcomeGuideObj = GetScene()->FindGameObjectById(welcomeGuideReference))
			{
				welcomeGuideObj->SetActive(false); // ウェルカムガイドを非表示にする例
			}
			break;
		}
		case TutorialStep::Shot:
		{
			if (auto shotGuideObj = GetScene()->FindGameObjectById(shotGuideReference))
			{
				shotGuideObj->SetActive(false); // ショットガイドを非表示にする例
			}
			break;
		}
		case TutorialStep::OneRoundTwoShots:
		{
			if (auto oneRoundTwoShotsGuideObj = GetScene()->FindGameObjectById(oneRoundTwoShotsGuideReference))
			{
				oneRoundTwoShotsGuideObj->SetActive(false); // 1ラウンド2ショットガイドを非表示にする例
			}
			break;
		}
		case TutorialStep::Shop:
		{
			if (auto shopGuideObj = GetScene()->FindGameObjectById(shopGuideReference))
			{
				shopGuideObj->SetActive(false); // ショップガイドを非表示にする例
			}
			break;
		}
		case TutorialStep::PurchaseAll:
		{
			if (auto purchaseAllGuideObj = GetScene()->FindGameObjectById(purchaseAllGuideReference))
			{
				purchaseAllGuideObj->SetActive(false); // 全部購入ガイドを非表示にする例
			}
			break;
		}
		case TutorialStep::PlacementEnter:
		{
			if (auto enterPlacementGuideObj = GetScene()->FindGameObjectById(enterPlacementGuideReference))
			{
				enterPlacementGuideObj->SetActive(false); // 配置に入るガイドを非表示にする例
			}
			break;
		}
		case TutorialStep::Placement:
		{
			if (auto placementGuideObj = GetScene()->FindGameObjectById(placementGuideReference))
			{
				placementGuideObj->SetActive(false); // 配置ガイドを非表示にする例
			}
			break;
		}
		case TutorialStep::NextRound:
		{
			// 次のラウンドに進む前の処理をここに実装します。
			if (auto nextRoundGuideObj = GetScene()->FindGameObjectById(nextRoundGuideReference))
			{
				nextRoundGuideObj->SetActive(false); // 次のラウンドに進むガイドを非表示にする例
			}
			break;
		}
		case TutorialStep::ObtrusiveGadget:
		{
			// お邪魔ガジェットの説明前の処理をここに実装します。
			if (auto obtrusiveGadgetGuideObj = GetScene()->FindGameObjectById(obtrusiveGadgetGuideReference))
			{
				obtrusiveGadgetGuideObj->SetActive(false); // お邪魔ガジェットのガイドを非表示にする例
			}
			break;
		}
		case TutorialStep::Objective:
		{
			// ゲームの目的や勝利条件の説明前の処理をここに実装します。
			if (auto objectiveGuideObj = GetScene()->FindGameObjectById(objectiveGuideReference))
			{
				objectiveGuideObj->SetActive(false); // 目的のガイドを非表示にする例
			}
			break;
		}
		case TutorialStep::Complete:
			LOG_INFO("Tutorial is already complete. No further steps to advance.");
			return; // チュートリアルが完了している場合はこれ以上進めない
		default:
			break;
	}

	currentStep++;
	// チュートリアルのステップを進めるための処理をここに実装します。

	switch (static_cast<TutorialStep>(currentStep))
	{
		case TutorialStep::Welcome:
		{
			// Welcome ステップの処理を実装します。
			if (auto nameplateObj = GetScene()->FindGameObjectById(nameplateReference))
			{
				nameplateObj->SetActive(false); // ネームプレートを非表示にする例
			}
			if (auto welcomeGuideObj = GetScene()->FindGameObjectById(welcomeGuideReference))
			{
				welcomeGuideObj->SetActive(true); // ウェルカムガイドを表示する例
			}
			break;
		}
		case TutorialStep::Shot:
		{
			// Shot ステップの処理を実装します。
			if (auto phaseManager = GetScene()->FindComponentById<PhaseManager>(phaseManagerReference))
			{
				phaseManager->SetPhase(PhaseManager::Phase::Playing); // プレイフェーズに切り替える例
			}
			else
			{
				LOG_ERROR("RoundManager object not found for the given reference.");
			}
			if (auto shotGuideObj = GetScene()->FindGameObjectById(shotGuideReference))
			{
				shotGuideObj->SetActive(true); // ショットガイドを表示する例
			}
			break;
		}
		case TutorialStep::WaitForFirstShotEnd:
		{
			// WaitForFirstShotEnd ステップの処理を実装します。
			break;
		}
		case TutorialStep::OneRoundTwoShots:
		{
			// OneRoundTwoShots ステップの処理を実装します。
			if (auto oneRoundTwoShotsGuideObj = GetScene()->FindGameObjectById(oneRoundTwoShotsGuideReference))
			{
				oneRoundTwoShotsGuideObj->SetActive(true); // 1ラウンド2ショットガイドを表示する例
			}
			break;
		}
		case TutorialStep::WaitForShopEnter:
		{
			// WaitForEndShots ステップの処理を実装します。
			break;
		}
		case TutorialStep::Shop:
		{
			// Shop ステップの処理を実装します。
			if (auto shopGuideObj = GetScene()->FindGameObjectById(shopGuideReference))
			{
				shopGuideObj->SetActive(true); // ショップガイドを表示する例
			}
			break;
		}
		case TutorialStep::PurchaseAll:
		{
			// PurchaseAll ステップの処理を実装します。
			if (auto purchaseAllGuideObj = GetScene()->FindGameObjectById(purchaseAllGuideReference))
			{
				purchaseAllGuideObj->SetActive(true); // 全部購入ガイドを表示する例
			}
			break;
		}
		case TutorialStep::PlacementEnter:
		{
			// PlacementEnter ステップの処理を実装します。
			if (auto enterPlacementGuideObj = GetScene()->FindGameObjectById(enterPlacementGuideReference))
			{
				enterPlacementGuideObj->SetActive(true); // 配置に入るガイドを表示する例
			}
			break;
		}
		case TutorialStep::WaitForPlacementEnter:
		{
			// WaitForPlacementEnter ステップの処理を実装します。
			break;
		}
		case TutorialStep::Placement:
		{
			// Placement ステップの処理を実装します。
			if (auto placementGuideObj = GetScene()->FindGameObjectById(placementGuideReference))
			{
				placementGuideObj->SetActive(true); // 配置ガイドを表示する例
			}
			break;
		}
		case TutorialStep::WaitForPlacementEnd:
		{
			// WaitForPlacementEnd ステップの処理を実装します。
			break;
		}
		case TutorialStep::NextRound:
		{
			// NextRound ステップの処理を実装します。
			if (auto nextRoundGuideObj = GetScene()->FindGameObjectById(nextRoundGuideReference))
			{
				nextRoundGuideObj->SetActive(true); // 次のラウンドに進むガイドを表示する例
			}
			break;
		}
		case TutorialStep::ObtrusiveGadget:
		{
			// ObtrusiveGadget ステップの処理を実装します。
			if (auto obtrusiveGadgetGuideObj = GetScene()->FindGameObjectById(obtrusiveGadgetGuideReference))
			{
				obtrusiveGadgetGuideObj->SetActive(true); // お邪魔ガジェットのガイドを表示する例
			}
			break;
		}
		case TutorialStep::Objective:
		{
			// Objective ステップの処理を実装します。
			if (auto objectiveGuideObj = GetScene()->FindGameObjectById(objectiveGuideReference))
			{
				objectiveGuideObj->SetActive(true); // 目的や勝利条件のガイドを表示する例
			}
			break;
		}
		case TutorialStep::Complete:
			// Complete ステップの処理を実装します。
			if (auto sceneTransitionButton = GetScene()->FindComponentById<SceneTransitionButton>(sceneTransitionButtonReference))
			{
				sceneTransitionButton->StartSceneTransition(); // シーン遷移を開始する例
			}
			break;
	default:
		break;
	}


}
