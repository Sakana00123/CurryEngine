#pragma once
#include "json.hpp"
#include <fstream>
#include <string>
#include "Engine/Editor/AssetBrowser.h"

using json = nlohmann::json;

enum class JsonIOFormat
{
	Text,
	Binary,
	Auto // ファイルの拡張子に基づいて自動的にフォーマットを判断する（例: .binならBinary、それ以外はText）
};

enum class JsonIOError
{
	None,
	FileNotFound,
	ParseError,
	WriteError,
	UnknownError
};

enum class FileAttribute
{
	ReadOnly = FILE_ATTRIBUTE_READONLY,
	Hidden = FILE_ATTRIBUTE_HIDDEN,
	System = FILE_ATTRIBUTE_SYSTEM,
	Archive = FILE_ATTRIBUTE_ARCHIVE,
	Normal = FILE_ATTRIBUTE_NORMAL
};

class JsonFileHandler
{
public:
	static void SaveJsonToFile(const json& j, const std::string& filePath, JsonIOFormat format = JsonIOFormat::Auto, FileAttribute attribute = FileAttribute::Normal) {

		std::ios_base::openmode mode = std::ios::out;

		// フォーマットがAutoの場合、ファイルの拡張子に基づいてフォーマットを判断する
		if (format == JsonIOFormat::Auto) { 
			std::filesystem::path path(filePath);
			// 拡張子が.binならバイナリフォーマット、それ以外はテキストフォーマットとする
			if (path.extension() == ".bin") {
				format = JsonIOFormat::Binary;
			}
			else {
				format = JsonIOFormat::Text;
			}
		}

		if (format == JsonIOFormat::Binary) {
			mode |= std::ios::binary;
		}

		// フォルダが存在しない場合は作成する
		std::filesystem::path path(filePath);
		std::filesystem::path directory = path.parent_path();
		if (!directory.empty() && !std::filesystem::exists(directory)) {
			std::filesystem::create_directories(directory);
		}

		// ファイルを開く
		std::ofstream file(filePath, mode);
		if (file) {
			if (format == JsonIOFormat::Binary) {
				std::vector<std::uint8_t> buffer = json::to_cbor(j);
				file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
			}
			else if (format == JsonIOFormat::Text)
			{
				file << j.dump(4);
			}
			file.close();

			// ファイル属性の設定
			SetFileAttributesA(filePath.c_str(), static_cast<DWORD>(attribute));

			// アセットブラウザのリフレッシュ
			AssetBrowser::Refresh();
		}
	}

	static bool LoadJsonFromFile(json& j, const std::string& filePath, JsonIOFormat format = JsonIOFormat::Auto) {
		
		//_ASSERT_EXPR(std::filesystem::exists(std::filesystem::path(filePath)), "ファイルを開けませんでした: " + filePath);
		
		//ファイルが存在しなかったら、新規作成
		if (!std::filesystem::exists(std::filesystem::path(filePath))) {
			std::ofstream ofs(filePath);
			if (ofs) {
				j = json::object();  // 空のオブジェクト {} で初期化
				SaveJsonToFile(j, filePath, format); // 新規作成したファイルに保存
				return true;
			}
		}

		std::ios_base::openmode mode = std::ios::in;

		// フォーマットがAutoの場合、ファイルの拡張子に基づいてフォーマットを判断する
		if (format == JsonIOFormat::Auto) {
			std::filesystem::path path(filePath);
			// 拡張子が.binならバイナリフォーマット、それ以外はテキストフォーマットとする
			if (path.extension() == ".bin") {
				format = JsonIOFormat::Binary;
			}
			else {
				format = JsonIOFormat::Text;
			}
		}

		if (format == JsonIOFormat::Binary) {
			mode |= std::ios::binary;
		}

		std::ifstream ifs(filePath, mode);
		if (!ifs) return false;

		if (format == JsonIOFormat::Binary) {
			std::vector<std::uint8_t> buffer((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			j = json::from_cbor(buffer);
		}
		else if (format == JsonIOFormat::Text)
		{
			ifs >> j;
		}
		ifs.close();
		return true;
	}
};