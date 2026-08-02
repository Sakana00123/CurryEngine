#include "pch.h"
#include "AchievementListPresenter.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "AchievementManager.h"
#include "ItemPageController.h"
#include "AchievementListItem.h"
#include <Engine/UI/Button.h>
#include "UIEasing.h"
#include "Engine/Audio/Audio.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(AchievementListPresenter, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(AchievementListPresenter, "UserScripts", ComponentAttributes::None, {})


void AchievementListPresenter::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。

	if (auto* openButton = GetScene()->FindComponentById<Button>(openButtonReference))
	{
		// openButton への参照を保存する処理をここに実装します。
		openButton->AddOnClickEvent([this]() {
			// 実装例: openButton がクリックされたときの処理をここに実装します。
			// 例えば、AchievementListPresenter を表示するなどの処理が考えられます。
			if (auto* achievementPanel = GetScene()->FindGameObjectById(achievementPanelReference))
			{
				if (UIEasing* easing = achievementPanel->GetComponent<UIEasing>())
				{
					//achievementPanel->SetActive(true); // AchievementListPresenter を表示する処理を呼び出す
					easing->StartEasing(1.0f); // UIEasing を開始する処理を呼び出す
					Audio::PlayOneShot(L"./Assets/Sounds/SE/openAchievement.wav", 0.5f); // ボタンクリック音を再生する処理を呼び出す
				}
				else
				{
					Console::LogError("UIEasing component not found for the given achievementPanelReference in AchievementListPresenter.");
				}
			}
		});
	}

	if (auto* closeButton = GetScene()->FindComponentById<Button>(closeButtonReference))
	{
		// closeButton への参照を保存する処理をここに実装します。
		closeButton->AddOnClickEvent([this]() {
			// 実装例: closeButton がクリックされたときの処理をここに実装します。
			// 例えば、AchievementListPresenter を非表示にするなどの処理が考えられます。
			if (auto* achievementPanel = GetScene()->FindGameObjectById(achievementPanelReference))
			{
				if (UIEasing* easing = achievementPanel->GetComponent<UIEasing>())
				{
					easing->StartEasing(0.0f); // UIEasing を開始する処理を呼び出す
					Audio::PlayOneShot(L"./Assets/Sounds/SE/closeAchievement.wav", 0.5f); // ボタンクリック音を再生する処理を呼び出す
				}
				else
				{
					Console::LogError("UIEasing component not found for the given achievementPanelReference in AchievementListPresenter.");
				}
			}
		});
	}
	else
	{
		Console::LogError("Button component not found for the given closeButtonReference in AchievementListPresenter.");
	}


	RefreshAchievementList(); // 実装例: 実績リストを更新する処理を呼び出す
}

void AchievementListPresenter::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void AchievementListPresenter::RefreshAchievementList()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	// 例えば、AchievementManager と ItemPageController への参照を取得して保存するなどの処理を行うことができます。
	if (auto* achievementManager = GetScene()->FindComponentById<AchievementManager>(achievementManagerReference))
	{
		// achievementManager への参照を保存する処理をここに実装します。
		achievementManager->InitList(); // 実装例: AchievementManager のリストを初期化する処理を呼び出す
		achievementManager->LoadStatus(); // 実装例: AchievementManager の状態をロードする処理を呼び出す
		auto& achievements = achievementManager->GetAchievements(); // 実装例: AchievementManager の実績データを取得する処理を呼び出す

		//size_t achievementCount = achievements.size(); // 取得した実績データの数を数える処理を呼び出す
		//auto children = GetOwner()->GetChildren(); // AchievementListPresenter の子オブジェクトをすべて取得する処理を呼び出す
		//size_t childCount = children.size(); // AchievementListPresenter の子オブジェクトの数を数える処理を呼び出す
		//if (achievementCount < childCount)
		//{
		//	// AchievementListPresenter の子オブジェクトの数が実績データの数より多い場合、余分な子オブジェクトを非表示にする処理をここに実装します。
		//	for (size_t i = achievementCount; i < childCount; ++i)
		//	{
		//		children[i]->Destroy(); // 余分な子オブジェクトを破棄する処理を呼び出す
		//	}
		//	Console::Log("AchievementListPresenter: Some child objects are hidden due to fewer achievements than child objects.");
		//}


		auto achievementListItems = GetOwner()->GetComponentsInChildren<AchievementListItem>(); // AchievementListItem コンポーネントを持つ子オブジェクトをすべて取得する処理を呼び出す
		int index = 0; // 実績リストのインデックスを管理する変数
		for (const auto& [id, data] : achievements)
		{
			// 取得した実績データを使用して、実績リストの表示を更新する処理をここに実装します。
			// 例えば、実績の名前や説明を UI に表示するなどの処理が考えられます。
			auto* achievementItem = achievementListItems.size() > index ? achievementListItems[index] : nullptr; // インデックスに対応する AchievementListItem を取得
			if (achievementItem)
			{
				achievementItem->SetAchievementData(data); // 実績データを設定して UI を更新する処理を呼び出す
			}
			else
			{
				Console::LogError("AchievementListItem component not found for the given reference in AchievementListPresenter.");
			}
			++index;
		}
	}

	if (auto* itemPageController = GetScene()->FindComponentById<ItemPageController>(itemPageControllerReference))
	{
		// itemPageController への参照を保存する処理をここに実装します。
		itemPageController->GoToPage(0); // 実装例: ItemPageController のページを最初に移動する処理を呼び出す
	}
}