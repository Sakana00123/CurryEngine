#include "pch.h"
#include "ModelImporter.h"
#include "Engine/Resources/ModelAsset.h"
#include "Engine/Utils/GltfImporter.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

namespace CurryEngine
{
	namespace Resources
	{
		std::shared_ptr<Resource> ModelImporter::Import(const AssetMeta& meta)
		{
			std::shared_ptr<ModelAsset> modelResource = std::make_shared<ModelAsset>();

			// TODO: インポーターを拡張して、FBXやOBJなどの他の形式もサポートするようにする
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

			return modelResource;
		}
		std::vector<std::string> ModelImporter::GetSupportedExtensions() const
		{
			return { /*".fbx", ".obj", */".gltf", ".glb" };
		}

	}
}