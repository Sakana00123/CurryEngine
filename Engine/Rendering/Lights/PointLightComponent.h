#pragma once

#include "LightBase.h"
#include "Engine/Core/Color.h"
#include "Engine/Rendering/Pipeline/LightData.h"

/** @brief PointLightComponent を表すクラスです。 */
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
	/**
	 * @brief PointLightComponent を構築します。
	 */
	PointLightComponent() = default;
	/**
	 * @brief PointLightComponent を破棄します。
	 */
	~PointLightComponent() override = default;
	/**
	 * @brief Enable イベントを処理します。
	 */
	void OnEnable() override;
	/**
	 * @brief Disable イベントを処理します。
	 */
	void OnDisable() override;

	/**
	 * @brief GetPointLightData に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	PointLight GetPointLightData() const;

#ifdef USE_IMGUI
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};
