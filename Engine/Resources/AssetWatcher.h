#pragma once
#include <filesystem>
#include <atomic>
#include <thread>
#include <mutex>
#include <optional>

namespace CurryEngine
{
	/**
	 * @brief ファイルの変更アクションを表す構造体。
	 * @details この構造体は、ファイルの変更イベントを表すために使用されます。変更アクションの種類と、変更されたファイルのパスを保持します。
	 */
	struct FileAction
	{
		DWORD action; // ファイルの変更アクション（作成、削除、変更など）
		std::filesystem::path path; // 変更されたファイルのパス
	};

	/**
	 * @brief アセットの監視を行うクラス。
	 */
	class AssetWatcher
	{
	public:
		/**
		 * @brief 指定されたディレクトリを監視し、アセットの変更を検出する関数。
		 * @param watchDir 監視するディレクトリのパス。
		 */
		void Start(const std::filesystem::path& watchDir);

		/**
		 * @brief 監視を停止する関数。
		 */
		void Stop();

		/**
		 * @brief 監視中に発生したファイルの変更イベントを処理する関数。
		 */
		void ProcessPendingEvents();

	private:
		/**
		 * @brief 監視スレッドを実行する関数。
		 */
		void WatchLoop();

		/**
		 * @brief ファイルの変更イベントを処理する関数。
		 * @param action ファイルの変更アクション（作成、削除、変更など）。
		 * @param path 変更されたファイルのパス。
		 */
		void OnFileAction(DWORD action, const std::filesystem::path& path);

		/**
		 * @brief 監視対象のディレクトリのパス。
		 */
		std::filesystem::path m_watchDir;
		/**
		 * @brief 監視スレッド。
		 */
		std::thread m_watchThread;
		/**
		 * @brief 監視スレッドの状態を示すフラグ。
		 */
		std::atomic<bool> m_running = false;

		/**
		 * @brief リネームが保留されている場合の旧パス。
		 */
		std::optional<std::filesystem::path> m_pendingRenameOldPath;


		std::mutex m_queueMutex; ///< ファイルアクションキューの保護用ミューテックス
		std::vector<FileAction> m_pendingEvents; ///< 保留中のファイルアクションイベントのキュー
	};
}
