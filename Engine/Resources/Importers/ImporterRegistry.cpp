#include "pch.h"
#include "ImporterRegistry.h"

#include "TextureImporter.h"


namespace CurryEngine
{
	namespace Resources
	{
		void ImporterRegistry::Initialize()
		{
			// 必要に応じて、ここでデフォルトのインポーターを登録することができます。
			Register(AssetType::Texture, std::make_unique<TextureImporter>());
		}

		IImporter* ImporterRegistry::Find(AssetType type)
		{
			auto& map = GetMap();
			auto it = map.find(type);
			if (it != map.end())
			{
				return it->second.get();
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