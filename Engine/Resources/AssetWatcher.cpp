#include "pch.h"
#include "AssetWatcher.h"
#include "AssetDatabase.h"
#include <Windows.h>
#include <filesystem>
#include <Engine\Editor\AssetBrowser.h>

namespace CurryEngine
{
	void AssetWatcher::Start(const std::filesystem::path& watchDir)
	{
		m_watchDir = watchDir;
		m_running = true;
		m_watchThread = std::thread(&AssetWatcher::WatchLoop, this);
	}

	void AssetWatcher::Stop()
	{
		m_running = false;
		if (m_watchThread.joinable())
		{
			m_watchThread.join();
		}
	}

	void AssetWatcher::WatchLoop()
	{
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

		if (hDir == INVALID_HANDLE_VALUE)
		{
			DWORD err = GetLastError();
			char buf[256];
			sprintf_s(buf, "[AssetWatcher] Failed to open directory for watching. error=%lu", err);
			OutputDebugStringA(buf);
			LOG_ERROR(buf);
			return;
		}

		alignas(DWORD) char buffer[4096];
		OVERLAPPED overlapped = {};
		overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

		if (overlapped.hEvent == NULL) {
			LOG_ERROR("[AssetWatcher] Failed to create event.");
			CloseHandle(hDir);
			return;
		}

		// 最初のリクエストを発行
		bool isPending = false;

		while (m_running) {

			// まだ非同期リクエストが発行されていなければ発行する
			if (!isPending) {
				ResetEvent(overlapped.hEvent);
				DWORD bytesReturned = 0; // 非同期ではこの変数は使われませんが引数として必要

				BOOL result = ReadDirectoryChangesW(
					hDir,
					buffer,
					sizeof(buffer),
					TRUE,
					FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_CREATION,
					&bytesReturned,
					&overlapped,
					NULL);

				if (!result && GetLastError() != ERROR_IO_PENDING) {
					LOG_ERROR("[AssetWatcher] ReadDirectoryChangesW failed.");
					break;
				}
				isPending = true;
			}

			// 500ms 待機
			DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 500);

			if (waitResult == WAIT_OBJECT_0) {
				// 非同期処理が完了したため、実際に書き込まれたバイト数を取得する
				DWORD bytesTransferred = 0;
				if (GetOverlappedResult(hDir, &overlapped, &bytesTransferred, FALSE)) {

					// 実際にデータが書き込まれていればパースする
					if (bytesTransferred > 0) {
						auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
						while (true)
						{
							if (info == nullptr) break;
							if (info->FileNameLength == 0) break;

							std::wstring relativePath(info->FileName, info->FileNameLength / sizeof(wchar_t));
							std::filesystem::path fullPath = std::filesystem::path(m_watchDir) / relativePath;

							OnFileAction(info->Action, fullPath);

							if (info->NextEntryOffset == 0) break;
							info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
								reinterpret_cast<char*>(info) + info->NextEntryOffset);
						}
					}
				}
				else {
					DWORD err = GetLastError();
					// エラーハンドリング（バッファオーバーフローなど）
					if (err == ERROR_NOTIFY_ENUM_DIR) {
						LOG_WARNING("[AssetWatcher] Buffer overflow. Some changes might be lost.");
					}
				}

				// 処理が完了したので、次のループで新しいリクエストを発行できるようにフラグを下げる
				isPending = false;
			}
			else if (waitResult == WAIT_TIMEOUT) {
				// タイムアウトした場合は、リクエストが完了していないので
				// ReadDirectoryChangesW を再発行せず、そのまま次の待機に進む (isPending = true のまま)
				continue;
			}
			else {
				// 何らかのエラー
				DWORD err = GetLastError();
				char buf[256];
				sprintf_s(buf, "[AssetWatcher] WaitForSingleObject failed. error=%lu", err);
				OutputDebugStringA(buf);
				LOG_ERROR(buf);
				break;
			}
		}

		// スレッドが終了する際、未完了の非同期入出力をキャンセルする
		CancelIo(hDir);

		if (overlapped.hEvent != NULL) {
			CloseHandle(overlapped.hEvent);
		}
		CloseHandle(hDir);
	}

	void AssetWatcher::OnFileAction(DWORD action, const std::filesystem::path& path)
	{
		// ここでファイルの変更イベントを処理する
		std::filesystem::path replacedPath = path;

		if (std::filesystem::path(replacedPath).extension() == ".meta") {
			// .metaファイルの変更は無視する
			return;
		}

		switch (action)
		{
		case FILE_ACTION_ADDED:
		{
			LOG_INFO(u8"[AssetWatcher] File added: " + replacedPath.u8string());
			CurryEngine::Resources::AssetDatabase::Import(replacedPath); // 新しいアセットをインポート
			break;
		}
		case FILE_ACTION_REMOVED:
			LOG_INFO(u8"[AssetWatcher] File removed: " + replacedPath.u8string());
			CurryEngine::Resources::AssetDatabase::RemoveByPath(replacedPath); // アセットを削除
			break;
		case FILE_ACTION_MODIFIED:
			LOG_INFO(u8"[AssetWatcher] File modified: " + replacedPath.u8string());
			AssetBrowser::Refresh(); // アセットブラウザをリフレッシュして変更を反映
			break;
		case FILE_ACTION_RENAMED_OLD_NAME:
		{
			LOG_INFO(u8"[AssetWatcher] File renamed (old name): " + replacedPath.u8string());
			m_pendingRenameOldPath = replacedPath; // リネームの旧パスを保留
			break;
		}
		case FILE_ACTION_RENAMED_NEW_NAME:
			if (m_pendingRenameOldPath.has_value()) {
				LOG_INFO(u8"[AssetWatcher] File renamed from " + m_pendingRenameOldPath.value().u8string() + u8" to " + replacedPath.u8string());
				if (std::filesystem::is_regular_file(replacedPath))
				{
					CurryEngine::Resources::AssetDatabase::Rename(m_pendingRenameOldPath.value(), replacedPath); // アセットをリネーム
				}
				else if (std::filesystem::is_directory(replacedPath))
				{
					CurryEngine::Resources::AssetDatabase::RemapPathPrefix(m_pendingRenameOldPath.value(), replacedPath); // フォルダをリネーム
				}
				m_pendingRenameOldPath.reset(); // 保留をクリア
			}
			else {
				LOG_WARNING(u8"[AssetWatcher] Received new name without old name: " + replacedPath.u8string());
			}
			break;
		default:
			LOG_WARNING("[AssetWatcher] Unknown file action: " + std::to_string(action));
			break;
		}
	}
}