#pragma once
#include <string>
#include <filesystem>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace fs = std::filesystem;

struct BuildCommand
{
	fs::path projectPath; // .csproj ファイルのパス
	std::string additionalArgs = "-c Release --nologo -v q 2>&1"; // 追加のビルド引数（オプション）
};

class ScriptWatcher
{
public:
	using ReloadCallback = std::function<void()>;

	ScriptWatcher() = default;
	~ScriptWatcher() { Stop(); }
	
	/**
	 * @brief スクリプトの変更を監視し、変更があった場合にプロジェクトをビルドしてリロードする機能を開始します。
	 * @param watchDir 監視するディレクトリのパス。
	 * @param csprojPath ビルドに使用する .csproj ファイルのパス。
	 * @param onReloaded スクリプトがリロードされたときに呼び出されるコールバック関数。
	 */
	bool Start(
		const std::string& watchDir,
		const std::vector<BuildCommand>& buildCommands,
		ReloadCallback onReloaded);

	/** @brief スクリプトの監視とリロード機能を停止します。*/
	void Stop();

	/** @brief プロジェクトのビルドを要求します。*/
	void RequestBuild() { m_pendingBuild = true; m_buildCv.notify_one(); }

private:
	// 監視ループとビルド処理
	void WatchLoop();
	void BuildLoop();
	bool BuildProjects();
	bool BuildProject(const BuildCommand& config);

private:
	std::string m_watchDir;
	std::vector<BuildCommand> m_buildCommands;
	ReloadCallback m_onReloaded;

	std::thread m_watchThread;
	std::thread m_buildThread;

	std::atomic_bool m_running{ false };
	std::atomic_bool m_pendingBuild{ false };

	std::mutex m_buildMutex;
	std::condition_variable m_buildCv;
};
