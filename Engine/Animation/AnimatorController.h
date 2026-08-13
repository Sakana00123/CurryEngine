#pragma once
#include "Engine/Core/Reflection/Meta.h"

struct AnimatorController
{
public:
	std::string name;

	int defaultAnimationIndex = 0;

	std::vector<std::string> animationNames; // アニメーション名のリスト

};
