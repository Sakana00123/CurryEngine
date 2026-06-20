#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <optional>

namespace CurryEngine
{
	class AssetWatcher
	{
	public:
		/**
		 * @brief 指定されたディレクトリを監視し、アセットの変更を検出する関数。
		 * @param watchDir 監視するディレクトリのパス。
		 */
		void Start(const std::string& watchDir);

		/**
		 * @brief 監視を停止する関数。
		 */
		void Stop();

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
		void OnFileAction(DWORD action, const std::string& path);

		/**
		 * @brief 監視対象のディレクトリのパス。
		 */
		std::string m_watchDir;
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
		std::optional<std::string> m_pendingRenameOldPath;
	};
}