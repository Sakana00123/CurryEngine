#pragma once
#include <string>

/** @brief ProjectSettingsData を表す構造体です。 */
struct ProjectSettingsData
{
	std::string projectName; // プロジェクト名
	std::string companyName; // 会社名
	std::string version; // バージョン
	std::string author; // 作者名
	std::string description; // プロジェクトの説明

	// その他のプロジェクト設定項目をここに追加可能
	std::string scriptProjectPath; // .csprojのパス
	std::string scriptOutputPath; // 出力されるDLLのパス
	std::string scriptWatchDirectory; // スクリプトの監視ディレクトリ

};

/** @brief ProjectSettings を表すクラスです。 */
class ProjectSettings
{
public:
	/**
	 * @brief Load に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	static bool Load(const std::string& exeDir);
	/**
	 * @brief Save の処理を行います。
	 */
	//static bool Save(const std::string& filePath);
	static const ProjectSettingsData& Get() { return s_data; }
	//static void Set(const ProjectSettingsData& data) { s_data = data; }

private:
	static inline ProjectSettingsData s_data;
	static inline std::string s_filePath;
};