#pragma once
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/string.hpp>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include <string>

#include "Engine/Rendering/Renderers/GltfModelRenderer.h"

struct RhythmAnimationClip
{
	std::string name;       // クリップ名
	GltfModelRenderer::Animation* sourceAnimation = nullptr; // 元となるアニメーション
	
	int startBeat = 0;    // 開始ビート
	int endBeat = -1;      // 終了ビート(-1 の場合は最後まで)
	bool snapToBeat = true; // ビートにスナップするか

};
