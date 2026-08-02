#include "pch.h"
#include "AchievementManager.h"
#include "AchievementNotifyUI.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Utils/JsonFileHandler.h"
#include "TutorialSystem.h"

REGISTER_COMPONENT(AchievementManager, "UserScripts")

//void AchievementManager::Start() {
//	InitList();
//	if (GameObject* uiObj = GetScene()->objectManager->Find("AchievementPopup")) {
//		notifyUI = uiObj->GetComponent<AchievementNotifyUI>();
//	}
//}

inline std::string GetCurrentDate()
{
	// 現在の日時を取得(2024/06/01形式)
	std::time_t t = std::time(nullptr);
	std::tm tm;
	localtime_s(&tm, &t);
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y/%m/%d", &tm);
	return std::string(buffer);
}

void AchievementManager::Start() {
	InitList();   // まずリストを初期化
	LoadStatus(); // 保存されたデータを上書きロード

	if (GameObject* uiObj = GetScene()->objectManager->Find("AchievementPopup")) {
		notifyUI = uiObj->GetComponent<AchievementNotifyUI>();
	}
}




void AchievementManager::InitList() {
	achievements["SHOP_FIRST"] = { "SHOP_FIRST", L"はじめてのおつかい", L"ショップで初めてアイテムを購入した", false, 0, 1 };

	achievements["BARGAIN_HUNTER"] = { "BARGAIN_HUNTER", L"寒いお財布", L"所持金を使い切る勢いで買い物をした", false, 0, 1 };

	achievements["GADGET_MANIA"] = { "GADGET_MANIA", L"初めてのガジェット", L"ガジェットを1回購入した", false, 0, 1 };
	achievements["GADGET_MANIA_5"] = { "GADGET_MANIA_5",L"ガジェット沼の入り口",		L"ガジェットを5回購入した",		false, 0, 5 };
	achievements["GADGET_MANIA_10"] = { "GADGET_MANIA_10",L"気づけば常連",L"ガジェットを10回購入した",false, 0, 10 };
	achievements["GADGET_MANIA_50"] = { "GADGET_MANIA_50",L"ガジェット中毒",	L"ガジェットを50回購入した",false, 0, 50 };
	achievements["GADGET_MANIA_100"] = { "GADGET_MANIA_100",L"ガジェットがないと生きていけない",L"ガジェットを100回購入した",false, 0, 100 };
	achievements["GADGET_MANIA_500"] = { "GADGET_MANIA_500",L"ガジェットに人生を捧げし者",L"ガジェットを500回購入した",false, 0, 500 };

	achievements["PASSIVE_COLLECTOR"] = { "PASSIVE_COLLECTOR", L"ぼくなにかしちゃいました？", L"パッシブスキルを1回購入した", false, 0, 1 };
	achievements["PASSIVE_COLLECTOR_5"] = { "PASSIVE_COLLECTOR_5", L"強化の味を覚えた者", L"パッシブスキルを5回購入した", false, 0, 5 };
	achievements["PASSIVE_COLLECTOR_10"] = { "PASSIVE_COLLECTOR_10", L"パッシブ10個積むと人格変わる説", L"パッシブスキルを10回購入した", false, 0, 10 };
	achievements["PASSIVE_COLLECTOR_50"] = { "PASSIVE_COLLECTOR_50", L"強化が止まらない", L"パッシブスキルを50回購入した", false, 0, 50 };
	achievements["PASSIVE_COLLECTOR_100"] = { "PASSIVE_COLLECTOR_100", L"強化の極意", L"パッシブスキルを100回購入した", false, 0, 100 };
	achievements["PASSIVE_COLLECTOR_500"] = { "PASSIVE_COLLECTOR_500", L"パッシブに魂を売った者", L"パッシブスキルを500回購入した", false, 0, 500 };

	achievements["MONEY_MULTI_1_5"] = { "MONEY_MULTI_1_5", L"ちょっとリッチな気分", L"所持金が目標金額の1.5倍に到達した", false, 0,1 };
	achievements["MONEY_MULTI_2"] = { "MONEY_MULTI_2", L"まだまだ通過点", L"所持金が目標金額の2倍に到達した", false, 0,1 };
	achievements["MONEY_MULTI_3"] = { "MONEY_MULTI_3", L"そろそろ税務署が気にし始める", L"所持金が目標金額の3倍に到達した", false, 0,1 };
	achievements["MONEY_MULTI_5"] = { "MONEY_MULTI_5", L"金運バグ発生中", L"所持金が目標金額の5倍に到達した", false, 0,1 };
	achievements["MONEY_MULTI_10"] = { "MONEY_MULTI_10", L"資産界のラスボス", L"所持金が目標金額の10倍に到達した", false, 0, 1 };

	achievements["Natural_Break_1"] = { "Natural_Break_1", L"ビットの目覚め", L"所持金が2の累乗に1回到達した。", false, 0, 1 };
	achievements["Natural_Break_2"] = { "Natural_Break_2", L"脳内ビット化開始", L"所持金が2の累乗に2回到達した。", false, 0, 2 };
	achievements["Natural_Break_3"] = { "Natural_Break_3", L"あなたの脳はもうバイナリ", L"所持金が2の累乗に3回到達した。", false, 0, 3 };
	achievements["Natural_Break_4"] = { "Natural_Break_4", L"脳内レジスタ稼働中", L"所持金が2の累乗に4回到達した。", false, 0, 4 };
	achievements["Natural_Break_5"] = { "Natural_Break_5", L"思考がシフト演算", L"所持金が2の累乗に5回到達した。", false, 0, 5 };
	achievements["Natural_Break_6"] = { "Natural_Break_6", L"脳内スタックオーバーフロー寸前", L"所持金が2の累乗に6回到達した。", false, 0, 6 };
	achievements["Natural_Break_7"] = { "Natural_Break_7", L"メモリ最適化完了", L"所持金が2の累乗に7回到達した。", false, 0, 7 };
	achievements["Natural_Break_8"] = { "Natural_Break_8", L"あなたの脳はキャッシュヒット率100%", L"所持金が2の累乗に8回到達した。", false, 0, 8 };
	achievements["Natural_Break_9"] = { "Natural_Break_9", L"脳内コンパイル成功", L"所持金が2の累乗に9回到達した。", false, 0, 9 };
	achievements["Natural_Break_10"] = { "Natural_Break_10", L"完全バイナリ生命体", L"所持金が2の累乗に10回到達した。", false, 0, 10 };

	achievements["TOTAL_EARN_1000"] = { "TOTAL_EARN_1000", L"財布がちょっと重くなった", L"累計で1000稼いだ", false, 0, 1000 };
	achievements["TOTAL_EARN_5000"] = { "TOTAL_EARN_5000", L"小銭マイスター", L"累計で5000稼いだ", false, 0, 5000 };
	achievements["TOTAL_EARN_10000"] = { "TOTAL_EARN_10000", L"万札の気配を感じる", L"累計で1万稼いだ", false, 0, 10000 };
	achievements["TOTAL_EARN_50000"] = { "TOTAL_EARN_50000", L"5万の景色も悪くない", L"累計で5万稼いだ", false, 0, 50000 };
	achievements["TOTAL_EARN_100000"] = { "TOTAL_EARN_100000", L"10万の壁", L"累計で10万稼いだ", false, 0, 100000 };
	achievements["TOTAL_EARN_500000"] = { "TOTAL_EARN_500000", L"ハーフミリオン", L"累計で50万稼いだ", false, 0, 500000 };
	achievements["TOTAL_EARN_1000000"] = { "TOTAL_EARN_1000000", L"クイズミリオネア", L"累計で100万稼いだ", false, 0, 1000000 };
	achievements["TOTAL_EARN_5000000"] = { "TOTAL_EARN_5000000", L"500万の景色", L"累計で500万稼いだ", false, 0, 5000000 };
	achievements["TOTAL_EARN_10000000"] = { "TOTAL_EARN_10000000", L"1000万の資産帯", L"累計で1000万稼いだ", false, 0, 10000000 };
	achievements["TOTAL_EARN_50000000"] = { "TOTAL_EARN_50000000", L"金運チート使用中", L"累計で5000万稼いだ", false, 0, 50000000 };
	achievements["TOTAL_EARN_100000000"] = { "TOTAL_EARN_100000000", L"1億の向こう側へ", L"累計で1億稼いだ", false, 0, 100000000 };

	achievements["MONEY_OVER_10000"] = { "MONEY_OVER_10000", L"財布が急に重くなった", L"所持金が1万を超えた", false, 0, 10000 };
	achievements["MONEY_OVER_100000"] = { "MONEY_OVER_100000", L"10万の貫禄", L"所持金が10万を超えた", false, 0, 100000 };
	achievements["MONEY_OVER_1000000"] = { "MONEY_OVER_1000000", L"金運の波に乗った者", L"所持金が100万を超えた", false, 0, 1000000 };
	achievements["MONEY_OVER_10000000"] = { "MONEY_OVER_10000000", L"もうもどれない", L"所持金が1000万を超えた", false, 0, 10000000 };
	achievements["MONEY_OVER_100000000"] = { "MONEY_OVER_100000000", L"億万長者の風格", L"所持金が1億を超えた", false, 0, 100000000 };

	achievements["SHOT_EARN_10"] = { "SHOT_EARN_10", L"10円ぱん", L"玉1発で10以上稼いだ", false, 0, 10 };
	achievements["SHOT_EARN_50"] = { "SHOT_EARN_50", L"五十鈴", L"玉1発で50以上稼いだ", false, 0, 50 };
	achievements["SHOT_EARN_100"] = { "SHOT_EARN_100", L"百裂肉球", L"玉1発で100以上稼いだ", false, 0, 100 };
	achievements["SHOT_EARN_300"] = { "SHOT_EARN_300", L"三百人委員会", L"玉1発で300以上稼いだ", false, 0, 300 };
	achievements["SHOT_EARN_500"] = { "SHOT_EARN_500", L"五百羅漢", L"玉1発で500以上稼いだ", false, 0, 500 };
	achievements["SHOT_EARN_1000"] = { "SHOT_EARN_1000", L"一騎当千", L"玉1発で1000以上稼いだ", false, 0, 1000 };
	achievements["SHOT_EARN_3000"] = { "SHOT_EARN_3000", L"三・千・世・界", L"玉1発で3000以上稼いだ", false, 0, 3000 };
	achievements["SHOT_EARN_5000"] = { "SHOT_EARN_5000", L"五千枚瓦正拳", L"玉1発で5000以上稼いだ", false, 0, 5000 };
	achievements["SHOT_EARN_10000"] = { "SHOT_EARN_10000", L"一撃一万", L"玉1発で10000以上稼いだ", false, 0, 10000 };

	achievements["ENDLESS"] = { "ENDLESS", L"無限回廊", L"何らかの条件を満たす", false, 0, 1 };

	achievements["DEBUGGER"] = { "DEBUGGER", L"デバッガー", L"不審な挙動を発見する", false, 0, 1 };

}

//void AchievementManager::Unlock(const std::string& id) {
//	if (achievements.count(id) && !achievements[id].isUnlocked) {
//		achievements[id].isUnlocked = true;
//		displayQueue_name.push(achievements[id].name);
//		displayQueue_description.push(achievements[id].description);
//	}
//}
void AchievementManager::Unlock(const std::string& id) {
	if (achievements.count(id) && !achievements[id].isUnlocked) {
		achievements[id].isUnlocked = true;
		displayQueue_name.push(achievements[id].name);
		displayQueue_description.push(achievements[id].description);
		achievements[id].dateUnlocked = GetCurrentDate(); // 解除日時を設定
		SaveStatus(); // 解除されたらセーブ
	}
}
void AchievementManager::Update(float deltaTime) {
	if (!notifyUI || displayQueue_name.empty()) return;
	if (!notifyUI || displayQueue_description.empty()) return;

	if (!notifyUI->IsShowing()) {
		notifyUI->Show(displayQueue_name.front(), displayQueue_description.front());
		displayQueue_name.pop();
		displayQueue_description.pop();
	}
}

void AchievementManager::AddProgress(const std::string& id, int amount) {
	if (TutorialSystem::IsTutorialMode()) return; // チュートリアル中は進行状況を追加しない
	if (achievements.count(id) && !achievements[id].isUnlocked) {
		auto& data = achievements[id];
		data.currentProgress += amount;

		if (data.currentProgress >= data.requiredProgress) {
			Unlock(id);
		}
		else {
			SaveStatus(); // 進行中もセーブ
		}
	}
}
void AchievementManager::ReplaceProgress(const std::string& id, int amount) {
	if (TutorialSystem::IsTutorialMode()) return; // チュートリアル中は進行状況を追加しない
	if (achievements.count(id) && !achievements[id].isUnlocked) {
		auto& data = achievements[id];
		data.currentProgress = amount;
		if (data.currentProgress >= data.requiredProgress) {
			Unlock(id);
		}
		else {
			SaveStatus(); // 進行中もセーブ
		}
	}
}
// --- 保存処理 ---
void AchievementManager::SaveStatus() {
	json j = json::object();

	for (auto& [id, data] : achievements) {
		json item;
		item["unlocked"] = data.isUnlocked;
		item["progress"] = data.currentProgress;
		item["dateUnlocked"] = data.dateUnlocked;
		j[id] = item;
	}

	JsonFileHandler::SaveJsonToFile(j, savePath);
}

// --- 読み込み処理 ---
void AchievementManager::LoadStatus() {
	json j;
	if (!JsonFileHandler::LoadJsonFromFile(j, savePath)) return;

	for (auto it = j.begin(); it != j.end(); ++it) {
		std::string id = it.key();
		if (achievements.count(id)) {
			achievements[id].isUnlocked = it.value().value("unlocked", false);
			achievements[id].currentProgress = it.value().value("progress", 0);
			achievements[id].dateUnlocked = it.value().value("dateUnlocked", "----/--/--");
		}
	}
}

void AchievementManager::AddProgressToManager(Scene* scene, const std::string& id, int amount) {
	if (!scene) return;
	GameObject* obj = scene->objectManager->Find("AchievementManager");
	if (obj) {
		if (auto* am = obj->GetComponent<AchievementManager>()) {
			am->AddProgress(id, amount);
		}
	}
}

void AchievementManager::ReplaceProgressToManager(Scene* scene, const std::string& id, int amount) {
	if (!scene) return;
	GameObject* obj = scene->objectManager->Find("AchievementManager");
	if (obj) {
		if (auto* am = obj->GetComponent<AchievementManager>()) {
			am->ReplaceProgress(id, amount);
		}
	}
}
