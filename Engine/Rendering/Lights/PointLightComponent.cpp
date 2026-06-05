#include "pch.h"
#include "PointLightComponent.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

REGISTER_COMPONENT(PointLightComponent, "Lights");

void PointLightComponent::OnEnable()
{
	// シーンのポイントライトリストに自身を登録
	SceneManager::GetLoadingSceneOrCurrentScene()->RegisterPointLight(this);
}

void PointLightComponent::OnDisable()
{
	// シーンのポイントライトリストから自身を削除
	SceneManager::GetCurrentScene()->UnregisterPointLight(this);
}

PointLight PointLightComponent::GetPointLightData() const
{
	PointLight lightData;
	lightData.enable = m_InternalEnable ? 1 : 0;
	lightData.position = GetOwner()->transform->position;
	lightData.color = color * intensity;
	lightData.range = range;
	return lightData;
}

void PointLightComponent::DrawProperty()
{
#ifdef USE_IMGUI

	ImGui::ColorEdit3("Color", &color.r);
	ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 10.0f);
	ImGui::DragFloat("Range", &range, 0.1f, 0.0f, 100.0f);

#endif // USE_IMGUI
}