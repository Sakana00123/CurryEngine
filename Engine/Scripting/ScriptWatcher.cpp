#include "pch.h"
#include "ScriptWatcher.h"
#include "Engine/Editor/Console.h"
#include "Engine/Scripting/ScriptSystem.h"
#include <Windows.h>
#include <filesystem>


bool ScriptWatcher::Start(
	const std::string& watchDir,
	const std::vector<BuildCommand>& buildCommands,
	ReloadCallback onReloaded)
{
	if (m_running) {
		Console::LogWarning("[ScriptWatcher] Already running.");
		return false;
	}
	m_watchDir = watchDir;
	m_buildCommands = buildCommands;
	m_onReloaded = onReloaded;
	m_running = true;
	m_watchThread = std::thread(&ScriptWatcher::WatchLoop, this);
	m_buildThread = std::thread(&ScriptWatcher::BuildLoop, this);
	return true;
}

void ScriptWatcher::Stop()
{
	if (!m_running) return;
	m_running = false;
	m_buildCv.notify_all(); // ビルドスレッドが待機している可能性があるので通知して起こす
	if (m_buildThread.joinable()) m_buildThread.join();
	if (m_watchThread.joinable()) m_watchThread.join();
	Console::Log("[ScriptWatcher] Stopped.");
}

void ScriptWatcher::WatchLoop()
{
	// まずパスを確認
	std::filesystem::path dirPath(m_watchDir);
	std::wstring watchDirW = dirPath.wstring();

	HANDLE hDir = CreateFileW(
		watchDirW.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		nullptr);

	if (hDir == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		char buf[256];
		sprintf_s(buf, "[ScriptWatcher] Failed error=%lu",
			err);
		Console::LogError(buf);
		return;
	}

	alignas(DWORD) char buffer[4096]; // FILE_NOTIFY_INFORMATION 構造体が複数入る可能性があるため、十分なサイズを確保
	DWORD bytesReturned = 0;
	OVERLAPPED overlapped = {};
	overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	// .cs で終わる かつ ~ で始まらない かつ . で始まらない
	auto IsValidCsFile = [](const std::wstring& relativePath) -> bool {
		// ビルド成果物ディレクトリを除外
		static const std::vector<std::wstring> excludedDirs = {
			L"\\bin\\", L"\\obj\\", L"\\.vs\\", L"\\.git\\", L"\\Debug\\", L"\\Release\\"
		};
		for (const auto& exDir : excludedDirs) {
			if (relativePath.find(exDir) != std::wstring::npos) {
				return false; // 除外ディレクトリがパスに含まれている場合は無効
			}
		}
		std::wstring fileName = std::filesystem::path(relativePath).filename().wstring();
		// 一時ファイルを除外
		if (fileName.starts_with(L"~"))  return false;
		if (fileName.starts_with(L"."))  return false;
		if (fileName.ends_with(L".tmp")) return false;
		if (fileName.ends_with(L".TMP")) return false;
		if (fileName.find(L"~RF") != std::wstring::npos) return false;

		// .cs ファイルのみ対象
		return relativePath.ends_with(L".cs");
		};

	// 最後の変更から一定時間待ってからビルド
	auto lastChange = std::chrono::steady_clock::now();

	while (m_running) {
		// 非同期でディレクトリの変更を監視
		ReadDirectoryChangesW(
			hDir,
			buffer,
			sizeof(buffer),
			TRUE,
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
			&bytesReturned,
			&overlapped,
			NULL);

		// 変更があったかを待機（タイムアウトは 500ms）
		DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 500);

		if (waitResult == WAIT_OBJECT_0) {
			ResetEvent(overlapped.hEvent);

			auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
			while (true)
			{
				std::wstring relativePath(info->FileName,
					info->FileNameLength / sizeof(wchar_t));

				if (IsValidCsFile(relativePath)) {
					Console::Log(std::string(reinterpret_cast<const char*>(u8"[ScriptWatcher] 変更検知: "))
						+ std::string(relativePath.begin(), relativePath.end()));
					// 変更時刻を更新してフラグを立てるだけ
					lastChange = std::chrono::steady_clock::now();
					RequestBuild();
				}
				// 次のエントリがなければループを抜ける
				if (info->NextEntryOffset == 0) break;
				// 次のエントリへ
				info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
					reinterpret_cast<char*>(info) + info->NextEntryOffset);
			}
		}
	}
	CloseHandle(overlapped.hEvent);
	CloseHandle(hDir);
}

void ScriptWatcher::BuildLoop()
{
	auto lastChange = std::chrono::steady_clock::now();

	while (m_running)
	{
		// ビルド要求が来るまで待機
		{
			std::unique_lock<std::mutex> lock(m_buildMutex);
			m_buildCv.wait(lock, [this] { return m_pendingBuild.load() || !m_running.load(); });
		}

		if (!m_running) break;

		// 変更から一定時間待ってからビルド
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		m_pendingBuild = false;
		
		// ビルドを実行
		if (!BuildProjects()) {
			Console::LogError(u8"[ScriptWatcher] ビルドに失敗しました。スクリプトはリロードされません。");
			continue;
		}

		// ビルド成功時はスクリプトをリロード
		Console::Log(u8"[ScriptWatcher] ビルド成功。スクリプトをリロードします。");
		m_onReloaded();
	}
}

bool ScriptWatcher::BuildProjects()
{
	bool allSuccess = true;
	for (const auto& command : m_buildCommands) {
		if (!BuildProject(command)) {
			allSuccess = false;
		}
	}
	return allSuccess;
}

bool ScriptWatcher::BuildProject(const BuildCommand& command)
{
	Console::Log(u8"[ScriptWatcher] Building project: " + command.projectPath.u8string());

	std::string cmd = "dotnet build \"" + command.projectPath.string()
		+ "\" " + command.additionalArgs;

	FILE* pipe = _popen(cmd.c_str(), "r");
	if (!pipe) {
		LOG_ERROR(u8"[ScriptWatcher] ビルドプロセスの起動に失敗");
		return false;
	}

	char line[512];
	bool hasError = false;
	while (fgets(line, sizeof(line), pipe)) {
		std::string s = line;
		if (!s.empty() && s.back() == '\n') s.pop_back();
		// ビルド出力を解析してエラーや警告をログに出す
		if (s.find("error") != std::string::npos ||
			s.find("Error") != std::string::npos) {
			Console::LogError("[Build] " + s);
			hasError = true;
		}
		else if (s.find("warning") != std::string::npos ||
			s.find("Warning") != std::string::npos) {
			Console::LogWarning("[Build] " + s);
		}
		else if (!s.empty()) {
			Console::Log("[Build] " + s);
		}
	}

	int exitCode = _pclose(pipe);
	return exitCode == 0 && !hasError;
}
