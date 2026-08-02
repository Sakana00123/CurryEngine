#include "pch.h"
#include "PassiveSkillView.h"
#include "Engine/Scenes/Scene.h"
#include "ItemTooltipController.h"
#include <Engine\UI\Text.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(PassiveSkillView, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(PassiveSkillView, "UserScripts", ComponentAttributes::None, {})


void PassiveSkillView::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void PassiveSkillView::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void PassiveSkillView::SetPassiveSkillData(const PassiveSkillData& newData)
{
	// パッシブスキルのデータを設定する処理をここに実装します。
	// アイコンを更新
	if (auto iconImage = GetScene()->FindComponentById<Image>(iconImageReference))
	{
		iconImage->SetSource(StringToWstring(newData.iconPath).c_str()); // アイコンのスプライトを更新
	}
	if (auto tooltip = GetScene()->FindComponentById<ItemTooltipController>(tooltipReference))
	{
		tooltip->SetupTooltip(newData); // ツールチップの内容を更新
	}
	if (auto backgroundImage = GetScene()->FindComponentById<Image>(backgroundImageReference))
	{
		backgroundImage->SetSource(StringToWstring(newData.backgroundImagePath).c_str()); // 背景画像を設定
	}
	UpdateStackCount(1); // 初期スタック数を 1 に設定（必要に応じて変更）
}

void PassiveSkillView::UpdateStackCount(int newCount)
{
	// 個数を更新
	if (auto itemCountText = GetScene()->FindComponentById<Text>(itemCountReference))
	{
		itemCountText->SetText((L"x" + std::to_wstring(newCount))); // 個数を更新（例: "x1"）
	}
}