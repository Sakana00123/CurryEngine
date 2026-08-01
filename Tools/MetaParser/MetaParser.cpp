// MetaParser.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "CSharpGenerater.h"
#include "Parser.h"
#include "Generater.h"
#include <filesystem>
#include <fstream>
#include <iostream>

int main(int argc, char** argv)
{
	std::cout << "argc = " << argc << std::endl;
	for (int i = 0; i < argc; i++)
		std::cout << "arg[" << i << "] = " << argv[i] << std::endl;

	
	if (argc < 3)
    {
        std::cout << "Usage: MetaParser <outputDir> <inputDir...>\n";
        return 1;
	}

	std::string outputJson = argv[1];
	std::vector<std::string> inputDirs;
	for (int i = 2; i < argc; i++)
	{
		inputDirs.push_back(argv[i]);
	}

	// パーサーを作成してディレクトリを解析し、JSON 出力
	std::string jsonOutputDir = outputJson + "/Json";
	Parser parser(jsonOutputDir);
	std::vector<FileInfo> allFiles;
	for (const auto& dir : inputDirs)
	{
		auto files = parser.ParseDirectory(dir);
		allFiles.insert(allFiles.end(), files.begin(), files.end());
	}

	// .cpp, .cs 生成の対象外とするクラスのリスト(Object や GameObject など、手動で定義されているクラスは生成しない)
	std::vector<std::string> ignoreClasses = {
		"Vector2", "Vector3", "Vector4", "Quaternion", "Matrix4x4",
		"Color", "Transform", "GameObject", "Component", "Object",
	};
	
	// コード生成器を作成してコード生成
	Generater generater(outputJson);
	generater.Generate(allFiles, ignoreClasses);

	// C# コード生成器を作成してコード生成
	std::filesystem::path typeMapPath = std::filesystem::current_path() / "type_map.json";

	// C# 出力ディレクトリは outputJson/CSharp とする
	std::filesystem::path csOutputDir(outputJson);
	csOutputDir /= "CSharp";
	auto createDirectoriesIfNotExist = [](const std::filesystem::path& dir) {
		if (!std::filesystem::exists(dir))
		{
			std::filesystem::create_directories(dir);
		}
		};

	// C# 出力ディレクトリを作成(サブディレクトリであるNative, Wrappers, Enums, Structsも作成)
	std::filesystem::path csNativeDir = csOutputDir / "Native";
	createDirectoriesIfNotExist(csNativeDir);
	std::filesystem::path csWrappersDir = csOutputDir / "Wrappers";
	createDirectoriesIfNotExist(csWrappersDir);
	std::filesystem::path csEnumsDir = csOutputDir / "Enums";
	createDirectoriesIfNotExist(csEnumsDir);
	std::filesystem::path csStructsDir = csOutputDir / "Structs";
	createDirectoriesIfNotExist(csStructsDir);


	std::vector<FileInfo> csFiles;

	// 基底クラスを Componentに変更するクラスのリスト(Component を継承しているクラスで、Component として扱いたいクラスを指定)
	std::vector<std::string> changeBasesToComponent = {
		"RectTransform"
	};

	for (auto& file : allFiles)
	{
		// C# 生成対象を絞る
		bool allowGenerate = false;
		for (auto& c : file.classes)
		{
			// ignoreClasses に含まれるクラスは生成対象外
			if (std::find(ignoreClasses.begin(), ignoreClasses.end(), c.name) != ignoreClasses.end())
			{
				std::cout << "Skipping C# generation for class: " << c.name << "\n";
				continue;
			}
			// changeBasesToComponent に含まれるクラスは基底クラスを Component に変更して生成
			if (std::find(changeBasesToComponent.begin(), changeBasesToComponent.end(), c.name) != changeBasesToComponent.end())
			{
				std::cout << "Changing base class to Component for: " << c.name << "\n";
				c.bases.clear();
				c.bases.push_back("Component");
				allowGenerate = true;
				break;
			}

			if (c.reflect)
			{
				allowGenerate = true;
				break;
			}
		}
		if (allowGenerate)
		{
			csFiles.push_back(file);
		}
	}


	CSharpGenerater csGenerater(csOutputDir.string(), typeMapPath.string());
	csGenerater.Generate(csFiles);

	return 0;
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
