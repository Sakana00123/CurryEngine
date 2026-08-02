#include "pch.h"
#include "EndlessButton.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/UI/Button.h"
#include "RankingManager.h"
#include "RoundManager.h"
#include "Engine/Audio/Audio.h"
#include <Engine/Audio/AudioSource.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(EndlessButton, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(EndlessButton, "UserScripts", ComponentAttributes::None, {})


void EndlessButton::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	if (Button* button = GetOwner()->GetComponent<Button>())
	{
		button->AddOnClickEvent([this]() {
			// ボタンがクリックされたときの処理をここに実装します。
			OnClick();
			Audio::PlayOneShot(L"Assets/Sounds/SE/clickGameStart.wav", 0.5f); // クリック音を再生
			});
	}
}

void EndlessButton::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void EndlessButton::OnClick()
{
	// ボタンがクリックされたときの処理をここに実装します。
	RankingManager* rankingManager = GetScene()->FindComponentById<RankingManager>(rankingManagerReference);
	if (rankingManager)
	{
		rankingManager->SetEndlessMode(true); // Endlessモードを設定
	}

	if (RoundManager* roundManager = GetScene()->FindComponentById<RoundManager>(roundManagerReference))
	{
		roundManager->StartEndlessMode(); // Endlessモードの開始処理を呼び出す
	}

	if (GameObject* bgmObj = GetScene()->objectManager->Find("BGM")) {
		if (AudioSource* mainBgm = bgmObj->GetComponent<AudioSource>()) {
			mainBgm->Play();
		}
	}

}