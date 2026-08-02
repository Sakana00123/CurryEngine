#include "pch.h"
#include "TutorialClickAdvance.h"
#include "TutorialSystem.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Core/Time.h"
#include "Engine/Audio/Audio.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(TutorialClickAdvance, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(TutorialClickAdvance, "UserScripts", ComponentAttributes::None, {})


void TutorialClickAdvance::OnEnable()
{
	// コンポーネントが有効になったときの処理をここに実装します。
	//Time::timeScale = 0.0f; // 時間を停止してチュートリアルモードに入る
}

void TutorialClickAdvance::OnDisable()
{
	// コンポーネントが無効になったときの処理をここに実装します。
	//Time::timeScale = 1.0f; // 時間を再開してチュートリアルモードを終了する
}

void TutorialClickAdvance::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void TutorialClickAdvance::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void TutorialClickAdvance::OnPointerClick(PointerEventData* eventData)
{
	// クリックイベントの処理をここに実装します。
	// 例えば、クリックされたときに次のチュートリアルステップに進む処理などを行うことができます。

	if (TutorialSystem* tutorialSystem = GetScene()->FindComponentById<TutorialSystem>(tutorialSystemReference))
	{
		tutorialSystem->AdvanceTutorialStep(); // チュートリアルのステップを進める関数を呼び出す
		Audio::PlayOneShot(L"./Assets/Sounds/SE/clickTutorial.wav", 0.5f); // クリック時の効果音を再生
	}
	else
	{
		Console::LogError("TutorialSystem component not found for the given tutorialSystemReference.");
	}
}