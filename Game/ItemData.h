#pragma once
#include <string>
//アイテムの情報
struct ItemData
{
	std::string name; // アイテムの名前
	std::string description; // パッシブスキルの説明
	std::string iconPath; // アイコンのファイルパス

	std::string inventoryItemPath; // インベントリ内でのアイテムのプレハブパス
	std::string prefabPath; // アイテムのプレハブパス(3Dモデルなどの実際のアイテムのプレハブパス)
	std::string backgroundImagePath; // アイテムの背景画像のファイルパス

	int price; // アイテムの価格
};

