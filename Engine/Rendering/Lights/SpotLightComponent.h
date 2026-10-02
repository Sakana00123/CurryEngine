#pragma once

#include "LightBase.h"
#include "Engine/Core/Color.h"
#include "Engine/Rendering/Pipeline/LightData.h"

/** @brief SpotLightComponent を表すクラスです。 */
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
	/**
	 * @brief SpotLightComponent を構築します。
	 */
	SpotLightComponent() = default;
	/**
	 * @brief SpotLightComponent を破棄します。
	 */
	~SpotLightComponent() override = default;
	/**
	 * @brief Enable イベントを処理します。
	 */
	void OnEnable() override;
	/**
	 * @brief Disable イベントを処理します。
	 */
	void OnDisable() override;

	/**
	 * @brief GetSpotLightData に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	SpotLight GetSpotLightData() const;

#ifdef USE_IMGUI
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI
};
