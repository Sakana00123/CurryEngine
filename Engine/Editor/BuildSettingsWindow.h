#pragma once
#include "Engine/Core/EnginePaths.h"
#include "Engine/BuildSettings.h"
#include <vector>
#include <mutex>

class BuildSettingsWindow
{
public:

	static BuildSettingsWindow& Get() {
		static BuildSettingsWindow instance;
		return instance;
	}


	/** @brief ウィンドウを表示します。*/
	void Show();

	/** @brief GUI を描画します。*/
	void DrawGUI();

private:

	// --- Build/Package 進捗 ---
	struct ProcessProgress
	{
		std::vector<std::string> logs;
		int  progressCurrent = 0;
		int  progressTotal = 0;
		bool finished = false;
		bool succeeded = false;
	};

	std::mutex      m_progressMutex;
	ProcessProgress m_buildProgress;    // RunBuild() 用
	ProcessProgress m_packageProgress;  // PackageBuildOutput() 用

#ifdef USE_IMGUI

	/** @brief 設定を描画します。*/
	void DrawSettings();

	/** @brief ビルドに含めるシーンのセレクタを描画します。*/
	void DrawScenesInBuild(const char* label);

	/** @brief コピーするファイルのセレクタを描画します。*/
	bool DrawFileSelector(const char* label, std::string& path, const char* filter = "*.*");

	/** @brief ディレクトリセレクタを描画します。*/
	bool DrawDirectorySelector(const char* label, std::string& path);

	/** @brief ビルドボタンを描画します。*/
	void DrawBuildButton();

	/** @brief ビルド/パッケージ進捗パネルを描画します。*/
	void DrawProgressPanel(const char* id, const ProcessProgress& prog, bool isRunning);

#endif // USE_IMGUI

	/** @brief ビルド設定を保存します。*/
	void SaveBuildSettings();
	/** @brief ビルド設定を読み込みます。*/
	void LoadBuildSettings();


	/** @brief ビルドを実行します。*/
	void RunBuild();

	/** @brief ビルド出力をパッケージ化します。*/
	void PackageBuildOutput();


	bool m_showWindow = false; //!< ウィンドウの表示フラグ
	BuildSettings m_settings; //!< 現在のビルド設定
	bool m_isBuilding = false; //!< ビルド中かどうかのフラグ
	bool m_isPackaging = false; //!< パッケージ化中かどうかのフラグ

	std::string appName; //!< アプリケーション名のキャッシュ（ImGui の InputText 用）
	std::string iconPath; //!< アイコンパスのキャッシュ（ImGui の InputText 用）
	std::string outputDir; //!< 出力ディレクトリのキャッシュ（ImGui の InputText 用）

};