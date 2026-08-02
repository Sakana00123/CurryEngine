#include "pch.h"
#include "Inventory.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/UI/Button.h"
#include "PhaseManager.h"
#include "AttachManager.h"
#include "RoundManager.h"
#include "PreserveValue.h"
#include "Engine/Core/GameObject.h"
#include <Engine/UI/Image.h>
#include "ItemInvetory.h"
#include "ItemPageController.h"
#include "UIEasing.h"
#include "BlendEasing.h"
#include "Engine/Audio/Audio.h"
#include "TutorialSystem.h"
#include "Engine/Events/EventSystem.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(Inventory, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(Inventory, "UserScripts", ComponentAttributes::None, {})


void Inventory::Start()
{
	//GameObject* inventoryObject = ObjectManager::Find(inventoryReference);
	//if(inventoryObject)
	//{
	//	inventoryObject->SetActive(false); // インベントリオブジェクトを非アクティブにする例
	//}
	//else
	//{
	//	std::cerr << "Error: GameObject not found for the given inventoryReference." << std::endl;
	//}

	// コンポーネントが開始されたときの処理をここに実装します。
	Button* shopButton = GetScene()->FindComponentById<Button>(shopButtonReference);
	Button* nextRoundButton = GetScene()->FindComponentById<Button>(nextRoundButtonReference);

	

	if(shopButton)
	{
		shopButton->AddOnClickEvent([this]() {
			Exit(); // インベントリに入る処理を呼び出す
			
			GameObject* phaseManagerObj = GetScene()->GetObjectManager()->Find("PhaseManager");
			PhaseManager* phaseManager = phaseManagerObj ? phaseManagerObj->GetComponent<PhaseManager>() : nullptr;

			if (phaseManager) 
			{
				phaseManager->SetPhase(PhaseManager::Shop);
				Audio::PlayOneShot(L"./Assets/Sounds/SE/clickButton.wav", 0.5f);
			} 
			else 
			{
				std::cerr << "Error: PhaseManager component not found on the PhaseManager object." << std::endl;
			}
			
			
		});
	}
	else
	{
		std::cerr << "Error: Button component not found for the given shopButtonReference." << std::endl;
	}

	if(nextRoundButton)
	{
		nextRoundButton->AddOnClickEvent([this]() {
			Exit(); // インベントリから出る処理を呼び出す

			GameObject* phaseManagerObj = GetScene()->GetObjectManager()->Find("PhaseManager");
			PhaseManager* phaseManager = phaseManagerObj ? phaseManagerObj->GetComponent<PhaseManager>() : nullptr;

			GameObject* roundManagerObj = GetScene()->GetObjectManager()->Find("RoundManager");
			RoundManager* roundManager = roundManagerObj ? roundManagerObj->GetComponent<RoundManager>() : nullptr;

			

			//ボールの位置をリセット
			/*Ball* ball = GetScene()->GetObjectManager()->Find("Sphere")->GetComponent<Ball>();
			if (ball) 
			{
				ball->ResetToInitialPosition();
				ball->ResetBallCount();
			} */

			if(roundManager && phaseManager)
			{
				roundManager->NextRound();
				phaseManager->SetPhase(PhaseManager::Playing);
				Audio::PlayOneShot(L"./Assets/Sounds/SE/clickButton.wav", 0.5f);
			}
			else 
			{
				std::cerr << "Error: RoundManager or PhaseManager component not found on the respective objects." << std::endl;
			}
			
		});
	}
	else
	{
		std::cerr << "Error: Button component not found for the given nextRoundButtonReference." << std::endl;
	}

	if (Button* previousPageButton = GetScene()->FindComponentById<Button>(previousPageButtonReference))
	{
		previousPageButton->AddOnClickEvent([this]() {
			GameObject* itemListObject = ObjectManager::Find(itemListReference);
			if (itemListObject)
			{
				if (ItemPageController* pageController = itemListObject->GetComponent<ItemPageController>())
				{
					pageController->PreviousPage(); // 前のページに移動する例
					UpdateDisplay();
				}
			}
		});
	}
	else
	{
		std::cerr << "Error: Button component not found for the given previousPageButtonReference." << std::endl;
	}

	if (Button* nextPageButton = GetScene()->FindComponentById<Button>(nextPageButtonReference))
	{
		nextPageButton->AddOnClickEvent([this]() {
			GameObject* itemListObject = ObjectManager::Find(itemListReference);
			if (itemListObject)
			{
				if (ItemPageController* pageController = itemListObject->GetComponent<ItemPageController>())
				{
					pageController->NextPage(); // 次のページに移動する例
					UpdateDisplay();
				}
			}
		});
	}
	else
	{
		std::cerr << "Error: Button component not found for the given nextPageButtonReference." << std::endl;
	}
}

void Inventory::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void Inventory::Enter()
{
	// インベントリに入るときの処理をここに実装します。
	// ショップに入る処理をここに実装します。

	// チュートリアルの特定のステップであれば、次のステップに進める
	if (TutorialSystem* tutorialSystem = TutorialSystem::GetInstance())
	{
		if (tutorialSystem->GetCurrentTutorialStep() == TutorialSystem::TutorialStep::PlacementEnter)
		{
			tutorialSystem->AdvanceTutorialStep(); // チュートリアルを進める
		}
	}
	
	if (UIEasing* uiEasing = GetScene()->FindComponentById<UIEasing>(uiEasingReference))
	{
		// UIEasing コンポーネントを使用してインベントリの表示をアニメーションさせる例
		auto onComplete = [this]() {
			// イージング完了後の処理をここに実装します。必要に応じてコールバック関数を定義して渡すこともできます。
			// チュートリアルの特定のステップであれば、次のステップに進める
			if (TutorialSystem* tutorialSystem = TutorialSystem::GetInstance())
			{
				if (tutorialSystem->GetCurrentTutorialStep() == TutorialSystem::TutorialStep::WaitForPlacementEnter)
				{
					tutorialSystem->AdvanceTutorialStep(); // チュートリアルを進める
				}
			}
			};
		uiEasing->StartEasing(1.0f, onComplete);
	}
	if (BlendEasing* blendEasing = GetScene()->FindComponentById<BlendEasing>(blendEasingReference))
	{
		// BlendEasing コンポーネントを使用して画面のフェードインをアニメーションさせる例
		blendEasing->StartBlend(1.0f); // 目標値を 1.0f に設定してイージングを開始
	}

	if (GameObject* magnificationTextRoot = GetScene()->FindGameObjectById(magnificationTextRootReference))
	{
		magnificationTextRoot->SetActive(true); // 倍率テキストのルートオブジェクトをアクティブにする
	}

	GameObject* itemListObject = GetScene()->FindGameObjectById(itemListReference);
	if (itemListObject)
	{
		//itemListObject->SetActive(true); // アイテムリストオブジェクトをアクティブにする例
		if (ItemPageController* pageController = itemListObject->GetComponent<ItemPageController>())
		{
			pageController->GoToPage(0); // アイテムページを最初のページにリセットする例
			UpdateDisplay(); // アイテムの表示を更新する例
		}
	}
	else
	{
		std::cerr << "Error: GameObject not found for the given itemListReference." << std::endl;
	}
	// AttachManagerに配置待機を依頼する
	GameObject* attachManagerObj = GetScene()->objectManager->Find("AttachManager");
	if (attachManagerObj)
	{
		AttachManager* attachManager = attachManagerObj->GetComponent<AttachManager>();
		if (attachManager)
		{
			attachManager->StartWaitingForAttachment();
		}
	}
}

void Inventory::Exit()
{
	// インベントリから出るときの処理をここに実装します。
	// ショップから出る処理をここに実装します。
	//GameObject* inventoryObject = ObjectManager::Find(inventoryReference);
	//if (inventoryObject)
	//{
	//	inventoryObject->SetActive(false); // インベントリオブジェクトを非アクティブにする例
	//}

	if (UIEasing* uiEasing = GetScene()->FindComponentById<UIEasing>(uiEasingReference))
	{
		// UIEasing コンポーネントを使用してインベントリの非表示をアニメーションさせる例
		uiEasing->StartEasing(0.0f); // 目標位置を開始位置に設定してイージングを開始
	}

	if (BlendEasing* blendEasing = GetScene()->FindComponentById<BlendEasing>(blendEasingReference))
	{
		// BlendEasing コンポーネントを使用して画面のフェードアウトをアニメーションさせる例
		blendEasing->StartBlend(0.0f); // 目標値を 0.0f に設定してイージングを開始
	}

	//GameObject* itemListObject = ObjectManager::Find(itemListReference);
	//if (itemListObject)
	//{
	//	itemListObject->SetActive(false); // アイテムリストオブジェクトを非アクティブにする例
	//}

	//// 使われたアイテムを削除する処理をここに実装します。
	//for (auto* item : removedItems)
	//{
	//	if (item)
	//	{
	//		item->GetOwner()->Destroy(); // アイテムオブジェクトを破棄する例
	//	}
	//}
	//removedItems.clear(); // 削除されたアイテムリストをクリアする

}

void Inventory::AddItem(const GadgetItemData& item)
{
	// 構造体としてデータを記録

	GameObject* itemListObject = GetScene()->GetObjectManager()->Find(itemListReference);
	if (itemListObject)
	{
		auto itemObject = Instantiate(item.inventoryItemPath, itemListObject->GetTransform()); // アイテムのプレハブをインスタンス化する
		if (itemObject)
		{
			RectTransform* rectTransform = itemObject->GetComponent<RectTransform>();
			if (rectTransform)
			{
				purchasedItems.push_back(rectTransform);
			}
			if (ItemInvetory* itemInventory = itemObject->GetComponent<ItemInvetory>())
			{
				itemInventory->SetItem(item);
			}
		}
	}

	//purchasedItems.push_back(newData);

	Console::Log("Inventory Recorded: " + item.name + " (Path: " + item.inventoryItemPath + ")");
	UpdateDisplay();
}

void Inventory::RemoveItem(RectTransform* itemRectTransform)
{
	// アイテムをインベントリから削除する処理をここに実装します。
	auto it = std::find(purchasedItems.begin(), purchasedItems.end(), itemRectTransform);
	if (it != purchasedItems.end())
	{
		purchasedItems.erase(it);
		//removedItems.push_back(itemRectTransform); // 削除されたアイテムを removedItems に追加する例
	}

	//itemRectTransform->GetOwner()->SetActive(false); // アイテムオブジェクトを非アクティブにする
	EventSystem::GetCurrent()->SetSelectedGameObject(nullptr); // アイテムが選択されている場合は選択を解除する(これにより、選択されたアイテムが削除された後に不正な参照を防止します)
	itemRectTransform->GetOwner()->Destroy(); // アイテムオブジェクトを破棄する
	UpdateDisplay();
}

void Inventory::UpdateDisplay()
{
	GameObject* itemListObject = ObjectManager::Find(itemListReference);
	if (itemListObject)
	{
		if (ItemPageController* pageController = itemListObject->GetComponent<ItemPageController>())
		{
			pageController->UpdatePageDisplay(); // アイテムページの表示を更新する例

			// アイテムの表示を更新する処理をここに実装します。
			Button* previousPageButton = GetScene()->FindComponentById<Button>(previousPageButtonReference);
			Button* nextPageButton = GetScene()->FindComponentById<Button>(nextPageButtonReference);

			// 前のページボタンと次のページボタンの表示を更新する例
			if (previousPageButton && nextPageButton)
			{
				previousPageButton->GetOwner()->SetActive(pageController->CanGoToPreviousPage()); // 前のページがある場合は表示する
				nextPageButton->GetOwner()->SetActive(pageController->CanGoToNextPage()); // 次のページがある場合は表示する
			}
		}
	}

	//static const float layoutX = 0.0f; // アイテムのレイアウト位置を管理する変数
	//static const float itemSpacing = 200.0f; // アイテム間のスペース

	//for (size_t i = 0; i < purchasedItems.size(); ++i)
	//{
	//	RectTransform* itemRect = purchasedItems[i];
	//	if (itemRect)
	//	{
	//		// アイテムの表示を更新する処理をここに実装します。
	//		// 例えば、アイテムの名前や価格を表示するためのテキストコンポーネントを更新するなど。
	//		//Image* itemImage = itemRect->gameObject->GetComponent<Image>();
	//		//if (itemImage)
	//		//{
	//		//	// アイテムの画像を設定する例
	//		//	//itemImage->SetSprite(itemPrefabPath); // プレハブパスを使用してスプライトを設定
	//		//	itemImage->SetSource(L"Assets/Textures/ItemIcon.png"); // 例として固定のアイコンを設定
	//		//}

	//		// アイテムの位置を更新する例
	//		itemRect->SetAnchoredPosition({ layoutX + i * itemSpacing, 0.0f }); // 横に並べる例
	//	}
	//}

	//
	//std::string displayText = "Inventory List:\n";
	//for (auto* item : purchasedItems)
	//{
	//	if (item)
	//	{
	//		displayText += "- " + item->gameObject->GetName() + "\n";
	//	}
	//	else
	//	{
	//		displayText += "- [Invalid Item]\n";
	//	}
	//}
	//Console::Log(displayText);
}
