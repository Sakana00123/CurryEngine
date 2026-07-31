#pragma once
#include <string>
#include <vector>
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

/**
 * @file
 * @brief エディタ内の簡易コンソール（ログ表示）機能。
 * @details 文字列の情報/警告/エラーログを蓄積し、ImGui を用いた表示・クリアなどを提供します。
 */
class Console
{
public:
	/**
	 * @brief ログレベル。
	 */
	enum class LogLevel { Info = 0, Warning = 1, Error = 2 };

	/**
	 * @brief 1 件のログエントリ。
	 */
	struct LogEntry
	{
		LogLevel level;       //!< レベル
		std::string message;  //!< メッセージ本文
		std::string file;      // __FILE__
		int         line;      // __LINE__
	};
private:
	/** @brief 蓄積されたログ。*/
	static std::vector<LogEntry> logs;
	/** @brief コンソールウィンドウが開いているか。*/
	static bool isOpen;
	/** @brief このフレームでログが増えたか（自動スクロール用）。*/
	static bool isLogSizeIncreasedInCurrentFrame;
	/** @brief ログレベルの表示/非表示フラグ。ビット 0: Info, 1: Warning, 2: Error。*/
	static uint8_t s_logLevelFlags;
	/** @brief ログフィルタの入力バッファ。*/
	static char s_filterBuffer[256];
public:
	/** @brief 初期化処理。必要なリソースの確保などを行います。*/
	static void Initialize();

	/** @brief 情報ログを追加します。*/
	static void Log(const std::string& message, const std::string& file = "", int line = -1);
	static void Log(const std::u8string& message, const std::string& file = "", int line = -1);

	/** @brief 警告ログを追加します。*/
	static void LogWarning(const std::string& message, const std::string& file = "", int line = -1);
	static void LogWarning(const std::u8string& message, const std::string& file = "", int line = -1);
	
	/** @brief エラーログを追加します。*/
	static void LogError(const std::string& message, const std::string& file = "", int line = -1);
	static void LogError(const std::u8string& message, const std::string& file = "", int line = -1);

	/** @brief カスタムログを追加します。*/
	static void CustomLog(LogLevel level, const std::string& message, const std::string& file, int line);
	static void CustomLog(LogLevel level, const std::u8string& message, const std::string& file, int line);

	/** @brief ログをすべてクリアします。*/
	static void ClearLog();

	/** @brief コンソールウィンドウを表示します。*/
	static void Show();

	/** @brief 終了処理。必要なリソースを解放します。*/
	static void Shutdown();

	/** @brief Visual Studio で指定したファイルの行を開きます。*/
	static void OpenInVisualStudio(const std::string& file, int line);

	/** @brief ImGui を用いてコンソール GUI を描画します。*/
	static void DrawGUI();
};

// ---- マクロ ----
#ifndef DEFINE_LOG
#define DEFINE_LOG
#define LOG_INFO(msg)    Console::CustomLog(Console::LogLevel::Info,    (msg), __FILE__, __LINE__)
#define LOG_WARNING(msg) Console::CustomLog(Console::LogLevel::Warning, (msg), __FILE__, __LINE__)
#define LOG_ERROR(msg)   Console::CustomLog(Console::LogLevel::Error,   (msg), __FILE__, __LINE__)
#endif // DEFINE_LOG
