#pragma once
#include "EasingComponent.h"

/** @brief EasingPosition を表すクラスです。 */
class EasingPosition : public EasingComponent
{
public:
#ifdef USE_IMGUI
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};

/** @brief EasingRotation を表すクラスです。 */
class EasingRotation : public EasingComponent
{
public:
#ifdef USE_IMGUI
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};

/** @brief EasingScale を表すクラスです。 */
class EasingScale : public EasingComponent
{
public:
#ifdef USE_IMGUI
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};