#pragma once
#include <string>

//struct AchievementData {
//	std::string id;          // 内部ID
//	std::wstring name;        // 表示名
//	std::wstring description; // 説明文
//	bool isUnlocked = false; // 解放フラグ
//};

struct AchievementData {
	std::string id;           // 内部ID
	std::wstring name;
	std::wstring description;
	bool isUnlocked = false;
	int currentProgress = 0;   // 現在のカウント
	int requiredProgress = 1;  // 解除に必要な数
	std::string dateUnlocked = ""; // 解除日時を文字列で保存（例: "2024年06月01日"）

	AchievementData() = default;
	AchievementData(std::string id, std::wstring name, std::wstring description, bool isUnlocked, int currentProgress, int requiredProgress)
		: id(std::move(id)), name(std::move(name)), description(std::move(description)), isUnlocked(isUnlocked), currentProgress(currentProgress), requiredProgress(requiredProgress), dateUnlocked("")
	{
	}

};