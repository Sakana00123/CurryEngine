#pragma once

#include "LightBase.h"
#include "Engine/Rendering/Pipeline/LightData.h"

/** @brief DirectionalLightComponent を表すクラスです。 */
class DirectionalLightComponent : public LightBase
{
	C_REFLECT(DirectionalLightComponent)
public:
	C_PROPERTY()
	Color color{ 1, 1, 1, 1 };

public:
	/**
	 * @brief DirectionalLightComponent を構築します。
	 */
	DirectionalLightComponent() = default;
	/**
	 * @brief DirectionalLightComponent を破棄します。
	 */
	~DirectionalLightComponent() override = default;
	/**
	 * @brief Enable イベントを処理します。
	 */
	void OnEnable() override;
	/**
	 * @brief Disable イベントを処理します。
	 */
	void OnDisable() override;

	/**
	 * @brief GetDirectionalLight に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	DirectionalLight GetDirectionalLight() const;

#ifdef USE_IMGUI
	// プロパティ描画
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};
