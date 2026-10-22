#include "pch.h"
#include "BeatScalerComponent.h"
#include "Engine/Audio/BeatManager.h"
#include "Engine/Core/GameObject.h"

REGISTER_COMPONENT(BeatScalerComponent, "Easing")

void BeatScalerComponent::Start()
{
	baseScale = GetOwner()->transform->GetScale().x;
	currentScale = baseScale;
	targetScale = baseScale;
}

void BeatScalerComponent::Update(float deltaTime)
{
	// ビートカウントの更新
	if (BeatManager::IsJustBeat())
	{
		OnBeat();
	}
	else
	{
		targetScale = baseScale;
	}

	// 現在のスケールをターゲットスケールに近づける
	currentScale += (targetScale - currentScale) * scaleSpeed * deltaTime;
	GetOwner()->transform->SetScale({ currentScale, currentScale, currentScale });
}

#ifdef USE_IMGUI
void BeatScalerComponent::DrawProperty(const PropertyDrawContext& context)
{
	ImGui::DragFloat(reinterpret_cast<const char*>(u8"拡大縮小の強度"), &scaleIntensity, 0.0f, 10.0f);
	ImGui::DragFloat(reinterpret_cast<const char*>(u8"拡大縮小の速度"), &scaleSpeed, 0.1f, 200.0f);
	ImGui::DragInt(reinterpret_cast<const char*>(u8"拡大縮小の頻度"), &scaleFrequency, 1, 1, 16);
}
#endif // USE_IMGUI

void BeatScalerComponent::OnBeat()
{
	// ビートカウントを増加
	beatCount++;
	
	// スケール頻度に基づいてスケーリングを行うか判定
	if (beatCount % scaleFrequency != 0)
	{
		return;
	}

	// ターゲットスケールを更新
	targetScale = baseScale * (1.0f + scaleIntensity);
	//LOG_INFO("OnBeat");
}