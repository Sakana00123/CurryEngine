#include "pch.h"
#include "ItemShop.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/UI/Button.h"
#include "Engine/UI/Text.h"
#include "PreserveValue.h"
#include "MagnificationZone.h"
#include "AttachManager.h"
#include "Inventory.h"
#include "Engine/UI/Graphic.h"
#include "PassiveSkillContainer.h"
#include "PassiveSkillComponent.h"
#include "GadgetItemComponent.h"
#include "AchievementManager.h"
#include <Engine\Audio\Audio.h>
#include "GadgetItemData.h"
#include "RoundManager.h"
#include "TutorialSystem.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(ItemShop, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(ItemShop, "UserScripts", ComponentAttributes::None, {})


void ItemShop::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。

	Button* button = GetScene()->FindComponentById<Button>(buttonReference);
	if (button)
	{
		// クリックイベントにコールバックを登録します。
		button->AddOnClickEvent([this]() {
			if (Button* button = GetScene()->FindComponentById<Button>(buttonReference))
			{
				if (button->IsInteractable())
				{
					// ここにアイテム購入処理を実装します。
					this->Purchase();
				}
				else
				{
					Console::Log("Cannot purchase item. Not enough currency or item already purchased.");
					// クリックできない理由をユーザーに伝えるフィードバックをここに実装します（例: サウンド再生、エフェクト表示など）。

					// 効果音再生
					Audio::PlayOneShot(L"Assets/Sounds/SE/coinShortage.wav");

				}
			}
			});
	}

	else
	{
		Console::LogError("Button component not found for ItemShop!");
	}

	// SoldOutオブジェクトを表示する
	for (auto* child : this->GetOwner()->GetChildren())
	{
		if (child->GetName().find("SoldOut") != std::string::npos)
		{
			child->SetActive(isPurchased);
			break;
		}
	}

	// ラウンドマネージャーから現在のラウンド数を取得して、価格に反映させる
	if (!TutorialSystem::IsTutorialMode())
	{
		GameObject* roundManagerObj = GetScene()->GetObjectManager()->Find("RoundManager");
		RoundManager* roundManager = roundManagerObj ? roundManagerObj->GetComponent<RoundManager>() : nullptr;

		if (roundManager)
		{
			float priceMultiplier = 1.0f + ((roundManager->GetCurrentRound() - 1) * 0.1f); // ラウンドごとに価格が10%増加する
			itemPrice = static_cast<int>(itemPrice * priceMultiplier);
		}
	}

	UpdateInteraction(); // 初期状態のインタラクション更新
}

void ItemShop::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void ItemShop::UpdateInteraction()
{
	// アイテムショップのインタラクション更新処理をここに実装します。

	Button* button = GetScene()->FindComponentById<Button>(buttonReference);
	Image* image = GetScene()->FindComponentById<Image>(iconImageReference);
	Image* backgroundImage = GetScene()->FindComponentById<Image>(backgroundImageReference);
	PreserveValue* preserveValue = GetScene()->objectManager->Find("PreserveValue")->GetComponent<PreserveValue>();
	Text* priceText = GetScene()->FindComponentById<Text>(priceTextReference);
	// 価格テキストを更新します。
	if (priceText)
	{
		priceText->SetText(std::to_wstring(itemPrice));
	}

	// 購入可能かどうかの条件をチェックします。
	bool hasEnoughCurrency = preserveValue && preserveValue->GetTotalValue() >= itemPrice;
	bool canPurchase = hasEnoughCurrency && !isPurchased;

	// アイテムの種類がパッシブスキルの場合、さらに条件を追加します。
	if (itemType == 0)
	{
		GameObject* passiveSkillContainerObj = GetScene()->objectManager->Find("PassiveList");
		PassiveSkillContainer* passiveSkillContainer = (passiveSkillContainerObj) ? passiveSkillContainerObj->GetComponent<PassiveSkillContainer>() : nullptr;
		if (!passiveSkillContainer)
		{
			LOG_ERROR("PassiveSkillContainer not found in scene!");
			return;
		}

		// パッシブアイテムの場合、さらにプレイヤーがすでに同じスキルを持っていないかも確認します。
		if (auto* skillComponent = GetOwner()->GetComponent<PassiveSkillComponent>())
		{
			canPurchase = canPurchase && passiveSkillContainer->CanAcquireSkill(skillComponent->GetPassiveSkillData());
		}
	}

	if (button)
	{
		// ここにボタンのインタラクション更新処理を実装します。
		if (canPurchase)
		{
			button->SetInteractable(true);
			if (image)
			{
				image->SetColor(Color::White); // ホワイトアウト
			}
			if (backgroundImage)
			{
				backgroundImage->SetColor(Color::White); // ホワイトアウト
			}

		}
		else
		{
			button->SetInteractable(false);
			if (image)
			{
				image->SetColor(Color(0.5f, 0.5f, 0.5f, 1.0f)); // グレーアウト
			}
			if (backgroundImage)
			{
				backgroundImage->SetColor(Color(0.5f, 0.5f, 0.5f, 1.0f)); // グレーアウト
			}
		}
	}

	if (priceText)
	{
		if (isPurchased)
		{
			priceText->SetText(L"---"); // 売り切れを示すために価格を "---" に変更
			priceText->SetColor(Color::White);
		}
		else
		{
			priceText->SetText(std::to_wstring(itemPrice));
			priceText->SetColor(hasEnoughCurrency ? Color::White : Color::Red); // 購入できない場合は価格を赤くするなどのフィードバックを追加
		}
	}
}

void ItemShop::Purchase()
{
	if (isPurchased)
	{
		Console::Log("Item already purchased.");
		return; // すでに購入されている場合は処理を終了
	}

	PreserveValue* preserveValue = GetScene()->objectManager->Find("PreserveValue")->GetComponent<PreserveValue>();
	if (preserveValue && preserveValue->GetTotalValue() >= itemPrice)
	{
		preserveValue->DecreaseTotalValue(itemPrice);
		isPurchased = true;

		// 効果音再生
		Audio::PlayOneShot(L"Assets/Sounds/SE/purchase.wav");


		switch (itemType)
		{
		case 0: // Passive
		{
			// Passiveアイテムの購入処理をここに実装します。
			// TODO: PassiveSkillContainerにアイテムの効果を追加する処理を実装する必要があります。
			GameObject* passiveSkillContainerObj = GetScene()->objectManager->Find("PassiveList");
			if (passiveSkillContainerObj)
			{
				PassiveSkillContainer* passiveSkillContainer = passiveSkillContainerObj->GetComponent<PassiveSkillContainer>();
				if (passiveSkillContainer)
				{
					if (auto* skillComponent = GetOwner()->GetComponent<PassiveSkillComponent>())
					{
						PassiveSkillData skillData = skillComponent->GetPassiveSkillData();
						if (Image* backgroundImage = GetScene()->FindComponentById<Image>(backgroundImageReference))
						{
							skillData.backgroundImagePath = backgroundImage->GetTexture() ? backgroundImage->GetTexture()->GetPath() : ""; // アイテムショップの背景画像をアイテムデータに設定
						}
						passiveSkillContainer->AddSkill(skillData);
					}
				}
			}

			Console::Log("Purchased Passive item.");
			break;
		}
		case 1: // Gadget
		{
			// Inventoryに購入記録を追加
			GameObject* inventoryObj = GetScene()->objectManager->Find("Inventory");
			if (inventoryObj)
			{
				Inventory* inventory = inventoryObj->GetComponent<Inventory>();
				if (inventory)
				{
					if (auto* gadgetItemComponent = GetOwner()->GetComponent<GadgetItemComponent>())
					{
						GadgetItemData itemData = gadgetItemComponent->GetItemData();
						itemData.price = itemPrice;
						if (Image* backgroundImage = GetScene()->FindComponentById<Image>(backgroundImageReference))
						{
							itemData.backgroundImagePath = backgroundImage->GetTexture() ? backgroundImage->GetTexture()->GetPath() : ""; // アイテムショップの背景画像をアイテムデータに設定
						}
						inventory->AddItem(itemData);
					}
				}
			}
			Console::Log("Purchased Gadget item.");
			break;
		}
		default:
			break;
		}

		if (RoundManager* roundManager = GetScene()->GetObjectManager()->Find("RoundManager")->GetComponent<RoundManager>())
		{
			roundManager->UpdateTargetValueText(roundManager->GetCurrentRound() + 1); // 目標金額テキストを更新
		}

		AchievementManager::AddProgressToManager(GetScene(), "SHOP_FIRST", 1); // 購入実績の進行状況を追加

		// 3. タイプ別の累計カウント
		if (itemType == 0) { // Passive
			AchievementManager::AddProgressToManager(GetScene(), "PASSIVE_COLLECTOR", 1);
			AchievementManager::AddProgressToManager(GetScene(), "PASSIVE_COLLECTOR_5", 1);
			AchievementManager::AddProgressToManager(GetScene(), "PASSIVE_COLLECTOR_10", 1);
			AchievementManager::AddProgressToManager(GetScene(), "PASSIVE_COLLECTOR_50", 1);
			AchievementManager::AddProgressToManager(GetScene(), "PASSIVE_COLLECTOR_100", 1);
			AchievementManager::AddProgressToManager(GetScene(), "PASSIVE_COLLECTOR_500", 1);
		}
		else if (itemType == 1) { // Gadget
			AchievementManager::AddProgressToManager(GetScene(), "GADGET_MANIA", 1);
			AchievementManager::AddProgressToManager(GetScene(), "GADGET_MANIA_5", 1);
			AchievementManager::AddProgressToManager(GetScene(), "GADGET_MANIA_10", 1);
			AchievementManager::AddProgressToManager(GetScene(), "GADGET_MANIA_50", 1);
			AchievementManager::AddProgressToManager(GetScene(), "GADGET_MANIA_100", 1);
			AchievementManager::AddProgressToManager(GetScene(), "GADGET_MANIA_500", 1);
		}

		// 4. 特殊：ギリギリの買い物
		if (preserveValue->GetTotalValue() == 0) {
			AchievementManager::AddProgressToManager(GetScene(), "BARGAIN_HUNTER", 1);
		}

		// SoldOutオブジェクトを表示する
		for (auto* child : this->GetOwner()->GetChildren())
		{
			if (child->GetName().find("SoldOut") != std::string::npos)
			{
				child->SetActive(isPurchased);
				break;
			}
		}

		//std::cout << "Purchased item: " << itemName << " for " << itemPrice << " coins!" << std::endl;
		for (auto* itemShop : GetScene()->FindComponents<ItemShop>())
		{
			itemShop->UpdateInteraction(); // 購入後のインタラクション更新
		}
	}
	else
	{
		Console::Log("Not enough coins to purchase.");
	}
}

void ItemShop::ResetPurchase()
{
	isPurchased = false;

	// SoldOutオブジェクトを非表示にする
	for (auto* child : this->GetOwner()->GetChildren())
	{
		if (child->GetName().find("SoldOut") != std::string::npos)
		{
			child->SetActive(isPurchased);

			//レイキャストターゲットをオフにする
			Graphic* graphic = child->GetComponent<Graphic>();
			if (graphic)
			{
				graphic->isRaycastTarget = false; // 例: レイヤー0を非インタラクティブなレイヤーとする場合
			}

			break;
		}
	}

	UpdateInteraction(); // インタラクションを更新して、再度購入可能にする
}

//void ItemShop::DrawProperty()
//{
//	// エディタでプロパティを描画するための処理をここに実装します。
//#ifdef USE_IMGUI
//	Component::DrawProperty(); // 基底クラスのプロパティ描画を呼び出す
//
//	// ここにアイテムショップ固有のプロパティ描画を実装します。
//	IMGUI_PROPERTY_BEGIN();
//	// アイテムの種類のコンボボックス
//	const char* itemTypes[] = { "Passive", "Gadget" };
//	IMGUI_PROPERTY_ENUM("Item Type", itemType, itemTypes, IM_ARRAYSIZE(itemTypes));
//
//
//	IMGUI_PROPERTY_END();
//#endif // USE_IMGUI
//
//}
