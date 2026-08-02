#include "pch.h"
#include "PauseButton.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Core/GameObject.h"
#include "Engine/UI/Button.h"
#include "Engine/Core/Time.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(PauseButton, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(PauseButton, "UserScripts", ComponentAttributes::None, {})


void PauseButton::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。

	if (Button* button = GetOwner()->GetComponent<Button>())
	{
		if (!resumeFlag)
		{
			button->SetCustomClickFunction([this]() {
				// クリック可能かを判定する関数をここに実装します。例えば、特定の条件下でクリックを無効にしたい場合など。
				if (InputSystem::GetKeyTrigger(VK_ESCAPE)) // 例: ESC キーがトリガーされたときはクリックを無効にする
				{
					return true;
				}
				return false;
				});
		}

		button->AddOnClickEvent([this]() {
			// ボタンがクリックされたときの処理をここに実装します。
			Time::timeScale = resumeFlag ? 1.0f : 0.0f; // resumeFlag が true のときゲームを再開、false のときゲームを一時停止

			if (GameObject* pauseMenu = GetScene()->FindGameObjectById(pauseMenuReference))
			{
				pauseMenu->SetActive(!resumeFlag); // resumeFlag が true のときポーズメニューを非表示、false のときポーズメニューを表示
			}

			});
	}

}

void PauseButton::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}