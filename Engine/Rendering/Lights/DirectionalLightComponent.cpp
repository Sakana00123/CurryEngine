#include "pch.h"
#include "DirectionalLightComponent.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

REGISTER_COMPONENT(DirectionalLightComponent, "Lights");

void DirectionalLightComponent::OnEnable()
{
	SceneManager::GetLoadingSceneOrCurrentScene()->RegisterDirectionalLight(this);
	m_InternalEnable = true;
}

void DirectionalLightComponent::OnDisable()
{
	SceneManager::GetCurrentScene()->UnregisterDirectionalLight(this);
	m_InternalEnable = false;
}

DirectionalLight DirectionalLightComponent::GetDirectionalLight() const
{
	DirectionalLight light{};
	light.enable = m_InternalEnable;
	XMFLOAT3 direction = gameObject->transform->GetForward();
	light.direction = XMFLOAT4(direction.x, direction.y, direction.z, 0);
	light.color = color;
	return light;
}

void DirectionalLightComponent::DrawProperty()
{
#ifdef USE_IMGUI
	ImGui::ColorEdit4("Color", &color.r);
#endif // USE_IMGUI
}