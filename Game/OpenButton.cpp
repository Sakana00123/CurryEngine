#include "pch.h"
#include "OpenButton.h"
#include "Engine/UI/Button.h"
#include "Engine/Editor/FileOpener.h"
#include "Engine/Audio/Audio.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(OpenButton, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(OpenButton, "UserScripts", ComponentAttributes::None, {})


void OpenButton::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	if (Button* button = GetOwner()->GetComponent<Button>())
	{
		button->AddOnClickEvent([this]() {
			// ボタンがクリックされたときの処理をここに実装します。
			OnClick();
			Audio::PlayOneShot(L"./Assets/Sounds/SE/openRanking.wav", 0.5f);
			});
	}
}

void OpenButton::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void OpenButton::OnClick()
{
	// ボタンがクリックされたときの処理をここに実装します。
	// 例えば、指定されたパスのファイルを開く処理などを行うことができます。
	if (!openPath.empty())
	{
		std::wstring wOpenPath(openPath.begin(), openPath.end());
		OpenFileWithDefaultApplication(wOpenPath);
	}
}