#pragma once
#include "ItemData.h"

struct GadgetItemData : public ItemData
{
	// ガジェットの耐久値
	int durability = 10;
	bool isDurabilityRoundDecrease = false; // ラウンド終了時に耐久値が減少するかどうかのフラグ
};