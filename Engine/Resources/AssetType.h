#pragma once
#include "Engine/Core/Reflection/Meta.h"

C_ENUM()
/** @brief AssetType を表す列挙型です。 */
enum class AssetType
{
	Unknown,
	Texture,
	Model,
	Sound,
	Scene,
	Prefab,
	Script,
	Shader,
	Material,
	Animation,
	AnimatorController,
	AnimationTimeline,
	Effect,
};
