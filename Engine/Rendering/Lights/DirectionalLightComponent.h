#pragma once

#include "LightBase.h"
#include "Engine/Rendering/Pipeline/LightData.h"

class DirectionalLightComponent : public LightBase
{
	C_REFLECT(DirectionalLightComponent)
public:
	C_PROPERTY()
	Color color{ 1, 1, 1, 1 };

public:
	DirectionalLightComponent() = default;
	~DirectionalLightComponent() override = default;
	void OnEnable() override;
	void OnDisable() override;

	DirectionalLight GetDirectionalLight() const;

#ifdef USE_IMGUI
	// プロパティ描画
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};
