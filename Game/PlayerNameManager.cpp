#include "pch.h"
#include "PlayerNameManager.h"
#include "Engine/Utils/JsonFileHandler.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/UI/InputField.h"

// エンジンへの登録
REGISTER_COMPONENT(PlayerNameManager, "UserScripts")

// 静的変数の初期化
std::wstring PlayerNameManager::s_playerName = L"Guest";
const std::string PlayerNameManager::SAVE_FILE_PATH = "Assets/Settings/player_name.json";

void PlayerNameManager::Start() {
	// 起動時にロードを実行
	Load();
	Console::Log("PlayerNameManager: Started. Current Name: " + WstringToUtf8(s_playerName));
	GetOwner()->SetActive(true);
}

void PlayerNameManager::Update(float elapsedTime) {

}

bool PlayerNameManager::SetName(const std::wstring& name) {
	std::wstring sanitized = name;
	sanitized.erase(
		std::remove(sanitized.begin(), sanitized.end(), L'\0'),
		sanitized.end()); // NULL文字の除去
	// 1. 前後の空白を削除 (Trim)
	sanitized.erase(0, sanitized.find_first_not_of(L" "));
	sanitized.erase(sanitized.find_last_not_of(L" ") + 1);

	// 2. NGワードチェック (簡易版)
	static const std::vector<std::wstring> ngWords = {
		// --- 英語 (蔑称・性的・攻撃的) ---
		L"Nigger", L"Nigga", L"Faggot", L"Retard", L"Shit", L"Fuck", L"Fucker",
		L"Bitch", L"Whore", L"Pussy", L"Dick", L"Cock", L"Cunt", L"Slut",
		L"Nazi", L"Hitler", L"Jap", L"Chink", L"Kike", L"Pedo", L"Rape",
		L"死ね", L"殺す", L"殺意", L"カス", L"ゴミ", L"クズ", L"デブ",
		L"ブス", L"ハゲ", L"ガイジ", L"土人", L"エラ", L"チョン", L"シナ",
		L"非国民", L"馬鹿", L"バカ", L"あほ", L"アホ",
		L"セックス", L"セフレ", L"まんこ", L"ちんこ", L"クリ", L"フェラ",
		L"オナニー", L"レイプ", L"媚薬", L"近親", L"淫乱", L"精子", L"糞",
		L"ウンコ",
		L"運営", L"GM", L"Admin", L"System", L"管理"
		L"穢多", L"非人", L"エタ", L"ヒニン", L"部落", L"同和", L"在日",
		L"三国人", L"チャンコロ", L"毛唐", L"露助", L"アメ公", L"特亜",
		L"池沼", L"知的", L"不具", L"片輪", L"カタワ", L"唖", L"聾",
		L"めくら", L"びっこ", L"ちんば", L"キチガイ", L"狂人", L"精神病",
		L"百姓", L"土方", L"乞食", L"コジキ", L"売春婦", L"パンパン",
		L"カルト", L"サタン", L"異端", L"情弱",
		L"ニート", L"無職", L"負け組", L"弱男", L"弱女", L"チー牛",
		L"老害", L"ガキ", L"厨房", L"消防", L"ネトウヨ", L"パヨク"
		L"4ね", L"4ネ", L"氏ね", L"〇ね", L"タヒね", L"ころす", L"564",
		L"殺。ね", L"セッ久", L"セッk", L"お〇にー", L"クリトり",
		L"習近平", L"金正恩", L"独裁", L"共産党", L"テロ", L"爆破",
		L"オウム", L"麻原", L"彰晃", L"サリン", L"統一協会", L"創価",
		L"大麻", L"覚醒剤", L"シャブ", L"LSD", L"パケ", L"密輸",
		L"売春", L"JK", L"JD", L"援交", L"パパ活",
		L"Official", L"公式", L"スタッフ", L"Support", L"Help",
		L"User", L"Player", L"Unknown", L"NoName",
		L"F*ck", L"F_u_c_k", L"Phuck", L"Sh1t", L"A$$", L"A55",
		L"Wanker", L"Bastard",
		L"左翼", L"右翼", L"ロリ", L"ショタ", L"ペド", L"ペドフィリア", L"ロリコン", L"ショタコン",
		L"810", L"114514",



	};
	for (const auto& word : ngWords) {
		if (sanitized.find(word) != std::wstring::npos) {
			sanitized = L"Guest"; // 強制変更
			return false;
		}
	}

	// 3. 記号の除去（URLパラメータ破壊防止）
	// 特殊記号が含まれていたら除去、またはデフォルト名へ
	const std::wstring forbiddenChars = L"?&=/\\%#";
	if (sanitized.find_first_of(forbiddenChars) != std::wstring::npos) {
		sanitized = L"Guest";
		return false;
	}

	// 4. 長さ・空チェック
	if (sanitized.empty()) {
		return false;
	}
	else if (sanitized.length() > MAX_NAME_LENGTH) {
		s_playerName = sanitized.substr(0, MAX_NAME_LENGTH);
	}
	else {
		s_playerName = sanitized;
	}

	Save();

	Console::Log("PlayerNameManager: Name set and saved: " + WstringToUtf8(s_playerName));
	return true;
}

std::wstring PlayerNameManager::GetName() {
	return s_playerName;
}

void PlayerNameManager::Save() {
	json j;
	// wstringをUTF-8文字列としてjsonに格納
	j["playerName"] = WstringToUtf8(s_playerName);

	// JsonFileHandlerを使用して保存
	// フォルダ作成やアセットブラウザの更新も自動で行われる
	JsonFileHandler::SaveJsonToFile(j, SAVE_FILE_PATH, JsonIOFormat::Text);
}

void PlayerNameManager::Load() {
	json j;
	// JsonFileHandlerを使用して読み込み
	if (JsonFileHandler::LoadJsonFromFile(j, SAVE_FILE_PATH, JsonIOFormat::Text)) {
		if (j.contains("playerName") && j["playerName"].is_string()) {
			std::string nameUtf8 = j["playerName"];
			s_playerName = Utf8ToWstring(nameUtf8);
		}
	}
}

// --- 文字列変換の実装 ---
std::wstring PlayerNameManager::Utf8ToWstring(const std::string& utf8Str) {
	if (utf8Str.empty()) return L"";
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), NULL, 0);
	std::wstring wstrTo(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), &wstrTo[0], sizeNeeded);
	return wstrTo;
}

std::string PlayerNameManager::WstringToUtf8(const std::wstring& wstr) {
	if (wstr.empty()) return "";
	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
	return strTo;
}