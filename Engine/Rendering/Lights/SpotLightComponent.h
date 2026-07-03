#pragma once

#include "LightBase.h"
#include "Engine/Core/Color.h"
#include "Engine/Rendering/Pipeline/LightData.h"

class SpotLightComponent : public LightBase
{
	C_REFLECT(SpotLightComponent)
public:
	C_PROPERTY()
	Color color = Color::White;
	C_PROPERTY()
	float intensity = 1.0f;
	C_PROPERTY()
	float range = 10.0f;
	C_PROPERTY()
	float innerConeAngle = 30.0f; // 内側のコーン角度（度数法）
	C_PROPERTY()
	float outerConeAngle = 45.0f; // 外側のコーン角度（度数法）

public:
	SpotLightComponent() = default;
	~SpotLightComponent() override = default;
	void OnEnable() override;
	void OnDisable() override;

	SpotLight GetSpotLightData() const;

#ifdef USE_IMGUI
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI
};
