#pragma once
#include "EasingComponent.h"

class EasingPosition : public EasingComponent
{
public:
	void DrawProperty() override;
};

class EasingRotation : public EasingComponent
{
public:
	void DrawProperty() override;
};

class EasingScale : public EasingComponent
{
public:
	void DrawProperty() override;
};