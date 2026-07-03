#include "pch.h"
#include "AssetMaterial.h"
#include <Engine\Utils\JsonFileHandler.h>


namespace CurryEngine::Resources
{
	bool AssetMaterial::LoadFromFile(const std::string& path)
	{
		// マテリアルをロード
		if (std::filesystem::path(path).extension() != ".mat")
		{
			LOG_ERROR("Invalid material file extension: " + path);
			return false;
		}

		if (!std::filesystem::exists(path))
		{
			LOG_ERROR("Material file not found: " + path);
			return false;
		}

		// Jsonファイルを読み込む
		json m_JsonData;
		if (!JsonFileHandler::LoadJsonFromFile(m_JsonData, path))
		{
			return false;
		}
		// JsonデータからMaterialを復元
		if (!m_material.Deserialize(m_JsonData))
		{
			return false;
		}
		_path = path;
		return true;
	}
	bool AssetMaterial::SaveToFile(const std::string& path)
	{
		// マテリアルをJsonに変換
		json m_JsonData;
		m_JsonData = m_material.Serialize();

		// Jsonファイルに保存
		JsonFileHandler::SaveJsonToFile(m_JsonData, path);
		return true;
	}
}