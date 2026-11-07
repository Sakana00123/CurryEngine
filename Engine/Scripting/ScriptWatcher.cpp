#include "pch.h"
#include "ScriptWatcher.h"
#include "Engine/Core/EnginePaths.h"
#include "Engine/Editor/Console.h"
#include "Engine/Scripting/ScriptSystem.h"
#include "Engine/Utils/stdUtiles.h"
#include <Windows.h>
#include <filesystem>


bool ScriptWatcher::Start(
	const fs::path& watchDir,
	const std::vector<BuildCommand>& buildCommands,
	ReloadCallback onReloaded)
{
	if (m_running) {
		LOG_WARNING("[ScriptWatcher] Already running.");
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
	LOG_INFO("[ScriptWatcher] Stopped.");
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
		LOG_ERROR(buf);
		return;
	}

	alignas(DWORD) char buffer[4096]{}; // FILE_NOTIFY_INFORMATION 構造体が複数入る可能性があるため、十分なサイズを確保
	DWORD bytesReturned = 0;
	OVERLAPPED overlapped = {};
	overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (overlapped.hEvent == nullptr) {
		LOG_ERROR("[ScriptWatcher] Failed to create event for overlapped I/O.");
		CloseHandle(hDir);
		return;
	}

	// .cs で終わる かつ ~ で始まらない かつ . で始まらない
	auto IsValidCsFile = [](const std::filesystem::path& relativePath) -> bool {
		std::wstring filePath = relativePath.wstring();
		// ビルド成果物ディレクトリを除外
		static const std::vector<std::wstring> excludedDirs = {
			L"bin\\", L"obj\\", L".vs\\", L".git\\", L"Debug\\", L"Release\\"
		};
		for (const auto& exDir : excludedDirs) {
			if (filePath.find(exDir) != std::wstring::npos) {
				return false; // 除外ディレクトリがパスに含まれている場合は無効
			}
		}
		// 一時ファイルを除外
		if (filePath.starts_with(L"~"))  return false;
		if (filePath.starts_with(L"."))  return false;
		if (filePath.ends_with(L".tmp")) return false;
		if (filePath.ends_with(L".TMP")) return false;
		if (filePath.find(L"~RF") != std::wstring::npos) return false;

		// .cs ファイルのみ対象
		return relativePath.extension() == L".cs";
		};

	// 監視ループ
	while (m_running) {
		// 非同期でディレクトリの変更を監視
		bool result = ReadDirectoryChangesW(
			hDir,
			buffer,
			sizeof(buffer),
			TRUE,
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
			&bytesReturned,
			&overlapped,
			NULL);

		// ReadDirectoryChangesW が失敗した場合はループを抜ける
		if (!result) {
			LOG_ERROR("[ScriptWatcher] ReadDirectoryChangesW failed.");
			break;
		}

		if (overlapped.hEvent == nullptr) {
			LOG_ERROR("[ScriptWatcher] Failed to create event for overlapped I/O.");
			break;
		}

		// 変更があったかを待機（タイムアウトは 500ms）
		DWORD waitResult;
        do { waitResult = WaitForSingleObject(overlapped.hEvent, 100); }
        while (waitResult == WAIT_TIMEOUT && m_running);
        if (!m_running || waitResult != WAIT_OBJECT_0) {
            CancelIoEx(hDir, &overlapped);
            GetOverlappedResult(hDir, &overlapped, &bytesReturned, TRUE);
            break;
        }
        if (!GetOverlappedResult(hDir, &overlapped, &bytesReturned, FALSE)) break;
        if (bytesReturned == 0) { ResetEvent(overlapped.hEvent); RequestBuild(); continue; }

		if (waitResult == WAIT_OBJECT_0) {
			ResetEvent(overlapped.hEvent);

			auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
			while (true)
			{
				std::wstring fileName(info->FileName, info->FileNameLength / sizeof(WCHAR));
				std::filesystem::path relativePath(fileName);

				if (IsValidCsFile(relativePath)) {
					//LOG_INFO(u8"[ScriptWatcher] 変更検知: "
					//	+ relativePath.u8string());
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
			LOG_ERROR(u8"[ScriptWatcher] ビルドに失敗しました。スクリプトはリロードされません。");
			continue;
		}

		// ビルド成功時はスクリプトをリロード
		//LOG_INFO(u8"[ScriptWatcher] ビルド成功。スクリプトをリロードします。");
		if (m_running) m_onReloaded();
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
    // Invoke dotnet directly: project paths containing %, &, etc. must not pass through cmd.exe.
    auto dotnetPath = EnginePaths::EngineRoot() / L"DotNet/dotnet.exe";
    if (!std::filesystem::is_directory(EnginePaths::EngineRoot() / L"DotNet")) {
        wchar_t installed[MAX_PATH];
        const auto count = SearchPathW(nullptr, L"dotnet.exe", nullptr, MAX_PATH, installed, nullptr);
        if (!count || count >= MAX_PATH) { LOG_ERROR("dotnet SDK was not found."); return false; }
        dotnetPath = installed;
    }
    const auto dotnet = dotnetPath.wstring();
    if (command.configuration != L"Debug" && command.configuration != L"Release") return false;
    std::wstring cmd = L"\"" + std::wstring(dotnet) + L"\" build \"" + command.projectPath.wstring() +
        L"\" -c " + command.configuration + L" --nologo -v minimal";
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE readPipe = nullptr, writePipe = nullptr;
    if (!CreatePipe(&readPipe, &writePipe, &security, 0)) return false;
    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = writePipe;
    startup.hStdError = writePipe;
    startup.hStdInput = nullptr;
    PROCESS_INFORMATION process{};
    const auto workingDirectory = command.projectPath.parent_path().wstring();
    const BOOL started = CreateProcessW(dotnet.c_str(), cmd.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, workingDirectory.c_str(), &startup, &process);
    CloseHandle(writePipe);
    if (!started) { CloseHandle(readPipe); LOG_ERROR("Failed to start dotnet build."); return false; }
	char buf[4096];
	DWORD n = 0;
	std::string pending;
	while (ReadFile(readPipe, buf, sizeof(buf), &n, nullptr) && n) {
		pending.append(buf, n);
		size_t pos;
		while ((pos = pending.find('\n')) != std::string::npos) {
			std::string line = pending.substr(0, pos);
			if (!line.empty() && line.back() == '\r') line.pop_back();
			pending.erase(0, pos + 1);
			std::wstring wline = StringToWstring(line);
			//if (wline.find(L"error") != std::wstring::npos) {
			//	LOG_ERROR("[ScriptWatcher] " + line);
			//} else if (wline.find(L"warning") != std::wstring::npos) {
			//	LOG_WARNING("[ScriptWatcher] " + line);
			//} else {
			//	LOG_INFO("[ScriptWatcher] " + line);
			//}
			OutputDebugStringW((L"[ScriptWatcher] " + wline).c_str());
			OutputDebugStringW(L"\n");
		}
	}
    CloseHandle(readPipe);
    WaitForSingleObject(process.hProcess, INFINITE);
    DWORD code = 1;
    GetExitCodeProcess(process.hProcess, &code);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
	LOG_INFO(std::format("[ScriptWatcher] Build finished for {} (exit code: {})", command.projectPath.string(), code));
    return code == 0;
}
