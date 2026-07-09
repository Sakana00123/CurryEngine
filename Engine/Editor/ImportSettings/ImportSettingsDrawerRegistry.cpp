#include "pch.h"
#include "ImportSettingsDrawerRegistry.h"

#include "TextureImportSettingsDrawer.h"
#include "ModelImportSettingsDrawer.h"
#include "FbxImportSettingsDrawer.h"

namespace CurryEngine::Resources
{

	void ImportSettingsDrawerRegistry::Initialize()
	{
		// 必要に応じて、ここでデフォルトの描画クラスを登録することができます。
		Register(AssetType::Texture, std::make_unique<TextureImportSettingsDrawer>());
		Register(AssetType::Model, std::make_unique<FbxImportSettingsDrawer>());
		//Register(AssetType::Model, std::make_unique<ModelImportSettingsDrawer>());
	}

	IImportSettingsDrawer* ImportSettingsDrawerRegistry::Find(AssetType assetType)
	{
		auto& map = GetMap();
		auto it = map.find(assetType);
		if (it != map.end())
		{
			return it->second.get();
		}
		return nullptr;
	}
	void ImportSettingsDrawerRegistry::Register(AssetType assetType, std::unique_ptr<IImportSettingsDrawer> drawer)
	{
		auto& map = GetMap();
		map[assetType] = std::move(drawer);
	}
	std::unordered_map<AssetType, std::unique_ptr<IImportSettingsDrawer>>& ImportSettingsDrawerRegistry::GetMap()
	{
		static std::unordered_map<AssetType, std::unique_ptr<IImportSettingsDrawer>> drawerMap;
		return drawerMap;
	}
}
