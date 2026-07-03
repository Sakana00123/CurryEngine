#include "pch.h"
#include "AssetReferenceIndex.h"
#include <fstream>
#include <json.hpp>



namespace CurryEngine
{
	namespace Resources
	{
		std::unordered_map<AssetId, std::unordered_set<std::string>> AssetReferenceIndex::s_referencedBy;
		std::unordered_map<std::string, std::unordered_set<AssetId>> AssetReferenceIndex::s_referencesOf;


		namespace
		{
			std::unordered_set<AssetId> ScanAssetIdsInJsonFile(const std::string& filePath)
			{
				std::unordered_set<AssetId> foundIds;
				try
				{
					std::ifstream file(filePath);
					if (!file.is_open())
					{
						LOG_ERROR("[AssetReferenceIndex] Failed to open file: " + filePath);
						return foundIds;
					}
					nlohmann::json jsonData;
					file >> jsonData;
					if (jsonData.contains("assetId") && jsonData["assetId"].is_string())
					{
						foundIds.insert(jsonData["assetId"].get<AssetId>());
					}
				}
				catch (const std::exception& e)
				{
					LOG_ERROR(std::string("[AssetReferenceIndex] Error scanning file: ") + e.what());
				}
				return foundIds;
			}
		}


		void AssetReferenceIndex::RebuildFull(const std::vector<std::string>& scanTargetFiles)
		{
			s_referencedBy.clear();
			s_referencesOf.clear();
			for (const auto& filePath : scanTargetFiles)
			{
				RebuildForFile(filePath);
			}
		}

		void AssetReferenceIndex::RebuildForFile(const std::string& filePath)
		{
			// 既存の参照情報を削除
			auto oldReferencesIt = s_referencesOf.find(filePath);
			if (oldReferencesIt != s_referencesOf.end())
			{
				for (const auto& oldId : oldReferencesIt->second)
				{
					s_referencedBy[oldId].erase(filePath);
				}
			}

			// 新しい参照情報を収集
			std::unordered_set<AssetId> foundIds = ScanAssetIdsInJsonFile(filePath);

			// 新しい参照情報を更新
			for (const auto& newId : foundIds)
			{
				s_referencedBy[newId].insert(filePath);
			}
			s_referencesOf[filePath] = std::move(foundIds);
		}

		std::vector<std::string> AssetReferenceIndex::FindReferencingFiles(const AssetId& assetId)
		{
			std::vector<std::string> referencingFiles;
			auto it = s_referencedBy.find(assetId);
			if (it != s_referencedBy.end())
			{
				referencingFiles.insert(referencingFiles.end(), it->second.begin(), it->second.end());
			}
			return referencingFiles;
		}

	}
}