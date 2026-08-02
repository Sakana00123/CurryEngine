#include "pch.h"
#include "RoundManager.h"
#include "PreserveValue.h"
#include "Engine/Scenes/Scene.h"
#include "MagnificationZone.h"
#include "ItemShop.h"
#include <Engine/UI/Text.h>
#include "PhaseManager.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#include "AttachManager.h"
#include "Gate.h"
#include "Flipper.h"
#include "ShopItemList.h"
#include "PassiveSkillContainer.h"
#include "ObtrusiveGadgetItemList.h"
#include "ObtrusiveGadgetItemComponent.h"
#include "TestAttachment.h"
#include "ComboText.h"
#include "Gadget.h"
#include "Coin.h"
#include "Engine/Physics/Rigidbody.h"
#include <Engine\UI\Button.h>
#include "Engine/Audio/Audio.h"
#include "TutorialSystem.h"
#include "RerollItem.h"
#include "ShopManager.h"
#include "UIEasing.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(RoundManager, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(RoundManager, "UserScripts", ComponentAttributes::None, {})

void RoundManager::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	initialBallSpawnCount = ballSpawnCount; // 初期のスポーンするボールの個数を保存


	currentRound = 0;
	NextRound(); // 最初のラウンドを開始

	resetTimer = 0.0f;
	isWaitingForReset = false;
}

void RoundManager::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	GameObject* preserveValueObj = GetScene()->objectManager->Find("PreserveValue");
	PreserveValue* preserveValue = preserveValueObj ? preserveValueObj->GetComponent<PreserveValue>() : nullptr;

	GameObject* phaseManagerObj = GetScene()->objectManager->Find("PhaseManager");
	PhaseManager* phaseManager = phaseManagerObj ? phaseManagerObj->GetComponent<PhaseManager>() : nullptr;

	if (!preserveValue || !phaseManager)
	{
		Console::Log("Error: PreserveValue, RoundManager, or PhaseManager component not found in the scene.");
		return; // 必要なコンポーネントが見つからない場合はこれ以上の処理を行わない
	}
	if (phaseManager->GetCurrentPhase() != PhaseManager::Phase::Playing)
	{
		return; // プレイ中以外のフェーズでは更新処理を行わない
	}

	if (isWaitingForReset)
	{
		resetTimer += deltaTime;

		if (resetTimer >= resetDelay)
		{
			ballCount--; // ボールの残り個数を減らす
			UpdateBallCountText(); // ボールの残り個数をUIに反映

			bool isGameOver = false;

			if (ballCount < 0 && (preserveValue == nullptr || preserveValue->IsTargetValueReached() == false))
			{
				Console::Log("Game Over! No more balls left.");
				/*if (preserveValue) preserveValue->ResetPreservedValue();

				if (roundManager != nullptr)
				{
					roundManager->ResetRounds();
				}*/
				isGameOver = true;
				phaseManager->SetPhase(PhaseManager::Phase::Result);
				return; // ゲームオーバーになったらこれ以上の処理は行わない
			}
			else if (ballCount < 0 && preserveValue != nullptr && preserveValue->IsTargetValueReached() == true)
			{
				Console::Log("Target value reached! You win!");

				if (GetCurrentRound() < GetMaxRounds())
				{
					// --- 修正: 目標金額を超えたら次のフェーズ（ショップ）に移行する ---
					preserveValue->SaveTotalValue(preserveValue->GetPreservedValue());

					// フェーズマネージャーをショップフェーズに移行させる処理を追加
					if (phaseManager = phaseManagerObj->GetComponent<PhaseManager>())
					{
						phaseManager->SetPhase(PhaseManager::Shop); // ショップ(1)へ直接移行
					}

					Console::Log("Moved to next phase (Shop).");
					// ボールの処理を完全にリセットし、再びこのループに入らないようにする
					resetTimer = 0.0f;
					isWaitingForReset = false; //ここを false にすることで、次回の Update でこのブロックに入らないようにする
					return; // 処理を抜けて次のフレームでのクリックを待つ
				}
				else
				{
					Console::Log("Congratulations! You've completed all rounds!");
					preserveValue->SaveTotalValue(preserveValue->GetPreservedValue());
					phaseManager->SetPhase(PhaseManager::Phase::Result); // 結果フェーズに移行
					return; // ゲームクリアになったらこれ以上の処理は行わない
				}
			}

			// 以下は配置待機が発生しなかった（ゲーム継続中など）場合の通常リセット処理
			if (!isGameOver && ballCount >= 0)
			{
				ResetBall();

				// チュートリアルの特定のステップであれば、次のステップに進める
				if (TutorialSystem* tutorialSystem = TutorialSystem::GetInstance())
				{
					if (tutorialSystem->GetCurrentTutorialStep() == TutorialSystem::TutorialStep::WaitForFirstShotEnd)
					{
						tutorialSystem->AdvanceTutorialStep(); // チュートリアルを進める
					}
				}

				resetTimer = 0.0f;
				isWaitingForReset = false;
			}
		}
	}
	else if (phaseManager != nullptr && phaseManager->GetCurrentPhase() == PhaseManager::Phase::Playing)
	{
		if (std::vector<Ball*> balls = GetScene()->FindComponents<Ball>(); balls.empty())
		{
			isWaitingForReset = true; // ボールがすべてなくなったらリセット待機状態にする
			resetTimer = 0.0f; // タイマーをリセット
			Console::Log("All balls lost! Starting reset timer...");

			if (preserveValue && preserveValue->IsTargetValueReached() && ballCount == 0)
			{
				OnRoundComplete(); // ラウンド完了時の処理を呼び出す
			}
		}
	}
}

void RoundManager::NextRound()
{
	if (currentRound < maxRounds) currentRound++;

	// TODO: ラウンド開始時のボールの個数のテキストの更新処理に変更を加えるときはここいじる
	ballCount = initialBallCount - 1; // ボールの個数を初期化
	UpdateBallCountText(); // ボールの個数テキストを更新

	UpdateTargetValueText(currentRound); // 目標金額テキストを更新

	//ラウンドが進むにつれて倍率ゾーンの倍率を上げる
	// MagnificationZonesの親オブジェクトから子を全部取得して倍率を更新
	GameObject* zonesParent = GetScene()->GetObjectManager()->Find("MagnificationZones");
	if (zonesParent)
	{
		// 一番多い倍率を持つゾーンを見つけるための変数
		float maxMagnification = 0.0f;
		std::vector<std::pair<float, MagnificationZone*>> zonesWithMaxMagnification; // 最大倍率を持つゾーンのペアを格納するベクター

		for (GameObject* child : zonesParent->GetChildren())
		{
			MagnificationZone* zone = child->GetComponent<MagnificationZone>();
			if (zone)
			{
				float baseMagnification = zone->GetMagnification(); // 元の倍率を取得

				// 0.0から1.0の、0.1間隔のランダムな倍率を生成
				float randomIncreaseMultiplier = static_cast<float>(rand() % 11) / 10.0f; // 0.0から1.0のランダムな値を生成

				float newMagnification = /*(currentRound == 1) ? baseMagnification : */baseMagnification + randomIncreaseMultiplier;

				//if (currentRound == 1)
				//{
				//	zone->SetMagnification(newMagnification, false); // ラウンド1は元の倍率を設定
				//}
				//else
				{
					if (newMagnification > maxMagnification)
					{
						maxMagnification = newMagnification; // 最大倍率を更新

						for (auto& pair : zonesWithMaxMagnification)
						{
							pair.second->SetMagnification(pair.first, false); // これまでの最大倍率を持つゾーンの倍率を通常の更新にする
						}
						zonesWithMaxMagnification.clear(); // 最大倍率を持つゾーンのリストをクリア
						zonesWithMaxMagnification.emplace_back(newMagnification, zone); // 現在のゾーンを最大倍率のゾーンのリストに追加
					}
					else if (fabsf(newMagnification - maxMagnification) < 0.001f) // 新しい倍率が現在の最大倍率とほぼ同じ場合
					{
						zonesWithMaxMagnification.emplace_back(newMagnification, zone); // 同じ最大倍率を持つゾーンもリストに追加
					}
					else
					{
						zone->SetMagnification(newMagnification, false); // 通常の倍率更新
					}
				}
			}
		}

		for (auto& pair : zonesWithMaxMagnification)
		{
			pair.second->SetMagnification(pair.first, true); // 最大倍率を持つゾーンの倍率を更新
		}

	}

	//ラウンド変更時にアイテムショップの状態をリセットする
	for (auto* ItemShop : GetScene()->FindComponents<ItemShop>())
	{
		ItemShop->ResetPurchase();
	}

	UpdateRoundText(); // ラウンドテキストを更新
}

void RoundManager::StartRound()
{
	// ラウンド開始の処理をここに実装します。
	Console::Log("Round " + std::to_string(currentRound) + " started!");

	GameObject* canvasObject = GetScene()->FindGameObjectById(canvasReference);
	if (canvasObject)
	{
		canvasObject->SetActive(true); // キャンバスオブジェクトをアクティブにする例
	}

	// ラウンドが開始されたときに、お邪魔アイテムの更新や配置などの処理をここに実装します。
	if (ObtrusiveGadgetItemList* itemList = GetScene()->FindComponentById<ObtrusiveGadgetItemList>(itemListReference))
	{
		// 3ラウンドごとにお邪魔ガジェットを生成する
		int remainingRounds = 3 - ((currentRound - 1) % 3); // currentRound - 1 をすることで、ラウンド数が3の倍数のときに remainingRounds が 3 になるように調整
		if (TutorialSystem::IsTutorialMode())
		{
			remainingRounds = 3; // チュートリアルモードでは常にお邪魔ガジェットを生成するため、remainingRounds を常に 3 に設定
		}
		// ラウンド数が3の倍数のときにお邪魔ガジェットを生成する
		// ラウンド1の開始時にはお邪魔ガジェットを生成しないようにするため、currentRound > 1 の条件を追加
		if (remainingRounds == 3)
		{
			if (currentRound > 1)
			{
				// AttachManagerに配置待機を依頼する
				TestAttachment* testAttachment = GetScene()->objectManager->Find("TestAttachment")->GetComponent<TestAttachment>();
				if (testAttachment)
				{
					Pin* hitPin = nullptr;

					std::vector<Pin*> pins = GetScene()->FindComponents<Pin>();
					std::vector<Pin*> canEquipPins;
					for (Pin* pin : pins)
					{
						if (pin && !pin->IsEquipped())
						{
							int pinLayer = pin->GetOwner()->GetLayer();
							int targetLayer = 5; // 例: お邪魔ガジェットを配置したいレイヤー番号
							if (pinLayer == targetLayer)
							{
								canEquipPins.push_back(pin);
							}
						}
					}

					int randomIndex = 0;
					if (!canEquipPins.empty())
					{
						randomIndex = rand() % canEquipPins.size();
					}
					else // 配置可能なピンがない場合の処理
					{
						int targetLayer = 7; // お邪魔ガジェットを配置したいレイヤー番号
						for (Pin* pin : pins)
						{
							if (pin && !pin->IsEquipped())
							{
								int pinLayer = pin->GetOwner()->GetLayer();
								if (pinLayer == targetLayer)
								{
									canEquipPins.push_back(pin);
								}
							}
						}
						if (!canEquipPins.empty())
						{
							randomIndex = rand() % canEquipPins.size();
						}
						else
						{
							Console::LogError("No available pins to equip the obtrusive gadget.");
							return; // 配置可能なピンが全くない場合は処理を抜ける
						}
					}
					hitPin = canEquipPins[randomIndex];

					// お邪魔ガジェットを生成する処理をここに実装します。
					for (auto* item : GetScene()->FindComponents<ObtrusiveGadgetItemComponent>())
					{
						GadgetItemData data = item->GetItemData();
						std::string prefabPath = data.prefabPath;

						// 生成する位置をランダムに決定（例: ゲートの周りなど）
						testAttachment->AttachToPin(prefabPath, hitPin);

						if (Gadget* gadget = hitPin->GetOwner()->GetComponentInChildren<Gadget>())
						{
							// Gadget に対する処理をここに実装
							gadget->SetDurability(data.durability); // 耐久値を設定
							gadget->SetGadgetItemData(data); // アイテムデータを設定
						}

						Audio::PlayOneShot(L"Assets/Sounds/SE/placeObtrusiveGadget.wav", 0.5f); // お邪魔ガジェット出現時の効果音を再生
					}
				}
			}

			// TODO: TutorialSystem
			if (!TutorialSystem::IsTutorialMode())
			{
				itemList->RerollItem(); // お邪魔アイテムのリロール処理を呼び出す
			}
		}
	}

	// コインを非kinematicにする(ラウンド開始前の配置フェーズなどでコインが動いてしまうのを防止するため)
	for (auto* coin : GetScene()->FindComponents<Coin>())
	{
		if (!coin || !coin->GetOwner())
		{
			continue; // コインやその所有者が無効な場合はスキップ
		}
		if (Rigidbody* rb = coin->GetOwner()->GetComponent<Rigidbody>())
		{
			rb->SetKinematic(false); // Rigidbody を非 kinematic に設定して物理挙動を再開
		}
	}

	if (TutorialSystem::IsTutorialMode())
	{
		// チュートリアルモードの場合の処理をここに実装します。必要に応じて、チュートリアルの特定のステップでのみお邪魔ガジェットを生成するなどの条件分岐を追加することもできます。
		if (TutorialSystem* tutorialSystem = TutorialSystem::GetInstance())
		{
			if (tutorialSystem->GetCurrentTutorialStep() == TutorialSystem::TutorialStep::NextRound)
			{
				tutorialSystem->AdvanceTutorialStep(); // チュートリアルを進める
			}
		}
	}

	ResetBall(); // ボールをセットする処理を呼び出す
}

void RoundManager::EndRound()
{
	// ラウンド終了の処理をここに実装します。
	Console::Log("Round " + std::to_string(currentRound) + " ended!");

	// コインをkinematicにする(配置フェーズなどでコインが動いてしまうのを防止するため)
	for (auto* coin : GetScene()->FindComponents<Coin>())
	{
		if (!coin || !coin->GetOwner())
		{
			continue; // コインやその所有者が無効な場合はスキップ
		}
		if (Rigidbody* rb = coin->GetOwner()->GetComponent<Rigidbody>())
		{
			rb->SetKinematic(true); // Rigidbody を kinematic に設定して物理挙動を停止
		}
	}

	// ラウンド終了時に、UIでのフィードバックや次のラウンドへの準備などの処理をここに実装します。
	if (GameObject* canvasObject = GetScene()->FindGameObjectById(canvasReference))
	{
		canvasObject->SetActive(false); // キャンバスオブジェクトを非アクティブにする例
	}
}

void RoundManager::EndGame()
{
	if (UIEasing* uiEasing = GetScene()->FindComponentById<UIEasing>(uiEasingReference))
	{
		//uiEasing->StartEasing(0.0f, nullptr);
		uiEasing->GetOwner()->SetActive(false); // UIを一旦非表示にする
	}

	auto* preserveValueObj = GetScene()->objectManager->Find("PreserveValue");
	auto* preserveValue = preserveValueObj ? preserveValueObj->GetComponent<PreserveValue>() : nullptr;

	if (auto* endlessButton = GetScene()->FindGameObjectById(endlessModeButtonReference))
	{
		bool isValueReached = preserveValue ? preserveValue->IsTargetValueReached() : false;
		if ((currentRound >= maxRounds) && isValueReached)
		{
			endlessButton->SetActive(true); // 無限モードボタンを表示する
		}
		else
		{
			endlessButton->SetActive(false); // 無限モードボタンを非表示にする
		}
	}
}

void RoundManager::OnRoundClear()
{
	// ラウンドクリア時の処理をここに実装します。
	Console::Log("Round " + std::to_string(currentRound) + " cleared!");
	// ラウンドクリア時に、UIでのフィードバックや次のラウンドへの準備などの処理をここに実装します。

	if (UIEasing* uiEasing = GetScene()->FindComponentById<UIEasing>(uiEasingReference))
	{
		//uiEasing->StartEasing(0.0f, nullptr);
		uiEasing->GetOwner()->SetActive(false); // UIを一旦非表示にする
	}

	// リロール処理を追加
	// TODO: TutorialSystem
	if (!TutorialSystem::IsTutorialMode())
	{
		for (auto* shopItemList : GetScene()->FindComponents<ShopItemList>())
		{
			if (shopItemList)
			{
				shopItemList->RerollItems();
			}
		}

		// 次の目標金額の更新処理を追加
		UpdateTargetValueText(GetCurrentRound() + 1); // 次のラウンドの目標金額を更新
	}
	else // チュートリアルモードの場合は、リロール処理を行わないようにする。
	{
		// TODO: TutorialSystem
		// チュートリアルモードの場合の処理をここに実装します。必要に応じて、チュートリアルの特定のステップでのみリロール処理を行うなどの条件分岐を追加することもできます。
		// お金を30にする処理を追加
		if (PreserveValue* preserveValue = GetScene()->GetObjectManager()->Find("PreserveValue")->GetComponent<PreserveValue>())
		{
			preserveValue->ResetPreservedValue(); // まずは現在の値をリセットしてから
			preserveValue->ResetTotalValue(); // プレイヤーのお金の合計値もリセット
			preserveValue->SaveTotalValue(30); // プレイヤーのお金の合計値を30に設定
		}

		if (auto* itemList = GetScene()->FindComponentById<ObtrusiveGadgetItemList>(itemListReference))
		{
			//itemList->RerollItem(); // お邪魔アイテムのリロール処理を呼び出す
			itemList->SpawnItem(0); // お邪魔アイテムのリロール処理を呼び出す
		}

		if (auto* rerollItem = GetScene()->FindComponentById<RerollItem>(rerollItemReference))
		{
			rerollItem->currentPrice = 999; // リロールアイテムの価格を999に設定して、チュートリアルモードではリロールできないようにする
			rerollItem->UpdatePrice(); // 価格テキストを更新して、プレイヤーにリロールできないことを明示する
		}


	}

	if (ObtrusiveGadgetItemList* itemList = GetScene()->FindComponentById<ObtrusiveGadgetItemList>(itemListReference))
	{
		int remainingRounds = 3 - ((GetCurrentRound()) % 3); // お邪魔アイテムの残りラウンド数を計算する処理を呼び出す
		if (TutorialSystem::IsTutorialMode())
		{
			remainingRounds = 3; // チュートリアルモードでは常にお邪魔ガジェットを生成するため、remainingRounds を常に 3 に設定
		}
		itemList->UpdateRoundText(remainingRounds); // お邪魔アイテムの残りラウンド数を更新する処理を呼び出す
	}

	// ラウンドクリア時に、前のショップでコインを使わなかったら次のラウンドでコインを増やす処理をここに実装します。
	
	// 前のショップでコインを使わなかった場合の条件をここに実装します。例えば、ShopManager に前のラウンドでコインが使われたかどうかのフラグを追加して、そのフラグをチェックするなどの方法があります。
	
	if (PreserveValue* preserveValue = GetScene()->GetObjectManager()->Find("PreserveValue")->GetComponent<PreserveValue>())
	{
		if (currentRound > 1) // ラウンド2まではコインを増やさないようにする条件を追加
		{
			if (!preserveValue->GetWasUseCoinLastShop())
			{
				int currentMoney = preserveValue->GetTotalValue();
				int moneyToAdd = 0; // 例: ラウンドクリア時に追加するお金の量
				const auto& containers = GetScene()->FindComponents<PassiveSkillContainer>();
				if (!containers.empty())
				{
					PassiveSkillContainer* passiveSkillContainer = containers.front();
					if (passiveSkillContainer)
					{
						// パッシブスキルコンテナから "ClearReward" プロパティの修正値を取得して、ラウンドクリア時の報酬に加算します。
						float modifier = passiveSkillContainer->GetModifier("EnterShopCollectCoin"); // "EnterShopCollectCoin" は、パッシブスキルコンテナが持つプロパティの名前で、ラウンドクリア時の報酬を増やす効果を持つものとします。必要に応じてプロパティ名や加算方法を調整してください。
						moneyToAdd = currentMoney * modifier; // 現在のお金に修正値を乗算して、追加するお金の量を計算します。
					}
				}
				preserveValue->SaveTotalValue(moneyToAdd, false); // プレイヤーのお金の合計値に追加する

			}
		}
		preserveValue->SetWasUseCoinLastShop(false); // 次のラウンドに向けて、前のショップでコインが使われなかったことを保存するフラグをリセットする
	}
	
}

void RoundManager::OnRoundComplete()
{
	// ラウンド完了時の処理をここに実装します。
	Console::Log("Round " + std::to_string(currentRound) + " completed!");

	// ラウンド完了時に、UIでのフィードバックや次のラウンドへの準備などの処理をここに実装します。
	// TODO: 毎ラウンドのクリア演出などを追加する場合はここに実装します。

	Audio::PlayOneShot(L"./Assets/Sounds/SE/roundClear.wav", 0.5f); // ラウンドクリア時の効果音を再生

	if (UIEasing* uiEasing = GetScene()->FindComponentById<UIEasing>(uiEasingReference))
	{
		uiEasing->GetOwner()->SetActive(true); // ラウンドクリアテキストのUIを表示する
		auto callback = [this]() {
			// イージングアニメーションが完了した後の処理をここに実装します。例えば、次のラウンドを開始するなどの処理を追加できます。
			
			// ガジェットのラウンド終了処理を追加
			for (auto* gadget : GetScene()->FindComponents<Gadget>())
			{
				if (gadget)
				{
					gadget->OnRoundEnd();
				}
			}

			};

		uiEasing->StartEasing(1.0f, callback); // ラウンドクリアテキストのイージングアニメーションを開始する処理を呼び出す
	}
}

void RoundManager::UpdateRoundText()
{
	Text* roundText = GetScene()->FindComponentById<Text>(roundTextReference);
	if (roundText)
	{
		std::wstring roundString = L"ラウンド " + std::to_wstring(currentRound);
		if (currentRound <= 10)
		{
			roundString = L"ラウンド " + std::to_wstring(currentRound) + L"/" + std::to_wstring(maxRounds);
		}

		roundText->SetText(roundString);
	}
}

void RoundManager::UpdateBallCountText()
{
	Text* text = GetScene()->FindComponentById<Text>(ballCountReference);
	if (text) {
		text->SetText(L"x " + std::to_wstring(max(0, ballCount)));
	}
}

void RoundManager::ResetBall()
{
	if (ComboText* comboText = GetScene()->FindComponentById<ComboText>(comboTextReference))
	{
		comboText->ResetComboCount(); // コンボ数をリセット
	}

	for (auto* ball : GetScene()->FindComponents<Ball>())
	{
		if (ball)
		{
			ball->GetOwner()->Destroy(); // 既存のボールをすべて破壊してから新しいボールをセットする
		}
	}

	// セットする前に、ボールのスポーン数を更新する
	if (PassiveSkillContainer* passiveSkillContainer = GetScene()->FindComponentById<PassiveSkillContainer>(passiveSkillContainerReference))
	{
		ballSpawnCount = initialBallSpawnCount + static_cast<int>(passiveSkillContainer->GetModifier("BallSpawnCount"));
	}

	// ボールをセットする処理をここに実装します。
	for (int i = 0; i < ballSpawnCount; i++)
	{
		GameObject* ballPrefab = Instantiate(ballPrefabPath, ballSpawnPosition);
	}

	// ボールの位置がリセットされた際に、関連するセットをクリア
	for (auto* gadget : GetScene()->FindComponents<Gadget>())
	{
		if (gadget)
		{
			gadget->ClearBallSet(); // 各ガジェットのセットをクリアする処理を呼び出す
		}
	}

	Gate* gate = GetScene()->GetObjectManager()->Find("Gate_Pivot")->GetComponent<Gate>();
	if (gate)
	{
		gate->OpenGate(); // ゲートを開く処理を呼び出す
	}

}

void RoundManager::UpdateTargetValueText(int round)
{
	// 目標金額を表示するテキストを更新する処理をここに実装します。
	PreserveValue* preserveValue = GetScene()->GetObjectManager()->Find("PreserveValue")->GetComponent<PreserveValue>();
	if (preserveValue)
	{
		// 目標金額の更新処理。ラウンド数に応じて目標金額を増加させる例。
		// Tn = B + ((n - 1) * K) * (n ^ 2)
		// Tn = 目標金額, B = initialTargetValue, n = round, K = difficultyScalingFactor
		int newTargetValue = CalculateTargetValue(initialTargetValue, difficultyScalingFactor, round);
		preserveValue->SetTargetValue(newTargetValue); // 計算した目標金額を設定
	}
}

int RoundManager::CalculateTargetValue(int initialValue, float scalingFactor, int round)
{
	// 例: Tn = B + ((n - 1) * K) * (n ^ 2.5)
/*	int value = static_cast<int>(initialValue + ((round - 1) * scalingFactor) * pow(round, 2.5f));

	float additionalValue = 1.0f; // 追加の修正値を格納する変数
	const auto& containers = GetScene()->FindComponents<PassiveSkillContainer>();
	if (!containers.empty())
	{
		PassiveSkillContainer* passiveSkillContainer = containers.front();
		if (passiveSkillContainer)
		{
			// パッシブスキルコンテナから "GoalRaise" プロパティの修正値を取得して、ボールの価値に加算します。
			additionalValue += (passiveSkillContainer->GetModifier("GoalRaise")); // "GoalRaise" は、パッシブスキルコンテナが持つプロパティの名前で、目標金額を上げる効果を持つものとします。必要に応じてプロパティ名や加算方法を調整してください。
		}
	}
	value = static_cast<int>(value * additionalValue); // 追加の修正値を乗算して最終的な目標金額を計算します。
	return value;*/

	// 序盤ゆるい → 後半爆発する指数

	float exponent = 1.2f + 0.15f * round;

	if (round > 10)
	{
		exponent = 2.0f; // ラウンドが10を超えたら指数を固定
	}

	int value = static_cast<int>(
		initialValue + ((round - 1) * scalingFactor) * powf(round, exponent)
		);

	float valueMultiply = 1.0f;
	const auto& containers = GetScene()->FindComponents<PassiveSkillContainer>();
	if (!containers.empty())
	{
		PassiveSkillContainer* passiveSkillContainer = containers.front();
		if (passiveSkillContainer)
		{
			valueMultiply += passiveSkillContainer->GetModifier("GoalRaise");
		}
	}

	value = static_cast<int>(value * valueMultiply);
	return value;
}

void RoundManager::StartEndlessMode()
{
	// 無限モード開始の処理をここに実装します。
	Console::Log("Endless Mode started!");
	// 無限モードの開始時に、UIでのフィードバックやゲームプレイの変更などの処理をここに実装します。

	maxRounds = INT_MAX; // ラウンド数の上限を非常に大きな値に設定して、事実上無限にする
	GameObject* phaseManagerObj = GetScene()->objectManager->Find("PhaseManager");
	PhaseManager* phaseManager = phaseManagerObj ? phaseManagerObj->GetComponent<PhaseManager>() : nullptr;
	if (phaseManager)
	{
		phaseManager->SetPhase(PhaseManager::Phase::Shop); // 無限モード開始後は直接ショップフェーズに移行する
	}
}