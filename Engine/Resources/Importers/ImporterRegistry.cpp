#include "pch.h"
#include "ImporterRegistry.h"

#include "TextureImporter.h"
#include "ModelImporter.h"
#include "FbxImporter.h"


namespace CurryEngine
{
	namespace Resources
	{
		void ImporterRegistry::Initialize()
		{
			// 必要に応じて、ここでデフォルトのインポーターを登録することができます。
			Register(AssetType::Texture, std::make_unique<TextureImporter>());
			Register(AssetType::Model, std::make_unique<FbxImporter>());
			//Register(AssetType::Model, std::make_unique<ModelImporter>());
		}

		IImporter* ImporterRegistry::Find(AssetType type, const std::filesystem::path& extension)
		{
			auto& map = GetMap();
			auto it = map.find(type);
			if (it != map.end())
			{
				IImporter* importer = it->second.get();
				// サポートされている拡張子を取得
				auto supportedExtensions = importer->GetSupportedExtensions();
				// 指定された拡張子がサポートされているか確認
				if (!supportedExtensions.empty())
				{
					for (const auto& ext : supportedExtensions)
					{
						if (ext == extension.string())
						{
							return importer;
						}
					}
				}
				else
				{
					// サポートされている拡張子が空の場合、すべての拡張子をサポートしているとみなす
					return importer;
				}
			}
			return nullptr;
		}

		void ImporterRegistry::Register(AssetType type, std::unique_ptr<IImporter> importer)
		{
			auto& map = GetMap();
			map[type] = std::move(importer);
		}

		std::unordered_map<AssetType, std::unique_ptr<IImporter>>& ImporterRegistry::GetMap()
		{
			static std::unordered_map<AssetType, std::unique_ptr<IImporter>> importerMap;
			return importerMap;
		}
	}
}
