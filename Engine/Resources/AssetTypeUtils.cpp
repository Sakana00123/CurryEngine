#include "pch.h"
#include "AssetTypeUtils.h"


namespace CurryEngine
{
	namespace Resources
	{
		AssetType AssetTypeUtils::DetectFromExtension(const std::string& extension)
		{
			std::string ext = extension;
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); // 小文字に変換して比較

			static const std::unordered_map<std::string, AssetType> extensionToTypeMap = {
				{ ".png", AssetType::Texture },
				{ ".jpg", AssetType::Texture },
				{ ".jpeg", AssetType::Texture },
				{ ".tif", AssetType::Texture },
				{ ".tga", AssetType::Texture },
				{ ".dds", AssetType::Texture },
				{ ".fbx", AssetType::Model },
				{ ".obj", AssetType::Model },
				{ ".gltf", AssetType::Model },
				{ ".glb", AssetType::Model },
				{ ".pmx", AssetType::Model },
				{ ".blend", AssetType::Model },
				{ ".wav", AssetType::Sound },
				//{ ".mp3", AssetType::Sound },
				//{ ".ogg", AssetType::Sound },
				{ ".scene", AssetType::Scene },
				{ ".prefab", AssetType::Prefab },
				{ ".cs", AssetType::Script },
				{ ".hlsl", AssetType::Shader },
				{ ".hlsli", AssetType::Shader },
				{ ".mat", AssetType::Material },
			};
			auto it = extensionToTypeMap.find(ext);
			if (it != extensionToTypeMap.end())
			{
				return it->second;
			}
			else
			{
				return AssetType::Unknown;
			}
		}
	}
}