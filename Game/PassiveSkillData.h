#pragma once
#include <string>
#include "ItemData.h"

struct PassiveSkillData : public ItemData
{
	std::string targetProperty; // 例: "spawnBallCount", "rarityUp", "mass" など、効果の対象となるプロパティ名
	float modifier; // 効果値（例: +1、-0.5、1.2など）

	int maxStack = 1; // スタックの最大数（例: 1、3、5など）。スタック可能なパッシブスキルの場合に使用。
	//std::string stackBehavior; // スタックの挙動（例: "additive", "multiplicative", "override" など）。スタック可能なパッシブスキルの場合に使用。
	std::string shopPrefabPath; // ショップ内でのアイテムのプレハブのファイルパス(参照用)
};