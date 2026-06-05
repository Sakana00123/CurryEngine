#include "pch.h"
#include "ModelImporter.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

static bool _NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*) {
    return true;
}

std::shared_ptr<ModelAsset> ModelImporter::ImportModel(const std::string& filePath, const ImportSettings& settings)
{
	std::shared_ptr<ModelAsset> modelAsset = std::make_shared<ModelAsset>();

    tinygltf::TinyGLTF tinyGltf;
    tinyGltf.SetImageLoader(_NullLoadImageData, nullptr);

    std::shared_ptr<tinygltf::Model> gltfModel;
    gltfModel = std::make_shared<tinygltf::Model>();

    //if (gltfModels.find(filePath) != gltfModels.end()) {
    //    auto& model = gltfModels[filePath];
    //    if (auto sharedModel = model)
    //    {
    //        gltfModel = sharedModel;
    //    }
    //}
    // Load glTF file
    //else
    {
        std::string error, warning;
        bool succeded = false;
        if (filePath.find(".glb") != std::string::npos) {
            succeded = tinyGltf.LoadBinaryFromFile(gltfModel.get(), &error, &warning, filePath.c_str());
        }
        if (filePath.find(".gltf") != std::string::npos) {
            succeded = tinyGltf.LoadASCIIFromFile(gltfModel.get(), &error, &warning, filePath.c_str());
        }

        _ASSERT_EXPR_A(warning.empty(), warning.c_str());
        _ASSERT_EXPR_A(error.empty(), error.c_str());
        _ASSERT_EXPR_A(succeded, L"Failed to load glTF file");

        //gltfModels[filePath] = gltfModel;
    }

	// ノード、メッシュ、マテリアル、テクスチャ、アニメーションの情報を取得
	FetchNodes(*gltfModel, modelAsset->nodes);
	if (!settings.staticBatching)
    {
        FetchMeshes(Graphics::GetDevice(), *gltfModel, modelAsset->meshes);
    }
	else
    {
        FetchBatchMeshes(Graphics::GetDevice(), *gltfModel, modelAsset->meshes);
    }
	FetchMaterials(Graphics::GetDevice(), *gltfModel, modelAsset->materials);
	std::vector<std::shared_ptr<Texture>> textures;
	FetchTextures(Graphics::GetDevice(), *gltfModel, textures);
	std::vector<std::shared_ptr<AnimationClip>> animations;


	return modelAsset;
}

void ModelImporter::FetchNodes(const tinygltf::Model& gltfModel, std::vector<NodeDesc>& nodes)
{
	// ノードの情報を取得
    for (const auto& gltfNode : gltfModel.nodes)
    {
        NodeDesc node;
        node.name = gltfNode.name;
        node.parentIndex = -1; // 後で親子関係を構築する際に設定
        // ローカルトランスフォーム行列の計算
        DirectX::XMFLOAT4X4 localTransform;
        if (!gltfNode.translation.empty() && !gltfNode.rotation.empty() && !gltfNode.scale.empty())
        {
            // TRS 形式のトランスフォーム
            DirectX::XMFLOAT3 translation = DirectX::XMFLOAT3(
                static_cast<float>(gltfNode.translation[0]),
                static_cast<float>(gltfNode.translation[1]),
                static_cast<float>(gltfNode.translation[2])
			);
            DirectX::XMVECTOR Translation = DirectX::XMLoadFloat3(&translation);
            DirectX::XMFLOAT4 rotation = DirectX::XMFLOAT4(
                static_cast<float>(gltfNode.rotation[0]),
                static_cast<float>(gltfNode.rotation[1]),
                static_cast<float>(gltfNode.rotation[2]),
                static_cast<float>(gltfNode.rotation[3])
			);
            DirectX::XMVECTOR Rotation = DirectX::XMLoadFloat4(&rotation);
            DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(
                static_cast<float>(gltfNode.scale[0]),
                static_cast<float>(gltfNode.scale[1]),
				static_cast<float>(gltfNode.scale[2])
			);
            DirectX::XMVECTOR Scale = DirectX::XMLoadFloat3(&scale);
            DirectX::XMStoreFloat4x4(&localTransform, DirectX::XMMatrixAffineTransformation(Scale, DirectX::XMVectorZero(), Rotation, Translation));
        }
        else if (!gltfNode.matrix.empty())
        {
            // 行列形式のトランスフォーム
            localTransform = DirectX::XMFLOAT4X4(
                static_cast<float>(gltfNode.matrix[0]), static_cast<float>(gltfNode.matrix[1]), static_cast<float>(gltfNode.matrix[2]), static_cast<float>(gltfNode.matrix[3]),
                static_cast<float>(gltfNode.matrix[4]), static_cast<float>(gltfNode.matrix[5]), static_cast<float>(gltfNode.matrix[6]), static_cast<float>(gltfNode.matrix[7]),
                static_cast<float>(gltfNode.matrix[8]), static_cast<float>(gltfNode.matrix[9]), static_cast<float>(gltfNode.matrix[10]), static_cast<float>(gltfNode.matrix[11]),
                static_cast<float>(gltfNode.matrix[12]), static_cast<float>(gltfNode.matrix[13]), static_cast<float>(gltfNode.matrix[14]), static_cast<float>(gltfNode.matrix[15])
            );
        }
        else
        {
            // デフォルトのトランスフォーム（単位行列）
            localTransform = DirectX::XMFLOAT4X4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
		}
		// gltfは右手座標系でYアップなので、左手座標系でYアップのCoordinateSystem変換を行う
		XMMATRIX coordinateSystemConversion = XMMatrixScaling(1.0f, 1.0f, -1.0f);
		XMMATRIX localTransformMatrix = XMLoadFloat4x4(&localTransform);
		localTransformMatrix = XMMatrixMultiply(localTransformMatrix, coordinateSystemConversion);
		XMStoreFloat4x4(&localTransform, localTransformMatrix);
        node.localTransform = localTransform;
        //node.meshIndex = gltfNode.mesh; // メッシュインデックスを保存
        nodes.push_back(node);
    }
    // 親子関係の構築
    for (size_t i = 0; i < gltfModel.nodes.size(); ++i)
    {
        const auto& gltfNode = gltfModel.nodes[i];
        for (const auto& childIndex : gltfNode.children)
        {
            nodes[childIndex].parentIndex = static_cast<int>(i);
        }
    }
}

void ModelImporter::FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel, std::vector<std::shared_ptr<Mesh>>& meshes)
{
	// メッシュの情報を取得
}

void ModelImporter::FetchBatchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel, std::vector<std::shared_ptr<Mesh>>& meshes)
{
}

void ModelImporter::FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel, std::vector<std::shared_ptr<Material>>& materials)
{
	// マテリアルの情報を取得
	std::vector<std::shared_ptr<Material>> gltfMaterials(gltfModel.materials.size());
	// TODO: 続きを実装（マテリアルの設定が合わなさそうなので、Materialクラスの設計を見直すか、gltfのマテリアル定義に合わせてMaterialクラスを拡張する必要があるかも）
    for (size_t i = 0; i < gltfModel.materials.size(); ++i)
    {
        const auto& gltfMaterial = gltfModel.materials[i];
        std::shared_ptr<Material> material = std::make_shared<Material>();

        //material->SetShader(Graphics::GetDevice(), );
        // マテリアルのプロパティを設定
        // 例: ベースカラー、メタリック、ラフネスなど
        // これらのプロパティは glTF のマテリアル定義から取得できます
        gltfMaterials[i] = material;
	}


}

void ModelImporter::FetchTextures(ID3D11Device* device, const tinygltf::Model& gltfModel, std::vector<std::shared_ptr<Texture>>& textures)
{
}

void ModelImporter::FetchAnimations(const tinygltf::Model& gltfModel, std::vector<std::shared_ptr<AnimationClip>>& animations)
{
}