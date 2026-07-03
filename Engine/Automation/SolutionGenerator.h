#pragma once
#include <string>

/**************************************************
 * @file
 * @brief Visual Studio でソリューションファイルを生成するユーティリティ
 * @details ソリューションファイルが存在しない場合に、プロジェクトファイルとソリューションファイルを生成する。
 *          生成には C# のスクリプトを使用し、Visual Studio の Developer Command Prompt を呼び出して実行する。
 **************************************************/

class SolutionGenerator
{
public:
	/**
	 * @brief ソリューションファイルとプロジェクトファイルを生成する。
	 * @param projectRoot プロジェクトのルートディレクトリ
	 * @param projectName プロジェクト名（例: "CurryEngine"）
	 * @param runtimeDllPath ランタイム DLL のパス（例: "./CurryEngine.Runtime.dll"）
	 * @return 生成に成功した場合は true、失敗した場合は false を返す。
	 */
	static bool Generate(const std::string& projectRoot,
		const std::string& projectName,
		const std::string& runtimeDllPath);
};