#include "pch.h"
#include "Exit.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/UI/Button.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(Exit, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(Exit, "UserScripts", ComponentAttributes::None, {})


void Exit::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	if (Button* button = GetOwner()->GetComponent<Button>())
	{
		button->AddOnClickEvent([this]() {
			// ボタンがクリックされたときの処理をここに実装します。
			PostQuitMessage(0);

			});
	}
}

void Exit::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}