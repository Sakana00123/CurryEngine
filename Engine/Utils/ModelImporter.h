#pragma once
#include "Engine/Resources/ModelAsset.h"
#include "Engine/Resources/Mesh.h"

#define NOMINMAX

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "../tinygltf-release/tiny_gltf.h"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>

struct ImportSettings
{
	bool staticBatching = false; // 静的バッチングを有効にするか
	bool importAnimations = true; // アニメーションをインポートするか
	float scaleFactor = 1.0f; // モデルのスケールファクター
};
class ModelImporter
{
public:
	static std::shared_ptr<ModelAsset> ImportModel(const std::string& filePath, const ImportSettings& settings);

private:
	static void FetchNodes(const tinygltf::Model& gltfModel, std::vector<NodeDesc>& nodes);
	static void FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel, std::vector<std::shared_ptr<Mesh>>& meshes);
	static void FetchBatchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel, std::vector<std::shared_ptr<Mesh>>& meshes);
	static void FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel, std::vector<std::shared_ptr<Material>>& materials);
	static void FetchTextures(ID3D11Device* device, const tinygltf::Model& gltfModel, std::vector<std::shared_ptr<Texture>>& textures);
	static void FetchAnimations(const tinygltf::Model& gltfModel, std::vector<std::shared_ptr<AnimationClip>>& animations);
};