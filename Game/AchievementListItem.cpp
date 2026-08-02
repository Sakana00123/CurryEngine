#include "pch.h"
#include "AchievementListItem.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include <Engine/UI/Text.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(AchievementListItem, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(AchievementListItem, "UserScripts", ComponentAttributes::None, {})


void AchievementListItem::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void AchievementListItem::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void AchievementListItem::SetAchievementData(const AchievementData& data)
{
	// 実績データを設定する処理をここに実装します。
	// 例えば、nameTextReference、descriptionTextReference、progressTextReference を使用して UI を更新するなどの処理が考えられます。
	if (auto* nameText = GetScene()->FindComponentById<Text>(nameTextReference))
	{
		nameText->SetText(data.name); // 実績名を更新
	}
	else
	{
		Console::LogError("Text component not found for the given nameTextReference in AchievementListItem.");
	}
	if (auto* descriptionText = GetScene()->FindComponentById<Text>(descriptionTextReference))
	{
		descriptionText->SetText(data.description); // 実績の説明を更新
	}
	else
	{
		Console::LogError("Text component not found for the given descriptionTextReference in AchievementListItem.");
	}
	if (auto* progressText = GetScene()->FindComponentById<Text>(progressTextReference))
	{
		std::wstring progressStr = std::to_wstring(data.currentProgress) + L"/" + std::to_wstring(data.requiredProgress);
		progressText->SetText(progressStr); // 実績の進行状況を更新
	}
	else
	{
		Console::LogError("Text component not found for the given progressTextReference in AchievementListItem.");
	}
	if (auto* unlockedDateText = GetScene()->FindComponentById<Text>(unlockedDateTextReference))
	{
		unlockedDateText->SetText(data.isUnlocked ? StringToWstring(data.dateUnlocked) : L"----/--/--"); // 実績の解除日時を更新（解除されていない場合は空文字）
	}
	else
	{
		Console::LogError("Text component not found for the given unlockedDateTextReference in AchievementListItem.");
	}
}