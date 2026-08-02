#include "pch.h"
#include "ItemTooltipController.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include <Engine\UI\Text.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(ItemTooltipController, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(ItemTooltipController, "UserScripts", ComponentAttributes::None, {})


void ItemTooltipController::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void ItemTooltipController::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void ItemTooltipController::OnPointerEnter(PointerEventData* eventData)
{
	// ポインタが対象に入った際の処理をここに実装します。
	// ツールチップを表示するなど。
	SetTooltipActive(true);
}

void ItemTooltipController::OnPointerExit(PointerEventData* eventData)
{
	// ポインタが対象から離れた際の処理をここに実装します。
	// ツールチップを非表示にするなど。
	SetTooltipActive(false);
}


void ItemTooltipController::SetupTooltip(const ItemData& data)
{
	// ツールチップを設定する処理をここに実装します。
	// 名前や説明を表示するテキストコンポーネント、アイコンを表示するイメージコンポーネントへの参照を取得して保存します。
	if (Text* nameText = GetScene()->FindComponentById<Text>(nameTextReference))
	{
		nameText->SetText(StringToWstring(data.name)); // スキルの名前をテキストコンポーネントに設定
	}
	if (Text* descriptionText = GetScene()->FindComponentById<Text>(descriptionTextReference))
	{
		descriptionText->SetText(StringToWstring(data.description)); // スキルの説明をテキストコンポーネントに設定
	}
}

void ItemTooltipController::SetTooltipActive(bool active)
{
	// ツールチップの表示/非表示を切り替える処理をここに実装します。
	// 例えば、ツールチップ全体を含むGameObjectをアクティブ/非アクティブにするなど。
	if (GameObject* tooltipPanel = GetScene()->GetObjectManager()->Find(tooltipPanelReference))
	{
		tooltipPanel->SetActive(active); // ツールチップのパネルをアクティブ/非アクティブに切り替える
	}
}