#include "pch.h"
#include "LoadingController.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(LoadingController, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(LoadingController, "UserScripts", ComponentAttributes::None, {})


void LoadingController::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	isFading = false; // フェード完了
	Graphics::SetSharedResource("SceneTransitionIsFading", &isFading);
	fadeValue = 0.0f; // フェード完了
	Graphics::SetSharedResource("SceneTransitionFadeValue", &fadeValue);
}

void LoadingController::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。

	if (SceneManager::IsLoadingComplete())
	{
		// シーンのロードが完了した場合の処理をここに実装します。
		SceneManager::AllowTransition();
	}
	else
	{
		// シーンがロード中の場合の処理をここに実装します。

	}
}
