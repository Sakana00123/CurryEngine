#include "pch.h"
#include "RerollItem.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "ShopItemList.h"
#include <Engine\UI\Button.h>
#include <Engine\Audio\Audio.h>
#include "PreserveValue.h"
#include <Engine\UI\Text.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(RerollItem, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(RerollItem, "UserScripts", ComponentAttributes::None, {})


void RerollItem::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。

	if (Button* button = GetScene()->FindComponentById<Button>(rerollButtonId))
	{
		// クリックイベントにコールバックを登録します。
		button->AddOnClickEvent([this]() {
			if (Button* button = GetScene()->FindComponentById<Button>(rerollButtonId))
			{
				if (button->IsInteractable())
				{
					// クリック可能な場合のみリロール処理を実行します。
					OnRerollButtonClicked();
				}
				else
				{
					Console::Log("Cannot reroll items. Not enough currency or reroll already used.");
					// クリックできない理由をユーザーに伝えるフィードバックをここに実装します（例: サウンド再生、エフェクト表示など）。
					Audio::PlayOneShot(L"Assets/Sounds/SE/coinShortage.wav");
				}
			}
			else
			{
				Console::LogError("Button component not found for RerollItem!");
			}
			});
	}
	else
	{
		Console::LogError("Button component not found for RerollItem!");
	}

	currentPrice = initialPrice; // 初期価格を設定します。
	UpdatePrice(); // 価格表示を初期化します。
}

void RerollItem::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	UpdateInteraction(); // インタラクションの更新処理を呼び出します。
}

void RerollItem::UpdateInteraction()
{
	// インタラクションの更新処理をここに実装します。
	if (Button* button = GetScene()->FindComponentById<Button>(rerollButtonId))
	{
		// ここにインタラクションの更新処理を実装します。
		// 例: プレイヤーの所持金やリロール回数に応じてボタンの interactable 状態を更新するなど。
		PreserveValue* preserveValue = GetScene()->FindComponentById<PreserveValue>(preserveValueId);
		if (preserveValue)
		{
			Text* priceText = GetScene()->FindComponentById<Text>(priceTextId);

			if (preserveValue->GetTotalValue() < currentPrice) // 価格が所持金より高い場合はクリックできないようにする
			{
				button->SetInteractable(false);
				if (priceText)
				{
					priceText->SetColor(Color::Red); // 価格が足りない場合は赤色にするなどのフィードバックを追加できます。
				}
			}
			else
			{
				button->SetInteractable(true);
				if (priceText)
				{
					priceText->SetColor(Color::White); // 価格が足りている場合は通常の色に戻すなどのフィードバックを追加できます。
				}
			}
		}
	}
	else
	{
		Console::LogError("Button component not found for RerollItem!");
	}
}

void RerollItem::UpdatePrice()
{
	// 価格表示を更新する処理をここに実装します。
	if (Text* priceText = GetScene()->FindComponentById<Text>(priceTextId))
	{
		// ここに価格表示の更新処理を実装します。
		priceText->SetText(std::to_wstring(currentPrice));
	}
	else
	{
		Console::LogError("Price Text component not found for RerollItem!");
	}
}

void RerollItem::OnRerollButtonClicked()
{
	// ボタンがクリックされたときの処理をここに実装します。
	// まずはお金が足りているかの確認を行います。
	PreserveValue* preserveValue = GetScene()->FindComponentById<PreserveValue>(preserveValueId);
	if (!preserveValue)
	{
		Console::LogError("PreserveValue component not found for RerollItem!");
		return;
	}
	if (preserveValue->GetTotalValue() < currentPrice)
	{
		Console::Log("Cannot reroll items. Not enough currency.");
		// クリックできない理由をユーザーに伝えるフィードバックをここに実装します（例: サウンド再生、エフェクト表示など）。
		Audio::PlayOneShot(L"Assets/Sounds/SE/coinShortage.wav");
		return;
	}

	// 購入できるので、価格を引いて更新します。
	preserveValue->DecreaseTotalValue(currentPrice);

	// ここにアイテムのリロール処理を実装します。
	ShopItemList* passiveList = GetScene()->FindComponentById<ShopItemList>(passiveShopItemListId);
	ShopItemList* gadgetList = GetScene()->FindComponentById<ShopItemList>(gadgetShopItemListId);
	if (!passiveList || !gadgetList)
	{
		Console::LogError("Passive ShopItemList not found for RerollItem!");
		return;
	}
	passiveList->RerollItems();
	gadgetList->RerollItems();
	Console::Log("Items rerolled successfully!");
	// クリック成功のフィードバックをユーザーに伝える処理をここに実装します（例: サウンド再生、エフェクト表示など）。
	Audio::PlayOneShot(L"Assets/Sounds/SE/purchase.wav");
	
	// 価格を更新します。
	//currentPrice += static_cast<int>(currentPrice * priceIncreaseRate);
	// 価格の増加率を乗じて新しい価格を計算します。
	currentPrice += (static_cast<int>(currentPrice * priceIncreaseRate) + 1); // 価格の増加率を乗じた値に 1 を加算して切り上げることで、価格が少なくとも 1 ずつ増加するようにします。

	UpdatePrice(); // 価格表示を更新します。
}