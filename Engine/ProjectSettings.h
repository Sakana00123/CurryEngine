#pragma once
#include <string>

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

class ProjectSettings
{
public:
	static bool Load(const std::string& exeDir);
	//static bool Save(const std::string& filePath);
	static const ProjectSettingsData& Get() { return s_data; }
	//static void Set(const ProjectSettingsData& data) { s_data = data; }

private:
	static inline ProjectSettingsData s_data;
	static inline std::string s_filePath;
};