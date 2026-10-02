#pragma once
#include "EasingComponent.h"

/** @brief EasingColor を表すクラスです。 */
class EasingColor : public EasingComponent
{
public:
#ifdef USE_IMGUI
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};