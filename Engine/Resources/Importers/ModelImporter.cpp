#include "pch.h"
#include "ModelImporter.h"
#include "Engine/Utils/GltfImporter.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include <filesystem>

#include <assimp/Importer.hpp>		 // C++インターフェース
#include <assimp/Exporter.hpp> 		 // C++インターフェース
#include <assimp/scene.h>            // 出力データ構造
#include <assimp/postprocess.h>      // 後処理フラグ
#include <Engine\Resources\AssetModel.h>

namespace CurryEngine
{
	namespace Resources
	{
		std::shared_ptr<Resource> ModelImporter::Import(const AssetMeta& meta)
		{
			std::shared_ptr<AssetModel> modelResource = std::make_shared<AssetModel>();

			// TODO: インポーターを拡張して、FBXやOBJなどの他の形式もサポートするようにする
#if 0
			if (std::filesystem::path(meta.path).extension() == ".gltf" || std::filesystem::path(meta.path).extension() == ".glb")
			{
				CurryEngine::Utils::GltfImporter gltfImporter;
				if (!gltfImporter.Import(meta.path, *modelResource))
				{
					LOG_ERROR("Failed to import model: " + meta.path);
					return nullptr;
				}

				// リソースの作成とアップロード
				auto device = Graphics::GetDevice();
				modelResource->CreateAndUploadResources(device);
			}
			else
			{
				LOG_ERROR("Unsupported model format: " + meta.path);
				return nullptr;
			}
#else
			if (!modelResource->LoadFromFile(meta.path))
			{
				LOG_ERROR("Failed to import model: " + meta.path);
				return nullptr;
			}
#endif // 0


			return modelResource;
		}
		std::vector<std::string> ModelImporter::GetSupportedExtensions() const
		{
			return { /*".fbx", ".obj", */".gltf", ".glb" };
		}

	}
}