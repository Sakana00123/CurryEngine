#pragma once

#include "LightBase.h"
#include "Engine/Core/Color.h"
#include "Engine/Rendering/Pipeline/LightData.h"

class PointLightComponent : public LightBase
{
	C_REFLECT(PointLightComponent)
public:
	C_PROPERTY()
	Color color = Color::White;
	C_PROPERTY()
	float intensity = 1.0f;
	C_PROPERTY()
	float range = 10.0f;
public:
	PointLightComponent() = default;
	~PointLightComponent() override = default;
	void OnEnable() override;
	void OnDisable() override;

	PointLight GetPointLightData() const;

#ifdef USE_IMGUI
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};
