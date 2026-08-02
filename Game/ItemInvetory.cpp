#include "pch.h"
#include "ItemInvetory.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Inventory.h"
#include "Engine/UI/Button.h"
#include "AttachManager.h"
#include "ItemTooltipController.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Utils/stdUtiles.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(ItemInvetory, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(ItemInvetory, "UserScripts", ComponentAttributes::None, {})


void ItemInvetory::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。

	Button* button = GetScene()->FindComponentById<Button>(buttonReference);
	if (button)
	{
		// クリックイベントにコールバックを登録します。
		button->AddOnClickEvent([this]() {

			Button* button = GetScene()->FindComponentById<Button>(buttonReference);
			if (button->IsInteractable())
			{
				OnUseItem(); // アイテム使用の処理を呼び出す
				Audio::PlayOneShot(L"./Assets/Sounds/SE/selectSetupGadget.wav", 0.5f); // アイテム使用時の効果音を再生
			}
			else // Disabled 状態でクリックされた場合はキャンセル処理を呼び出す
			{
				OnCancel(); // アイテム使用がキャンセルされたときの処理を呼び出す
				Audio::PlayOneShot(L"./Assets/Sounds/SE/cancelSetupGadget.wav", 0.5f); // キャンセル時の効果音を再生
			}
			
		});
	}
}

void ItemInvetory::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void ItemInvetory::SetItem(const GadgetItemData& item)
{
	itemData = item; // アイテムデータを保存

	// アイテムの表示を更新する処理をここに実装します。
	if (auto* image = GetScene()->FindComponentById<Image>(imageReference))
	{
		image->SetSource(StringToWstring(item.iconPath).c_str()); // アイコンを更新
	}
	else
	{
		Console::LogError("Image component not found for the given imageReference in ItemInvetory.");
	}

	if (auto* backgroundImage = GetScene()->FindComponentById<Image>(backgroundImageReference))
	{
		if (!item.backgroundImagePath.empty())
		{
			backgroundImage->SetSource(StringToWstring(item.backgroundImagePath).c_str()); // アイテムの背景画像を設定
		}
	}
	else
	{
		Console::LogError("Image component not found for the given backgroundImageReference in ItemInvetory.");
	}

	if (auto* tooltip = GetScene()->FindComponentById<ItemTooltipController>(tooltipReference))
	{
		tooltip->SetupTooltip(item); // ツールチップのデータを更新
	}
	else
	{
		Console::LogError("ItemTooltipController component not found for the given tooltipReference in ItemInvetory.");
	}

}

void ItemInvetory::UpdateDisplay()
{
	// アイテムの表示を更新する処理をここに実装します。

}

void ItemInvetory::OnUseItem()
{
	// アイテムが使用されたときの処理をここに実装します。
	for (auto* itemInventory : GetScene()->FindComponents<ItemInvetory>())
	{
		if (itemInventory != this)
		{
			itemInventory->OnCancel(); // 他の ItemInvetory コンポーネントのキャンセル処理を呼び出す
		}
	}
	Button* button = GetScene()->FindComponentById<Button>(buttonReference);
	if (button)
	{
		button->SetInteractable(false); // アイテム使用中はボタンを無効化
	}

	GameObject* zoneObj = GetScene()->GetObjectManager()->Find(attachReference);
	if (zoneObj)
	{
		AttachManager* magZone = zoneObj->GetComponent<AttachManager>();
		if (magZone)
		{
			magZone->SetSelectedPrefabPath(itemData, [this]() {
				OnConfirmed();
			}, [this]() {
				OnCancel();
			});
			Console::Log("Item selected: " + itemData.name + " Path: " + itemData.prefabPath);
		}
		else
		{
			Console::Log("Error: AttachManager object found, but AttachManager component is missing.");
		}
	}
	else
	{
		Console::Log("Error: attachReference is invalid or not set in Inspector.");
	}
}

void ItemInvetory::OnCancel()
{
	// アイテムの使用がキャンセルされたときの処理をここに実装します。
	Button* button = GetScene()->FindComponentById<Button>(buttonReference);
	if (button)
	{
		button->SetInteractable(true); // アイテム使用がキャンセルされたので、ボタンを再度有効化
	}

	GameObject* zoneObj = GetScene()->GetObjectManager()->Find(attachReference);
	if (zoneObj)
	{
		AttachManager* magZone = zoneObj->GetComponent<AttachManager>();
		if (magZone)
		{
			magZone->ClearSelectedPrefab(); // 選択されたプレハブをクリアする関数を呼び出す
		}
	}
}

void ItemInvetory::OnConfirmed()
{
	// アイテムの使用が確定したときの処理をここに実装します。
	Inventory* inventory = GetScene()->GetObjectManager()->Find("Inventory")->GetComponent<Inventory>();
	if (inventory)
	{
		inventory->RemoveItem(GetOwner()->GetComponent<RectTransform>());
	}
}
