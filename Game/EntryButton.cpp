#include "pch.h"
#include "EntryButton.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "PhaseManager.h"
#include "Engine/UI/Button.h"
#include <Engine/UI/InputField.h>
#include "Engine/UI/Text.h"
#include "PlayerNameManager.h"

#include "Engine/Audio/Audio.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(EntryButton, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(EntryButton, "UserScripts", ComponentAttributes::None, {})


void EntryButton::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	if (Button* button = GetOwner()->GetComponent<Button>())
	{
		button->AddOnClickEvent([this]() {
			// ボタンがクリックされたときの処理をここに実装します。

			if (auto inputField = GetScene()->FindComponentById<InputField>(InputFieldRef))
			{
				std::wstring playerName = inputField->GetTextComponent()->GetText();
				// プレイヤー名を PlayerNameManager に保存する
				if (!PlayerNameManager::SetName(playerName)) {

					Audio::PlayOneShot(L"./Assets/Sounds/SE/ButtonFalse.wav");
					return; // 名前の保存に失敗した場合は処理を中断（例: 
				}
				else {

					Audio::PlayOneShot(L"./Assets/Sounds/SE/clickButton.wav");
				}
			}

			if (auto entryPanel = GetScene()->GetObjectManager()->Find(EntryPanelRef))
			{
				entryPanel->SetActive(false); // エントリーパネルを非表示にする例
			}

			if (auto phaseManager = GetScene()->FindComponentById<PhaseManager>(PhaseManagerRef))
			{
				phaseManager->SetPhase(PhaseManager::Phase::Playing); // プレイ中のフェーズに切り替える例
			}
			});
	}

	if (auto inputField = GetScene()->FindComponentById<InputField>(InputFieldRef))
	{
		std::wstring playerName = PlayerNameManager::GetName();
		inputField->GetTextComponent()->SetText(playerName); // 既に保存されているプレイヤー名を入力フィールドに表示する
	}
}

void EntryButton::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}