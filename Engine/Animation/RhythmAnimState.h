#pragma once
#include "RhythmAnimationClip.h"

// リズムアニメーションステートの定義
struct RhythmAnimState
{
	std::string name; // ステート名
	RhythmAnimationClip clip; // 再生するクリップ

	bool loop = true; // ループ再生するか
	bool syncToBeat = true; // ビートに同期するか
};
