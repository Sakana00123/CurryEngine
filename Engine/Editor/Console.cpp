#include "pch.h"
#include "Console.h"

// OpenInVisualStudio で使用
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>   // SHGetFolderPathW
#include <thread>     // OpenInVisualStudio の非同期化
#include <mutex>      // std::once_flag / std::call_once
#pragma comment(lib, "shell32.lib")
#endif

std::vector<Console::LogEntry> Console::logs;
bool Console::isOpen = true;
bool Console::isLogSizeIncreasedInCurrentFrame = false;
uint8_t Console::s_logLevelFlags = 0b111; // デフォルトは全レベル表示
char Console::s_filterBuffer[256] = {};

static std::mutex s_logMutex;

void Console::Initialize()
{
	ClearLog();
	Show();
}

void Console::Log(const std::string& message, const std::string& file, int line) {
	std::lock_guard<std::mutex> lock(s_logMutex); // ログの追加は排他制御する
#ifdef USE_IMGUI
	logs.push_back({ LogLevel::Info, message, file, line });
	isLogSizeIncreasedInCurrentFrame = true;
#endif // USE_IMGUI
}

void Console::Log(const std::u8string& message, const std::string& file, int line) {
	std::lock_guard<std::mutex> lock(s_logMutex); // ログの追加は排他制御する
#ifdef USE_IMGUI
	logs.push_back({ LogLevel::Info, reinterpret_cast<const char*>(message.c_str()), file, line });
	isLogSizeIncreasedInCurrentFrame = true;
#endif // USE_IMGUI
}

void Console::LogWarning(const std::string& message, const std::string& file, int line) {
	std::lock_guard<std::mutex> lock(s_logMutex); // ログの追加は排他制御する
#ifdef USE_IMGUI
	logs.push_back({ LogLevel::Warning, message, file, line });
	isLogSizeIncreasedInCurrentFrame = true;
#endif // USE_IMGUI
}

void Console::LogWarning(const std::u8string& message, const std::string& file, int line) {
	std::lock_guard<std::mutex> lock(s_logMutex); // ログの追加は排他制御する
#ifdef USE_IMGUI
	logs.push_back({ LogLevel::Warning, reinterpret_cast<const char*>(message.c_str()), file, line });
	isLogSizeIncreasedInCurrentFrame = true;
#endif // USE_IMGUI
}

void Console::LogError(const std::string& message, const std::string& file, int line) {
	std::lock_guard<std::mutex> lock(s_logMutex); // ログの追加は排他制御する
#ifdef USE_IMGUI
	logs.push_back({ LogLevel::Error, message, file, line });
	isLogSizeIncreasedInCurrentFrame = true;
#endif // USE_IMGUI
}

void Console::LogError(const std::u8string& message, const std::string& file, int line) {
	std::lock_guard<std::mutex> lock(s_logMutex); // ログの追加は排他制御する
#ifdef USE_IMGUI
	logs.push_back({ LogLevel::Error, reinterpret_cast<const char*>(message.c_str()), file, line });
	isLogSizeIncreasedInCurrentFrame = true;
#endif // USE_IMGUI
}

void Console::CustomLog(LogLevel level, const std::string& message, const std::string& file, int line)
{
	std::lock_guard<std::mutex> lock(s_logMutex); // ログの追加は排他制御する
#ifdef USE_IMGUI
	logs.push_back({ level,message,file,line });
	isLogSizeIncreasedInCurrentFrame = true;
#endif // USE_IMGUI
}

void Console::CustomLog(LogLevel level, const std::u8string& message, const std::string& file, int line)
{
	std::lock_guard<std::mutex> lock(s_logMutex); // ログの追加は排他制御する
#ifdef USE_IMGUI
	logs.push_back({ level, reinterpret_cast<const char*>(message.c_str()), file, line });
	isLogSizeIncreasedInCurrentFrame = true;
#endif // USE_IMGUI
}

void Console::ClearLog() {
	logs.clear();
}

void Console::Show() {
	isOpen = true;
}

void Console::Shutdown() {
	ClearLog();
}

// ---------------------------------------------------------------------------
//  OpenInVisualStudio
//    vswhere で devenv.exe のパスを取得し、
//    /Edit <file> /command "Edit.GoTo <line>" で該当行を開く。
//    日本語パスに対応するため、全処理を UTF-16 (W 系 API) で行う。
// ---------------------------------------------------------------------------

// UTF-8 std::string → UTF-16 std::wstring 変換ヘルパー
static std::wstring Utf8ToWide(const std::string& utf8)
{
	if (utf8.empty()) return {};
	int wlen = MultiByteToWideChar(CP_UTF8, 0,
		utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
	std::wstring result(wlen, L'\0');
	MultiByteToWideChar(CP_UTF8, 0,
		utf8.c_str(), static_cast<int>(utf8.size()), result.data(), wlen);
	return result;
}

// UTF-16 std::wstring → UTF-8 std::string 変換ヘルパー
static std::string WideToUtf8(const std::wstring& wide)
{
	if (wide.empty()) return {};
	int len = WideCharToMultiByte(CP_UTF8, 0,
		wide.c_str(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
	std::string result(len, '\0');
	WideCharToMultiByte(CP_UTF8, 0,
		wide.c_str(), static_cast<int>(wide.size()), result.data(), len, nullptr, nullptr);
	return result;
}

void Console::OpenInVisualStudio(const std::string& file, int line)
{
	if (file.empty() || line <= 0) return;

	// vswhere の検索・ShellExecuteW はブロッキング処理になるため
	// 別スレッドで実行して UI スレッドを止めない。
	// detach() でスレッドを切り離し、完了を待たずに戻る。
	std::thread([file, line]()
		{
#ifdef _WIN32

			// --- 1. devenvPath のキャッシュ ---
			//        call_once により初回のみ vswhere を実行し、以降は即座に返す。
			//        複数スレッドが同時に呼んでも once_flag が排他を保証する。
			static std::wstring s_devenvPath;
			static std::once_flag s_devenvOnce;

			std::call_once(s_devenvOnce, []()
				{
					wchar_t progFilesX86[MAX_PATH] = {};
					if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILESX86, nullptr, 0, progFilesX86)))
					{
						std::wstring vswhere = std::wstring(progFilesX86)
							+ L"\\Microsoft Visual Studio\\Installer\\vswhere.exe";

						if (GetFileAttributesW(vswhere.c_str()) != INVALID_FILE_ATTRIBUTES)
						{
							std::wstring cmd = L"\"" + vswhere + L"\""
								L" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop"
								L" -property installationPath";

							FILE* pipe = _wpopen(cmd.c_str(), L"r");
							if (pipe)
							{
								char buf[MAX_PATH * 3] = {};
								if (fgets(buf, sizeof(buf), pipe))
								{
									std::string installPathUtf8(buf);
									while (!installPathUtf8.empty() &&
										(installPathUtf8.back() == '\n' || installPathUtf8.back() == '\r'))
									{
										installPathUtf8.pop_back();
									}
									s_devenvPath = Utf8ToWide(installPathUtf8) + L"\\Common7\\IDE\\devenv.exe";
								}
								_pclose(pipe);
							}
						}
					}

					// vswhere が使えなかった場合は PATH 上の devenv を使う
					if (s_devenvPath.empty() ||
						GetFileAttributesW(s_devenvPath.c_str()) == INVALID_FILE_ATTRIBUTES)
					{
						s_devenvPath = L"devenv";
					}
				});

			const std::wstring& devenvPath = s_devenvPath;

			// --- 2. __FILE__ のエンコーディングを判定して wstring に変換 ---
			//        /utf-8 コンパイルオプションが付いている → UTF-8 として変換
			//        付いていない環境 (CP_ACP) のフォールバックも用意する
			std::wstring wfile;
			{
				// まず UTF-8 として変換を試みる
				int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
					file.c_str(), static_cast<int>(file.size()), nullptr, 0);
				if (wlen > 0)
				{
					wfile.resize(wlen);
					MultiByteToWideChar(CP_UTF8, 0,
						file.c_str(), static_cast<int>(file.size()), wfile.data(), wlen);
				}
				else
				{
					// UTF-8 として不正なバイト列 → システムのデフォルトコードページで変換
					wlen = MultiByteToWideChar(CP_ACP, 0,
						file.c_str(), static_cast<int>(file.size()), nullptr, 0);
					wfile.resize(wlen);
					MultiByteToWideChar(CP_ACP, 0,
						file.c_str(), static_cast<int>(file.size()), wfile.data(), wlen);
				}
			}

			// --- 3. devenv /Edit <file> /command "Edit.GoTo <line>" を CreateProcess で実行 ---
			//
			//   ShellExecuteW はコマンドライン文字列を内部でパースするため、
			//   クォートや (line) サフィックスが誤解釈される。
			//   CreateProcessW は lpCommandLine をそのまま渡すため確実。
			//
			//   コマンドライン形式:
			//     "devenv.exe" /Edit "file.cs" /command "Edit.GoTo line"
			//
			//   /Edit    : 既存インスタンスがあればそこへ渡す（なければ新規起動）
			//   Edit.GoTo: VS の IDE コマンド。行番号を受け取りカーソルをジャンプさせる
			{
				// CreateProcessW の lpCommandLine は先頭にも実行ファイルパスが必要
				std::wstring cmdLine =
					L"\"" + devenvPath + L"\""
					+ L" /Edit "
					+ L"\"" + wfile + L"\""
					+ L" /command \"Edit.GoTo "
					+ std::to_wstring(line)
					+ L"\"";

				STARTUPINFOW si = {};
				si.cb = sizeof(si);
				PROCESS_INFORMATION pi = {};

				// lpCommandLine は書き換えられる可能性があるため非 const バッファに渡す
				std::vector<wchar_t> cmdBuf(cmdLine.begin(), cmdLine.end());
				cmdBuf.push_back(L'\0');

				CreateProcessW(
					nullptr,        // lpApplicationName  (cmdLine 先頭から取得)
					cmdBuf.data(),  // lpCommandLine
					nullptr,        // lpProcessAttributes
					nullptr,        // lpThreadAttributes
					FALSE,          // bInheritHandles
					0,              // dwCreationFlags
					nullptr,        // lpEnvironment
					nullptr,        // lpCurrentDirectory
					&si,
					&pi
				);

				// ハンドルは不要なので即閉じる
				if (pi.hProcess) CloseHandle(pi.hProcess);
				if (pi.hThread)  CloseHandle(pi.hThread);
			}
#endif // _WIN32
		}).detach();
}

// ---------------------------------------------------------------------------
//  DrawGUI
// ---------------------------------------------------------------------------
void Console::DrawGUI()
{
#ifdef USE_IMGUI
	std::lock_guard<std::mutex> lock(s_logMutex);
	ZoneScopedN("Console::DrawGUI");
	ImGui::Begin("Console", &isOpen);

	// ---- クリアボタン ----
	if (ImGui::Button("Clear"))
	{
		ClearLog();
	}

	// --- レベル別カウント ---
	int countInfo = 0, countWarning = 0, countError = 0;
	for (const LogEntry& e : logs)
	{
		if (e.level == LogLevel::Info)    ++countInfo;
		else if (e.level == LogLevel::Warning) ++countWarning;
		else if (e.level == LogLevel::Error)   ++countError;
	}

	// --- Info トグル ---
	ImGui::SameLine();
	{
		if (s_logLevelFlags & (1 << static_cast<int>(LogLevel::Info)))
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		else
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
		ImVec4 textColor = (s_logLevelFlags & (1 << static_cast<int>(LogLevel::Info))) ?
			ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);
		std::string labelInfo = "Info " + std::to_string(countInfo);
		if (ImGui::Button(labelInfo.c_str()))
			s_logLevelFlags ^= (1 << static_cast<int>(LogLevel::Info)); // トグル
		ImGui::PopStyleColor(2);
	}

	// --- Warning トグル ---
	ImGui::SameLine();
	{
		if (s_logLevelFlags & (1 << static_cast<int>(LogLevel::Warning)))
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.12f, 1.0f));
		else
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
		ImVec4 textColor = (s_logLevelFlags & (1 << static_cast<int>(LogLevel::Warning))) ?
			ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.6f, 0.6f, 0.0f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);
		std::string labelWarn = "Warning " + std::to_string(countWarning);
		if (ImGui::Button(labelWarn.c_str()))
			s_logLevelFlags ^= (1 << static_cast<int>(LogLevel::Warning)); // トグル
		ImGui::PopStyleColor(2);
	}

	// --- Error トグル ---
	ImGui::SameLine();
	{
		if (s_logLevelFlags & (1 << static_cast<int>(LogLevel::Error)))
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.10f, 0.10f, 1.0f));
		else
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
		ImVec4 textColor = (s_logLevelFlags & (1 << static_cast<int>(LogLevel::Error))) ?
			ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.6f, 0.3f, 0.3f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);
		std::string labelErr = "Error " + std::to_string(countError);
		if (ImGui::Button(labelErr.c_str()))
			s_logLevelFlags ^= (1 << static_cast<int>(LogLevel::Error)); // トグル
		ImGui::PopStyleColor(2);
	}

	// --- 検索ボックス（残り幅いっぱいに広げる） ---
	ImGui::SameLine();
	ImGui::SetNextItemWidth(-1.0f);
	ImGui::InputText("##search", s_filterBuffer, sizeof(s_filterBuffer));

	ImGui::BeginChild("Logs", ImVec2(0, 0), ImGuiChildFlags_Border);

	if (!logs.empty())
	{
		// ログの最大数を制限
		{
			const size_t maxLogs = 1000;
			if (logs.size() > maxLogs)
			{
				logs.erase(logs.begin(), logs.begin() + (logs.size() - maxLogs));
			}
		}

		// ログ表示
		for (size_t i = 0; i < logs.size(); ++i)
		{
			const LogEntry& entry = logs[i]; // 参照のみ
			ImGui::PushID(&entry);

			// --- レベルフィルタリング ---
			bool levelVisible = s_logLevelFlags & (1 << static_cast<int>(entry.level));

			// --- テキストフィルタリング ---
			bool textVisible = true;
			if (s_filterBuffer[0] != '\0')
			{
				std::string filterLower = s_filterBuffer;
				std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
				std::string messageLower = entry.message;
				std::transform(messageLower.begin(), messageLower.end(), messageLower.begin(), ::tolower);
				std::string fileLower = entry.file;
				std::transform(fileLower.begin(), fileLower.end(), fileLower.begin(), ::tolower);
				textVisible = (messageLower.find(filterLower) != std::string::npos) ||
					(fileLower.find(filterLower) != std::string::npos);
			}

			if (!levelVisible || !textVisible) // どちらかの条件を満たさない場合はスキップ
			{
				ImGui::PopID();
				continue;
			}

			// --- 色の設定 ---
			ImVec4 col;
			switch (entry.level) {
			case LogLevel::Info:    col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
			case LogLevel::Warning: col = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); break;
			case LogLevel::Error:   col = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); break;
			default:                col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
			}
			ImGui::PushStyleColor(ImGuiCol_Text, col);

			// --- ファイル名（パスの最後の部分だけ）と行番号を作成 ---
			std::string shortFile;
			if (!entry.file.empty())
			{
				// '\\' と '/' の両方に対応して最後のセパレータ以降を取得
				size_t pos = entry.file.find_last_of("/\\");
				shortFile = (pos != std::string::npos)
					? entry.file.substr(pos + 1)
					: entry.file;
			}

			// --- メッセージ本文の表示 ---
			ImGui::TextUnformatted(entry.message.c_str());

			// シングルクリック：メッセージをクリップボードにコピー（既存動作）
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				ImGui::SetClipboardText(entry.message.c_str());
			}

			// --- ファイル名:行番号 をグレーで右寄せ表示 ---
			if (!shortFile.empty() && entry.line > 0)
			{
				std::string location = shortFile + ":" + std::to_string(entry.line);

				// 右寄せ：ウィンドウ幅からテキスト幅を引いてカーソルを移動
				float textWidth = ImGui::CalcTextSize(location.c_str()).x;
				float windowWidth = ImGui::GetContentRegionAvail().x;
				float cursorX = windowWidth - textWidth;
				if (cursorX > ImGui::GetCursorPosX())
					ImGui::SetCursorPosX(cursorX);

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
				ImGui::TextLink(location.c_str());
				ImGui::PopStyleColor();

				// ダブルクリック：Visual Studio で該当行を開く
				if (ImGui::IsItemHovered() &&
					ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					OpenInVisualStudio(entry.file, entry.line);
				}

				// ホバー時のツールチップ（フルパスを表示）
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Double-click to open in Visual Studio\n%s:%d",
						entry.file.c_str(), entry.line);
				}
			}

			ImGui::PopStyleColor(); // PushStyleColor(ImGuiCol_Text, col)
			ImGui::PopID();

			// ログエントリ間の区切り線（薄め）
			ImGui::Separator();
		}

		// ログが増えたときに最新のログが見えるようにスクロール
		if (isLogSizeIncreasedInCurrentFrame)
		{
			ImGui::SetScrollHereY(1.0f);
			isLogSizeIncreasedInCurrentFrame = false;
		}
	}

	ImGui::EndChild();
	ImGui::End();
#endif // USE_IMGUI
}
