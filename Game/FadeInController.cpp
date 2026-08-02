#include "pch.h"
#include "FadeInController.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Core/GameObject.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(FadeInController, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(FadeInController, "UserScripts", ComponentAttributes::None, {})


void FadeInController::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	isFading = true; // フェード開始
	Graphics::SetSharedResource("SceneTransitionIsFading", &isFading);
	fadeValue = 0.0f; // フェード開始
	Graphics::SetSharedResource("SceneTransitionFadeValue", &fadeValue);

	fadeEasing.AddEasing(EaseType::Linear, 0.0f, 1.0f, fadeDuration);

	fadeEasing.SetCompletedFunction([this]() {
		isFading = false; // フェード完了
		Graphics::SetSharedResource("SceneTransitionIsFading", &isFading);
		fadeValue = 0.0f; // フェード完了
		Graphics::SetSharedResource("SceneTransitionFadeValue", &fadeValue);
		GetOwner()->Destroy(); // フェード完了後に自身を破棄
		});
}

void FadeInController::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	if (fadeEasing.GetSequenceCount() > 0)
	{
		fadeEasing.Update(fadeValue, deltaTime);

		// フェードの進行度をSharedResourceに保存
		Graphics::SetSharedResource("SceneTransitionFadeValue", &fadeValue);
	}
}