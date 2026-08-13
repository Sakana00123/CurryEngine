#pragma once
#include "Engine/Core/Reflection/Meta.h"

C_ENUM()
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
};
