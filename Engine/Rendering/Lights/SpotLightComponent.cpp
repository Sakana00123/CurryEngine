#include "pch.h"
#include "SpotLightComponent.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

REGISTER_COMPONENT(SpotLightComponent, "Lights");

void SpotLightComponent::OnEnable()
{
	SceneManager::GetLoadingSceneOrCurrentScene()->RegisterSpotLight(this);
}

void SpotLightComponent::OnDisable()
{
	SceneManager::GetCurrentScene()->UnregisterSpotLight(this);
}

SpotLight SpotLightComponent::GetSpotLightData() const
{
	SpotLight data{};
	data.enable = m_InternalEnable ? 1 : 0;
	data.position = GetTransform()->GetWorldPosition();
	Vector3 forward = GetTransform()->GetForward(); 
	data.direction = { forward.x, forward.y, forward.z, 0 };
	data.color = color * intensity;
	data.range = range;
	data.innerCorn = innerConeAngle;
	data.outerCorn = outerConeAngle;
	return data;
}

void SpotLightComponent::DrawProperty()
{
#ifdef USE_IMGUI
	ImGui::ColorEdit3("Color", &color.r);
	ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f);
	ImGui::SliderFloat("Range", &range, 0.1f, 100.0f);
	ImGui::SliderFloat("Inner Cone Angle", &innerConeAngle, 0.0f, 90.0f);
	ImGui::SliderFloat("Outer Cone Angle", &outerConeAngle, 0.0f, 90.0f);
#endif // USE_IMGUI
}