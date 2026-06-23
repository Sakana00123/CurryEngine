#include "pch.h"
#include "AssetMetaSerializer.h"
#include "AssetMeta.h"
#include <fstream>


namespace CurryEngine
{
	namespace Resources
	{
		std::string AssetMetaSerializer::MetaPathFor(const std::string& assetPath)
		{
			std::string metaPath = assetPath + ".meta";
			std::replace(metaPath.begin(), metaPath.end(), '\\', '/'); // パス区切りを統一
			return metaPath;
		}

		AssetType AssetMetaSerializer::AssetTypeFromString(const std::string& typeStr)
		{
			if (typeStr == "Texture") return AssetType::Texture;
			if (typeStr == "Model") return AssetType::Model;
			if (typeStr == "Sound") return AssetType::Sound;
			if (typeStr == "Scene") return AssetType::Scene;
			if (typeStr == "Prefab") return AssetType::Prefab;
			if (typeStr == "Script") return AssetType::Script;
			if (typeStr == "Shader") return AssetType::Shader;
			return AssetType::Unknown;
		}

		std::string AssetMetaSerializer::AssetTypeToString(AssetType type)
		{
			switch (type)
			{
			case AssetType::Texture: return "Texture";
			case AssetType::Model: return "Model";
			case AssetType::Sound: return "Sound";
			case AssetType::Scene: return "Scene";
			case AssetType::Prefab: return "Prefab";
			case AssetType::Script: return "Script";
			case AssetType::Shader: return "Shader";
			default: return "Unknown";
			}
		}

		void to_json(nlohmann::json& j, const AssetMeta& meta)
		{
			j = nlohmann::json{
				{"id", meta.id.ToString()},
				{"path", meta.path},
				{"type", CurryEngine::Resources::AssetMetaSerializer::AssetTypeToString(meta.type)},
				{"isFolder", meta.isFolder},
				{"importSettings", meta.importSettings}
			};
		}
		void from_json(const nlohmann::json& j, AssetMeta& meta)
		{
			meta.id = AssetId(j.at("id").get<std::string>());
			j.at("path").get_to(meta.path);
			meta.type = CurryEngine::Resources::AssetMetaSerializer::AssetTypeFromString(j.at("type").get<std::string>());
			if (j.contains("isFolder"))
			{
				j.at("isFolder").get_to(meta.isFolder);
			}
			else
			{
				meta.isFolder = std::filesystem::is_directory(meta.path); // デフォルト値としてファイルシステムの情報を使用
			}
			if (j.contains("importSettings"))
			{
				meta.importSettings = j.at("importSettings");
			}
			else
			{
				meta.importSettings = nlohmann::json::array(); // デフォルト値として空の配列を設定
			}
		}


		bool AssetMetaSerializer::Save(const AssetMeta& meta)
		{
			std::string metaPath = MetaPathFor(meta.path);
			std::ofstream ofs(metaPath);
			if (!ofs)
			{
				return false;
			}
			nlohmann::json j = meta;
			ofs << j.dump(4); // インデントを4スペースにして保存
			return true;
		}

		AssetMeta AssetMetaSerializer::Load(const std::string& assetPath)
		{
			std::string metaPath = MetaPathFor(assetPath);
			std::ifstream ifs(metaPath);
			if (!ifs)
			{
				return AssetMeta(); // デフォルトの空のメタデータを返す
			}
			nlohmann::json j;
			ifs >> j;
			return j.get<AssetMeta>();
		}

	}
}