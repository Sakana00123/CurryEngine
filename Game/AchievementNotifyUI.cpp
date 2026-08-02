#include "pch.h"
#include "AchievementNotifyUI.h"
#include "Engine/Scenes/Scene.h"

#include "Engine/Audio/Audio.h"
#include "Engine/Audio/AudioSource.h"

REGISTER_COMPONENT(AchievementNotifyUI, "UserScripts")

void AchievementNotifyUI::Start() {
	GetOwner()->SetActive(false);
}

void AchievementNotifyUI::Show(const std::wstring& name, const std::wstring& description) {
	if (Text* t = GetScene()->FindComponentById<Text>(nameTextRef)) {
		t->SetText(name);
	}
	if (Text* t = GetScene()->FindComponentById<Text>(descriptionTextRef)) {
		t->SetText(description);
	}
	timer = 0.0f;
	isShowing = true;
	GetOwner()->SetActive(true);
	Audio::PlayOneShot(L"./Assets/Sounds/SE/achievementUnlock.wav");
}

void AchievementNotifyUI::Update(float deltaTime) {
	if (!isShowing) return;
	timer += deltaTime;

	// フェードイン・アウト処理
	float alpha = 0.0f;
	if (timer < 0.5f) alpha = timer / 0.5f;
	else if (timer < 3.5f) alpha = 1.0f;
	else if (timer < 4.0f) alpha = 1.0f - ((timer - 3.5f) / 0.5f);
	else {
		isShowing = false;
		GetOwner()->SetActive(false);
		return;
	}

	ApplyAlpha(alpha);

	/*	// タイトルのようにサイン波で少し揺らす
		if (RectTransform* rect = GetRectTransform()) {
			float yPos = std::sin(timer * 2.0f) * 5.0f;
			rect->SetAnchoredPosition({ rect->anchoredPosition.x, rect->anchoredPosition.y + (yPos * deltaTime) });
		}*/
}

void AchievementNotifyUI::ApplyAlpha(float alpha) {
	if (Image* img = GetScene()->FindComponentById<Image>(backgroundRef)) img->color.a = alpha;
	if (Text* txt = GetScene()->FindComponentById<Text>(nameTextRef)) txt->color.a = alpha;
	if (Text* txt = GetScene()->FindComponentById<Text>(descriptionTextRef)) txt->color.a = alpha;
}