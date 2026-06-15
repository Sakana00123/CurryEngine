#pragma once
#include "EasingComponent.h"

class EasingColor : public EasingComponent
{
public:
#ifdef USE_IMGUI
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};