#pragma once
#include "Engine/Core/Component.h"
#include "AchievementData.h"
#include <unordered_map>
#include <queue>

class AchievementNotifyUI;

class AchievementManager : public Component {
	C_REFLECT(AchievementManager)
public:
	void Start() override;
	void Update(float deltaTime) override;
private:
	// 外部（ItemShop等）から呼び出す
	void Unlock(const std::string& id);

	// 進行状況の追加（カウントアップ）
	void AddProgress(const std::string& id, int amount);
	void ReplaceProgress(const std::string& id, int amount);

	// セーブとロード
	void SaveStatus();
public:
	// 静的関数：シーンからマネージャーを探して Progress を追加する
	static void AddProgressToManager(Scene* scene, const std::string& id, int amount);
	static void ReplaceProgressToManager(Scene* scene, const std::string& id, int amount);

	void InitList();
	void LoadStatus();

	std::map<std::string, AchievementData>& GetAchievements() { return achievements; }
private:
	std::map<std::string, AchievementData> achievements;
	std::queue<std::wstring> displayQueue_name;
	std::queue<std::wstring> displayQueue_description;
	AchievementNotifyUI* notifyUI = nullptr;

#ifdef _DEBUG

	const std::string savePath = "./Assets/Saves/Achievements.json";

#else
	const std::string savePath = "./Assets/Saves/Achievements.bin";
#endif // _DEBUG


};