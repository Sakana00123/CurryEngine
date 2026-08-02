#pragma once
#include "Engine/Core/Component.h"

#include "Engine/Events/EventHandlers.h"
#include <string>

/**
 * @brief プレイヤー名を管理するコンポーネント。
 * インスタンスを介さず静的にアクセス可能で、JsonFileHandlerを使用して名前を永続化します。
 */
class PlayerNameManager : public Component {
	C_REFLECT(PlayerNameManager)

public:
	// --- 静的インターフェース ---
	static bool SetName(const std::wstring& name);
	static std::wstring GetName();

	// --- コンポーネントオーバーライド ---
	void Start() override;
	void Update(float elapsedTime) override;

private:
	static std::wstring s_playerName;
	static const int MAX_NAME_LENGTH = 12;

	// セーブ・ロードの内部処理
	static void Save();
	static void Load();

	// 保存パス（Assetフォルダ内など、環境に合わせて変更してください）
	static const std::string SAVE_FILE_PATH;

	// --- 文字列変換ユーティリティ (Windows API 使用) ---
	static std::wstring Utf8ToWstring(const std::string& utf8Str);
	static std::string WstringToUtf8(const std::wstring& wstr);

};