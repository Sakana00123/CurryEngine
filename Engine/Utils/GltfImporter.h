#pragma once
#include "IModelImporter.h"

#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "../tinygltf-release/tiny_gltf.h"

// TODO: バッチメッシュの分岐のせいでコードが複雑になっているので、あとでリファクタリングすること。
#define SUPPORT_BATCHING

namespace CurryEngine
{
	namespace Utils
	{
		/**
		 * @brief GLTF モデルインポーター。GLTF 形式のモデルファイルを読み込み、`ModelAsset` に変換するクラス。
		 * @details tinygltf ライブラリを使用して GLTF ファイルを解析し、モデルデータを `ModelAsset` に格納します。
		 */
		class GltfImporter : public IModelImporter
		{
		public:
			virtual ~GltfImporter() = default;
			/**
			 * @brief GLTF ファイルを読み込み、`ModelAsset` に変換する関数。
			 * @param path 読み込む GLTF ファイルのパス。
			 * @param asset 読み込んだモデルデータを格納する `ModelAsset` オブジェクトへの参照。
			 * @return 成功した場合は true、失敗した場合は false を返します。
			 */
			bool Import(const std::string& path, ModelAsset& asset) override;
			/**
			 * @brief このインポーターがサポートするファイル拡張子のリストを返す関数。
			 * @return サポートするファイル拡張子のリスト（例: {".gltf", ".glb"}）。
			 */
			std::vector<std::string> GetSupportedExtensions() const override;

		private:
			void FetchScenes(const tinygltf::Model& gltfModel, ModelAsset& asset);
			void FetchNodes(const tinygltf::Model& gltfModel, ModelAsset& asset);
			void FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel, ModelAsset& asset);
#ifdef SUPPORT_BATCHING
			void FetchBatchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel, ModelAsset& asset);
#endif // SUPPORT_BATCHING
			void FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel, ModelAsset& asset);
			void FetchTextures(ID3D11Device* device, const tinygltf::Model& gltfModel, ModelAsset& asset);
			void FetchSkins(const tinygltf::Model& gltfModel, ModelAsset& asset);
			void FetchAnimations(const tinygltf::Model& gltfModel, ModelAsset& asset);
		};
	}
}