#pragma once
#include <string>

struct CopyItem
{
	std::string src;						//!< コピー元のファイルパス
	std::string type;						//!< コピーの種類（例: "file" | "folder" | "glob"）
	std::vector<std::string> exclude;		//!< コピーから除外する拡張子のリスト（例: [".tmp", ".log"], glob パターンの場合は ["*.tmp", "*.log"] など。フォルダタイプのみ有効）
};

/**
 * @file
 * @brief ビルド設定を管理するヘッダ。
 * @details ビルド設定の読み込み/保存を提供します。ビルド設定は `./BuildSetting/build_settings.json` に JSON 形式で保存されます。
 */
struct BuildSettings
{
	std::string appName = "MyGame";									//!< アプリケーション名
	std::string iconPath = "./BuildSetting/curry.ico";				//!< アプリケーションアイコンのファイルパス
	std::string outputDir = "./BuildOutput/";						//!< ビルド出力先ディレクトリ
	std::string zipToolPath = "C:/Program Files/7-Zip/7z.exe";		//!< 圧縮ツールのファイルパス（例: 7-Zip）
	std::vector<CopyItem> copyItems;	//!< ビルド時にコピーするファイル/フォルダのリスト

	void Load(const std::string& path = "./BuildSetting/build_settings.json");
	void Save(const std::string& path = "./BuildSetting/build_settings.json") const;
};