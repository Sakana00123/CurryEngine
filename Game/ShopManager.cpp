#include "pch.h"
#include "ShopManager.h"
#include "ItemShop.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include <Engine/UI/Button.h>
#include "Engine/UI/Text.h"
#include "PhaseManager.h"
#include "PreserveValue.h"
#include "UIEasing.h"
#include "Engine/Audio/Audio.h"
#include "TutorialSystem.h"
// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(ShopManager, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(ShopManager, "UserScripts", ComponentAttributes::None, {})


void ShopManager::Start()
{
	//GameObject* itemShopObject = ObjectManager::Find(itemShopReference);
	//if (itemShopObject)
	//{
	//	itemShopObject->SetActive(false); // ショップは最初は非アクティブにしておきます。
	//}

	Button* endButton = GetScene()->FindComponentById<Button>(endButtonReference);
	if (endButton)
	{
		// ボタンのクリックイベントでフェーズを進める処理をバインドします。
		endButton->AddOnClickEvent([this]() {
			// ショップを閉じる処理
			Exit();

			

			// PhaseManager を探してフェーズを設定
			if (auto phaseManagerObj = GetScene()->GetObjectManager()->Find("PhaseManager"))
			{
				if (auto phaseManager = phaseManagerObj->GetComponent<PhaseManager>())
				{
					
					// 買ってても買っていなくても Placement フェーズに進む
					phaseManager->SetPhase(PhaseManager::Placement);
					Audio::PlayOneShot(L"./Assets/Sounds/SE/clickButton.wav", 0.5f);
					
					
				}
			}
			});
	}
	else
	{
		std::cerr << "Error: Button component not found for the given endButtonReference." << std::endl;
	}
}

void ShopManager::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void ShopManager::Enter()
{
	// ショップに入る処理をここに実装します。
	
	//GameObject* itemShopObject = ObjectManager::Find(itemShopReference);
	//if (itemShopObject)
	//{
	//	itemShopObject->SetActive(true); // アイテムショップオブジェクトをアクティブにする
	//}
	//else
	//{
	//	std::cerr << "Error: GameObject not found for the given itemShopReference." << std::endl;
	//}

	if (UIEasing* uiEasing = GetScene()->FindComponentById<UIEasing>(uiEasingReference))
	{
		std::function<void()> onComplete = [this]() {
			// イージング完了後の処理をここに実装します。例: ショップUIのインタラクションを有効にするなど。
			// チュートリアルの特定のステップであれば、次のステップに進める
			if (TutorialSystem* tutorialSystem = TutorialSystem::GetInstance())
			{
				if (tutorialSystem->GetCurrentTutorialStep() == TutorialSystem::TutorialStep::WaitForShopEnter)
				{
					tutorialSystem->AdvanceTutorialStep(); // チュートリアルを進める
				}
			}

			};

		uiEasing->StartEasing(1.0f, onComplete); // 例: イージングを開始する
	}

	GameObject* canvasObject = ObjectManager::Find(canvasReference);
	if (canvasObject)
	{
		canvasObject->SetActive(false); // キャンバスオブジェクトを非アクティブにする
	}
	else
	{
		std::cerr << "Error: GameObject not found for the given canvasReference." << std::endl;
	}

}

void ShopManager::Exit()
{
	// ショップから出る処理をここに実装します。
	//GameObject* itemShopObject = ObjectManager::Find(itemShopReference);
	//if (itemShopObject)
	//{
	//	itemShopObject->SetActive(false); // アイテムショップオブジェクトを非アクティブにする例
	//}

	if (UIEasing* uiEasing = GetScene()->FindComponentById<UIEasing>(uiEasingReference))
	{
		uiEasing->StartEasing(0.0f); // 例: イージングを終了する
	}

	GameObject* canvasObject = ObjectManager::Find(canvasReference);
	if (canvasObject)
	{
		canvasObject->SetActive(true); // キャンバスオブジェクトをアクティブにする例
	}
}