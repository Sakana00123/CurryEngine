#pragma once
#include "EasingComponent.h"

class EasingPosition : public EasingComponent
{
public:
#ifdef USE_IMGUI
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};

class EasingRotation : public EasingComponent
{
public:
#ifdef USE_IMGUI
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};

class EasingScale : public EasingComponent
{
public:
#ifdef USE_IMGUI
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};