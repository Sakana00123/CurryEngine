#include "pch.h"
#include "StartButton.h"

#include "Engine/Scenes/SceneManager.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(StartButton, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(StartButton, "UserScripts", ComponentAttributes::None, {})


void StartButton::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}
void StartButton::Update(float deltaTime)
{
	// コンポーネントが更新されるたびの処理をここに実装します。
}

 
void StartButton::OnPointerClick(PointerEventData* eventData) 
{
	// クリックされたときの処理をここに実装します。
	Console::Log("StartButton clicked!");
	SceneManager::LoadScene("DemoGame0");
}
