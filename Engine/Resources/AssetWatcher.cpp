#include "pch.h"
#include "AssetWatcher.h"
#include "AssetDatabase.h"
#include <Windows.h>
#include <filesystem>
#include <Engine\Editor\AssetBrowser.h>

namespace CurryEngine
{
	void AssetWatcher::Start(const std::string& watchDir)
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
		// 監視対象のディレクトリのパスを取得
		std::filesystem::path dirPath(m_watchDir);
		std::wstring watchDirW = dirPath.wstring();

		// 監視対象のディレクトリを開く
		HANDLE hDir = CreateFileW(
			watchDirW.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			nullptr);

		// 監視対象のディレクトリが開けなかった場合のエラーハンドリング
		if (hDir == INVALID_HANDLE_VALUE)
		{
			DWORD err = GetLastError();
			char buf[256];
			sprintf_s(buf, "[AssetWatcher] Failed to open directory for watching. error=%lu", err);
			OutputDebugStringA(buf);
			LOG_ERROR(buf);
			return;
		}

		alignas(DWORD) char buffer[4096]; // FILE_NOTIFY_INFORMATION 構造体が複数入る可能性があるため、十分なサイズを確保
		DWORD bytesReturned = 0;
		OVERLAPPED overlapped = {};
		overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

		while (m_running) {
			// 非同期でディレクトリの変更を監視
			ReadDirectoryChangesW(
				hDir,
				buffer,
				sizeof(buffer),
				TRUE,
				FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_CREATION,
				&bytesReturned,
				&overlapped,
				NULL);

			// 変更があったかを待機（タイムアウトは 500ms）
			if (overlapped.hEvent == NULL) {
				LOG_ERROR("[AssetWatcher] Event handle is NULL.");
				break;
			}
			DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 500);

			if (waitResult == WAIT_OBJECT_0) {
				// イベントをリセット
				if (overlapped.hEvent == NULL) {
					LOG_ERROR("[AssetWatcher] Event handle is NULL.");
				}
				else if (!ResetEvent(overlapped.hEvent)) {
					DWORD err = GetLastError();
					char buf[256];
					sprintf_s(buf, "[AssetWatcher] Failed to reset event. error=%lu", err);
					OutputDebugStringA(buf);
					LOG_ERROR(buf);
				}

				auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
				while (true)
				{
					std::wstring relativePath(info->FileName,
						info->FileNameLength / sizeof(wchar_t));
					std::filesystem::path fullPath = std::filesystem::path(m_watchDir) / relativePath;

					// ファイルの変更イベントを処理
					OnFileAction(info->Action, fullPath.string());

					// 次のエントリがなければループを抜ける
					if (info->NextEntryOffset == 0) break;
					// 次のエントリへ
					info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
						reinterpret_cast<char*>(info) + info->NextEntryOffset);
				}
			}
		}
		if (overlapped.hEvent != NULL) {
			CloseHandle(overlapped.hEvent);
		}
		CloseHandle(hDir);
	}

	void AssetWatcher::OnFileAction(DWORD action, const std::string& path)
	{
		// ここでファイルの変更イベントを処理する
		std::string replacedPath = path;
		std::replace(replacedPath.begin(), replacedPath.end(), '\\', '/'); // パスの区切り文字を統一

		switch (action)
		{
		case FILE_ACTION_ADDED:
			LOG_INFO("[AssetWatcher] File added: " + replacedPath);
			CurryEngine::Resources::AssetDatabase::Import(replacedPath); // 新しいアセットをインポート
			break;
		case FILE_ACTION_REMOVED:
			LOG_INFO("[AssetWatcher] File removed: " + replacedPath);
			CurryEngine::Resources::AssetDatabase::RemoveByPath(replacedPath); // アセットを削除
			break;
		case FILE_ACTION_MODIFIED:
			LOG_INFO("[AssetWatcher] File modified: " + replacedPath);
			AssetBrowser::Refresh(); // アセットブラウザをリフレッシュして変更を反映
			break;
		case FILE_ACTION_RENAMED_OLD_NAME:
			LOG_INFO("[AssetWatcher] File renamed (old name): " + replacedPath);
			m_pendingRenameOldPath = replacedPath; // リネームの旧パスを保留
			break;
		case FILE_ACTION_RENAMED_NEW_NAME:
			if (m_pendingRenameOldPath.has_value()) {
				LOG_INFO("[AssetWatcher] File renamed from " + m_pendingRenameOldPath.value() + " to " + replacedPath);
				CurryEngine::Resources::AssetDatabase::Rename(m_pendingRenameOldPath.value(), replacedPath); // アセットをリネーム
				m_pendingRenameOldPath.reset(); // 保留をクリア
			}
			else {
				LOG_WARNING("[AssetWatcher] Received new name without old name: " + replacedPath);
			}
			break;
		default:
			LOG_WARNING("[AssetWatcher] Unknown file action: " + std::to_string(action));
			break;
		}
	}
}