#include "pch.h"
#include "SceneTransitionButton.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#include <Engine/UI/Button.h>
#include "RankingManager.h"
#include "TutorialSystem.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(SceneTransitionButton, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(SceneTransitionButton, "UserScripts", ComponentAttributes::None, {})


void SceneTransitionButton::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	if (Button* button = GetOwner()->GetComponent<Button>())
	{
		button->AddOnClickEvent([this]() {
			// ボタンがクリックされたときの処理をここに実装します。
			
			StartSceneTransition(); // シーン遷移を開始する関数を呼び出す
		});
	}
}

void SceneTransitionButton::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	if (fadeEasing.GetSequenceCount() > 0)
	{
		fadeEasing.Update(fadeValue, deltaTime);

		// フェードの進行度をSharedResourceに保存
		Graphics::SetSharedResource("SceneTransitionFadeValue", &fadeValue);
	}
}

void SceneTransitionButton::StartSceneTransition()
{
	// シーン遷移を開始する関数の実装をここに記述します。
	TutorialSystem::SetTutorialMode(transitionTutorial); // チュートリアルモードの設定

	isFading = false; // フェード開始
	Graphics::SetSharedResource("SceneTransitionFadeValue", &fadeValue); // フェード値を共有リソースに保存
	fadeEasing.AddEasing(EaseType::Linear, 0.0f, 1.0f, fadeDuration);
	Audio::PlayOneShot(L"./Assets/Sounds/SE/clickGameStart.wav", 0.5f);
	fadeEasing.SetCompletedFunction([this]() {
		TransitionScene(); // フェード完了後にシーン遷移を実行
		});
}

void SceneTransitionButton::TransitionScene()
{
	// シーン遷移を実行する関数の実装をここに記述します。
	if (RankingManager* rankingManager = GetScene()->FindComponentById<RankingManager>(rankingManagerReference))
	{
		rankingManager->CancelRequest(); // ランキングの取得を中断する
	}
	if (!transitionSceneName.empty())
	{
		SceneManager::LoadScene(transitionSceneName);
	}
}
