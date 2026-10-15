#include "pch.h"
#include "ProjectSettings.h"
#include "Engine/Utils/JsonFileHandler.h"
#include "Engine/Core/EnginePaths.h"


bool ProjectSettings::Load(const std::string& exeDir)
{
	s_data = {};
    if (EnginePaths::IsExternalProject())
    {
        s_data.projectName = EnginePaths::StartupContext.projectName;
        s_data.scriptProjectPath = EnginePaths::ScriptProject();
        s_data.scriptOutputPath = EnginePaths::ScriptAssembly();
        s_data.scriptWatchDirectory = EnginePaths::ProjectRoot() / "Assets";
        return true;
    }
	json data;
	std::string filePath = exeDir + "\\projectSettings.json";
	std::ifstream settingsFile(filePath);
    if (!std::filesystem::exists(filePath)) data = json::object();
	else if (!settingsFile || (data = json::parse(settingsFile, nullptr, false)).is_discarded() || !data.is_object())
	{
		return false;
	}

	s_data.projectName = data.value("projectName", "New Project");
	s_data.companyName = data.value("companyName", "My Company");
	s_data.version = data.value("version", "0.1.0");
	s_data.author = data.value("author", "Author Name");
	s_data.description = data.value("description", "Project Description");

	// デフォルトパスを構築。絶対パスで指定されていない場合は、exe のディレクトリを基準にした相対パスを使用する。

	// ソリューションのディレクトリを取得
	// TODO: ソリューションのディレクトリを自動的に検出するか、プロジェクト設定で指定できるようにする
	std::filesystem::path solutionDir = data.value("solutionDir", std::filesystem::path(exeDir).parent_path().parent_path());

	// ソリューションのディレクトリを基準にした絶対パス
	std::filesystem::path defaultScriptProjectPath = solutionDir / "UserScripts/Assembly-CSharp.csproj";
	std::filesystem::path defaultScriptOutputPath = std::filesystem::path(exeDir) / "Assembly-CSharp.dll";
	//std::filesystem::path defaultScriptOutputPath = solutionDir / "Library/ScriptAssemblies/Assembly-CSharp.dll";
	std::filesystem::path defaultScriptWatchDirectory = solutionDir / "UserScripts/";
	// JSONからパスを取得。絶対パスで指定されていない場合は、exe のディレクトリを基準にした絶対パスを使用する。
	std::filesystem::path scriptProjectPath = data.value("scriptProjectPath", defaultScriptProjectPath.string());
	std::filesystem::path scriptOutputPath = data.value("scriptOutputPath", defaultScriptOutputPath.string());
	std::filesystem::path scriptWatchDirectory = data.value("scriptWatchDirectory", defaultScriptWatchDirectory.string());
	// パスを正規化して保存
	s_data.scriptProjectPath = scriptProjectPath.lexically_normal().string();
	s_data.scriptOutputPath = scriptOutputPath.lexically_normal().string();
	s_data.scriptWatchDirectory = scriptWatchDirectory.lexically_normal().string();
	
	return true;
}

