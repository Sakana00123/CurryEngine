#include "pch.h"
#include "ModelImporter.h"
#include "Engine/Resources/ModelAsset.h"
#include "Engine/Utils/GltfImporter.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

#include <assimp/Importer.hpp>		 // C++インターフェース
#include <assimp/scene.h>            // 出力データ構造
#include <assimp/postprocess.h>      // 後処理フラグ

namespace CurryEngine
{
	namespace Resources
	{
		std::shared_ptr<Resource> ModelImporter::Import(const AssetMeta& meta)
		{
			std::shared_ptr<ModelAsset> modelResource = std::make_shared<ModelAsset>();

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
			// Assimpを使用してモデルをインポート
			ImportWithAssimp(meta, modelResource);
			// リソースの作成とアップロード


#endif // 0


			return modelResource;
		}
		std::vector<std::string> ModelImporter::GetSupportedExtensions() const
		{
			return { /*".fbx", ".obj", */".gltf", ".glb" };
		}

		bool ModelImporter::ImportWithAssimp(const AssetMeta& meta, std::shared_ptr<ModelAsset>& outResource)
		{
			// Assimpを使用してモデルをインポート
			Assimp::Importer importer;
			// 読み込むファイルのパスと、必要な後処理フラグを指定してシーンを読み込む(例: 三角形化、UV座標の反転、接線空間の計算など)
			const aiScene* scene = importer.ReadFile(meta.path, aiProcessPreset_TargetRealtime_Quality | aiProcess_MakeLeftHanded | aiProcess_FlipUVs);
			// 読み込みに失敗した場合はエラーメッセージをログに出力してfalseを返す
			if (!scene)
			{
				LOG_ERROR("Failed to import model: " + meta.path + " - " + importer.GetErrorString());
				return false;
			}

			// 読み込んだシーンからモデルアセットデータを構築する
			
			outResource->scenes.resize(1);
			outResource->scenes[0].name = scene->mName.C_Str();

			

			// ノードの階層構造を再帰的に処理して、モデルアセットのノード構造を構築する処理をここに実装する
			auto ProcessNode = [&](const aiNode* node, auto&& ProcessNodeRef) -> void
				{
					// ノードの名前や変換行列などの情報をモデルアセットのノード構造にコピーする処理をここに実装する
					node->mName;

					// 子ノードがある場合は再帰的に処理する
					for (unsigned int i = 0; i < node->mNumChildren; ++i)
					{
						ProcessNodeRef(node->mChildren[i], ProcessNodeRef);
					}
				};
			ProcessNode(scene->mRootNode, ProcessNode);
			return true;
		}

	}
}