#include "pch.h"
#include "GltfModelRenderer.h"
#include "Engine/Core/GameObject.h"

#include <filesystem>
#include <fstream>

#include "Engine/Core/Misc.h"
#include "Engine/Editor/Dialog.h"
#include "Engine/Resources/Texture.h"
#include "Engine/Resources/Shader.h"
#include "Engine/Resources/ResourceManager.h"

#include "Engine/Editor/HlslEditor.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

REGISTER_COMPONENT(GltfModelRenderer, "Rendering")

Math::BoundingBox GltfModelRenderer::CalculateAABB() const
{
    Math::BoundingBox aabb;

    DirectX::XMMATRIX M = DirectX::XMMatrixIdentity();
    XMFLOAT3 scale = GetOwner()->transform->GetWorldScale();
    XMFLOAT4 rotation = GetOwner()->transform->GetWorldRotation();
    M = DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&scale));
    M *= DirectX::XMMatrixRotationQuaternion(XMLoadFloat4(&rotation));
    XMFLOAT4X4 matrix;
    DirectX::XMStoreFloat4x4(&matrix, M);

    // Traverse nodes
    std::function<void(int, const DirectX::XMFLOAT4X4&)> traverse;
    traverse = [&](int nodeIndex, const DirectX::XMFLOAT4X4& parentTransform) {
        const Node& node = nodes[nodeIndex];

        // Compute local transform
        DirectX::XMMATRIX local = DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z) *
            DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotation)) *
            DirectX::XMMatrixTranslation(node.translation.x, node.translation.y, node.translation.z);

        // Compute global transform
        DirectX::XMMATRIX parent = DirectX::XMLoadFloat4x4(&parentTransform);
        DirectX::XMMATRIX global = local * parent;

        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, global.r[3]);
        aabb.Encapsulate(pos);

        DirectX::XMFLOAT4X4 globalFloat4x4;
        DirectX::XMStoreFloat4x4(&globalFloat4x4, global);

        // Recurse to children
        for (int childIndex : node.children) {
            traverse(childIndex, globalFloat4x4);
        }
        };
    for (int rootNodeIndex : scenes[defaultScene].nodes) {
        traverse(rootNodeIndex, matrix);
    }
    return aabb;
}

void GltfModelRenderer::ReplacePixelShader(ID3D11Device* device, const char* filePath)
{
    CreatePixelShaderFromCSO(device, filePath, pixelShader.ReleaseAndGetAddressOf());
#ifdef _DEBUG
    material->SetShader(device, ResourceManager::Load<PixelShader>(filePath));
#endif // _DEBUG
}

void GltfModelRenderer::ReplaceVertexShader(ID3D11Device* device, const char* filePath)
{
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"JOINTS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    CreateVertexShaderFromCSO(device, filePath, vertexShader.ReleaseAndGetAddressOf(), inputLayout.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
#ifdef _DEBUG
    //material->SetShader(device, ResourceManager::Load<PixelShader>(filePath));
#endif // _DEBUG
}

void GltfModelRenderer::ReplaceCSMVertexShader(ID3D11Device* device, const char* filePath)
{
    CreateVertexShaderFromCSO(device, filePath, vertexShaderCsm.ReleaseAndGetAddressOf(), NULL, NULL, 0);
}

bool _NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*) {
    return true;
}

GltfModelRenderer::GltfModelRenderer() : staticBatching(false)
{
	
}

void GltfModelRenderer::LoadModel(ID3D11Device* device, const std::string& filePath, bool staticBatching)
{
	this->filePath = filePath;
	this->staticBatching = staticBatching;

	// キャッシュファイルがあればそちらを読み込む
    std::filesystem::path cerealFilePath(filePath);
    cerealFilePath.replace_extension(staticBatching ? "batchCereal" : "cereal");
    if (std::filesystem::exists(cerealFilePath.c_str())) {
        std::ifstream ifs(cerealFilePath.c_str(), std::ios::binary);
        cereal::BinaryInputArchive deserialization(ifs);
        deserialization(
            cereal::make_nvp("scenes", scenes),
            cereal::make_nvp("defaultScene", defaultScene),
            cereal::make_nvp("nodes", nodes),
            cereal::make_nvp("materials", materials)
        );
        deserialization(cereal::make_nvp("batchMeshes", batchMeshes));
        deserialization(cereal::make_nvp("meshes", meshes));
        deserialization(cereal::make_nvp("textures", textures), cereal::make_nvp("images", images));
        deserialization(cereal::make_nvp("skins", skins), cereal::make_nvp("animations", animations));
    }
    else {
        tinygltf::TinyGLTF tinyGltf;
        tinyGltf.SetImageLoader(_NullLoadImageData, nullptr);

        std::shared_ptr<tinygltf::Model> gltfModel;
        gltfModel = std::make_shared<tinygltf::Model>();

        if (gltfModels.find(filePath) != gltfModels.end()) {
            auto& model = gltfModels[filePath];
            if (auto sharedModel = model)
            {
                gltfModel = sharedModel;
            }
        }
        // Load glTF file
        else
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

            gltfModels[filePath] = gltfModel;
        }

        for (const tinygltf::Scene& gltfScene : gltfModel->scenes) {
            Scene& scene = scenes.emplace_back();
            scene.name = gltfScene.name;
            scene.nodes = gltfScene.nodes;
        }
        defaultScene = gltfModel->defaultScene < 0 ? 0 : gltfModel->defaultScene;

        FetchNodes(*gltfModel);
        FetchMaterials(device, *gltfModel);
        FetchTextures(device, *gltfModel);

        if (staticBatching) {
            FetchBatchMeshes(device, *gltfModel);
        }
        else {
            FetchMeshes(device, *gltfModel);
            FetchAnimations(*gltfModel);
        }

        std::ofstream ofs(cerealFilePath.c_str(), std::ios::binary);
        cereal::BinaryOutputArchive serialization(ofs);
        serialization(
            cereal::make_nvp("scenes", scenes),
            cereal::make_nvp("defaultScene", defaultScene),
            cereal::make_nvp("nodes", nodes),
            cereal::make_nvp("materials", materials)
        );
        serialization(cereal::make_nvp("batchMeshes", batchMeshes));
        serialization(cereal::make_nvp("meshes", meshes));
        serialization(cereal::make_nvp("textures", textures), cereal::make_nvp("images", images));
        serialization(cereal::make_nvp("skins", skins), cereal::make_nvp("animations", animations));
    }
	// リソースの作成とアップロード
    CreateAndUploadResources(device);

    // AABBの計算
    boundingBox = CalculateAABB();
}

void GltfModelRenderer::Initialize()
{
    
}

void GltfModelRenderer::FetchNodes(const tinygltf::Model& gltfModel) {
    for (const tinygltf::Node& gltfNode : gltfModel.nodes) {
        Node& node = nodes.emplace_back();
        node.name = gltfNode.name;
        node.skin = gltfNode.skin;
        node.mesh = gltfNode.mesh;
        node.children = gltfNode.children;
        if (!gltfNode.matrix.empty()) {
            DirectX::XMFLOAT4X4 matrix;
            for (size_t row = 0; row < 4; row++) {
                for (size_t column = 0; column < 4; column++) {
                    matrix(row, column) = static_cast<float>(gltfNode.matrix.at(4 * row + column));
                }
            }

            DirectX::XMVECTOR S, R, T;
            bool succeed = DirectX::XMMatrixDecompose(&S, &R, &T, DirectX::XMLoadFloat4x4(&matrix));
            _ASSERT_EXPR(succeed, L"Failed to decompose matrix.");

            DirectX::XMStoreFloat3(&node.scale, S);
            DirectX::XMStoreFloat4(&node.rotation, R);
            DirectX::XMStoreFloat3(&node.translation, T);
        }
        else {
            if (gltfNode.scale.size() > 0)
            {
                node.scale.x = static_cast<float>(gltfNode.scale.at(0));
                node.scale.y = static_cast<float>(gltfNode.scale.at(1));
                node.scale.z = static_cast<float>(gltfNode.scale.at(2));
            }
            if (gltfNode.translation.size() > 0)
            {
                node.translation.x = static_cast<float>(gltfNode.translation.at(0));
                node.translation.y = static_cast<float>(gltfNode.translation.at(1));
                node.translation.z = static_cast<float>(gltfNode.translation.at(2));
            }
            if (gltfNode.rotation.size() > 0)
            {
                node.rotation.x = static_cast<float>(gltfNode.rotation.at(0));
                node.rotation.y = static_cast<float>(gltfNode.rotation.at(1));
                node.rotation.z = static_cast<float>(gltfNode.rotation.at(2));
                node.rotation.w = static_cast<float>(gltfNode.rotation.at(3));
            }
        }
        
        // TODO: たまたま上手くいってる可能性があるので、今後問題が出てきたらここを見直すこと
		node.translation.z = -node.translation.z; // glTFの右手系を左手系に変換
		node.rotation.x = -node.rotation.x; // glTFの右手系を左手系に変換
		node.rotation.y = -node.rotation.y; // glTFの右手系を左手系に変換
    }
    CumulateTransforms(nodes);
}
void GltfModelRenderer::CumulateTransforms(std::vector<Node>& nodes) {
    std::function<void(int, int)> traverse = [&](int parentIndex, int nodeIndex)->void
        {
            DirectX::XMMATRIX P = parentIndex > -1 ? DirectX::XMLoadFloat4x4(&nodes.at(parentIndex).globalTransform) : DirectX::XMMatrixIdentity();

            Node& node = nodes.at(nodeIndex);
            DirectX::XMMATRIX S = DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
            DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotation));
            DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(node.translation.x, node.translation.y, node.translation.z);
            DirectX::XMStoreFloat4x4(&node.globalTransform, S * R * T * P);

            for (int childIndex : node.children) {
                traverse(nodeIndex, childIndex);
            }
        };
    for (int nodeIndex : scenes.at(defaultScene).nodes) {
        traverse(-1, nodeIndex);
    }
}

DXGI_FORMAT _dxgi_format(const tinygltf::Accessor& accessor)
{
    switch (accessor.type)
    {
    case TINYGLTF_TYPE_SCALAR:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return DXGI_FORMAT_R8_UINT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return DXGI_FORMAT_R16_UINT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return DXGI_FORMAT_R32_UINT;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            return DXGI_FORMAT_UNKNOWN;
        }
    case TINYGLTF_TYPE_VEC2:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return DXGI_FORMAT_R32G32_FLOAT;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            return DXGI_FORMAT_UNKNOWN;
        }
    case TINYGLTF_TYPE_VEC3:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            return DXGI_FORMAT_UNKNOWN;
        }
    case TINYGLTF_TYPE_VEC4:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            return DXGI_FORMAT_UNKNOWN;
        }
        break;
    default:
        _ASSERT_EXPR(FALSE, L"This accessor type is not supported.");
        return DXGI_FORMAT_UNKNOWN;
    }
}
UINT _sizeof_component(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_R8_UINT: return 1;
    case DXGI_FORMAT_R16_UINT: return 2;
    case DXGI_FORMAT_R32_UINT: return 4;
    case DXGI_FORMAT_R32G32_FLOAT: return 8;
    case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
    case DXGI_FORMAT_R8G8B8A8_UINT: return 4;
    case DXGI_FORMAT_R16G16B16A16_UINT: return 8;
    case DXGI_FORMAT_R32G32B32A32_UINT: return 16;
    case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
    }
    _ASSERT_EXPR(FALSE, L"Not supported");
    return 0;
}
template<class T>
static void _copy(unsigned char* d_data, const size_t d_stride, const unsigned char* s_data, const size_t s_stride, size_t count)
{
    while (count-- > 0)
    {
        *reinterpret_cast<T*>(d_data) = *reinterpret_cast<const T*>(s_data);
        s_data += s_stride;
        d_data += d_stride;
    }
};
void GltfModelRenderer::FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel) {
    for (const tinygltf::Mesh& gltfMesh : gltfModel.meshes) {
        Mesh& mesh = meshes.emplace_back();
        mesh.name = gltfMesh.name;
        for (const tinygltf::Primitive& gltfPrimitive : gltfMesh.primitives) {
            Mesh::Primitive& primitive = mesh.primitives.emplace_back();
            primitive.material = gltfPrimitive.material;

            // Create index buffer view
            if (gltfPrimitive.indices > -1) {
                const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfPrimitive.indices);
                const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);

                primitive.indexBufferView.format = _dxgi_format(gltfAccessor);
                primitive.indexBufferView.sizeInBytes = static_cast<UINT>(gltfAccessor.count) * _sizeof_component(primitive.indexBufferView.format);
                primitive.cachedIndices.resize(primitive.indexBufferView.sizeInBytes);
                const unsigned char* data = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;

                memcpy_s(primitive.cachedIndices.data(), primitive.cachedIndices.size(), data, primitive.indexBufferView.sizeInBytes);
            }

            // Create vertex buffer view
            if (gltfPrimitive.attributes.size() > 0 && gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end()) {
                primitive.cachedVertices.resize(gltfModel.accessors.at(gltfPrimitive.attributes.at("POSITION")).count);
            }
            else {
                continue;
            }
            for (std::map<std::string, int>::const_reference gltfAttribute : gltfPrimitive.attributes) {
                const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfAttribute.second);
                const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);

                const unsigned char* s_data = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                const size_t s_stride = gltfAccessor.ByteStride(gltfBufferView);
                const size_t d_stride = sizeof(Mesh::Vertex);
                if (gltfAttribute.first == "POSITION") {
                    const size_t count = gltfAccessor.count;
                    _ASSERT_EXPR(count == primitive.cachedVertices.size(), L"The number of components on all vertices comprising the mesh must be the same.");

                    unsigned char* d_data = reinterpret_cast<unsigned char*>(&primitive.cachedVertices.data()->position);
                    _copy<DirectX::XMFLOAT3>(d_data, d_stride, s_data, s_stride, count);
                }
                else if (gltfAttribute.first == "NORMAL") {
                    const size_t count = gltfAccessor.count;
                    _ASSERT_EXPR(count == primitive.cachedVertices.size(), L"The number of components on all vertices comprising the mesh must be the same.");

                    unsigned char* d_data = reinterpret_cast<unsigned char*>(&primitive.cachedVertices.data()->normal);
                    _copy<DirectX::XMFLOAT3>(d_data, d_stride, s_data, s_stride, count);
                }
                else if (gltfAttribute.first == "TANGENT") {
                    const size_t count = gltfAccessor.count;
                    _ASSERT_EXPR(count == primitive.cachedVertices.size(), L"The number of components on all vertices comprising the mesh must be the same.");

                    unsigned char* d_data = reinterpret_cast<unsigned char*>(&primitive.cachedVertices.data()->tangent);
                    _copy<DirectX::XMFLOAT4>(d_data, d_stride, s_data, s_stride, count);
                }
                else if (gltfAttribute.first == "TEXCOORD_0") {
                    const size_t count = gltfAccessor.count;
                    _ASSERT_EXPR(count == primitive.cachedVertices.size(), L"The number of components on all vertices comprising the mesh must be the same.");

                    unsigned char* d_data = reinterpret_cast<unsigned char*>(&primitive.cachedVertices.data()->texcoord);
                    _copy<DirectX::XMFLOAT2>(d_data, d_stride, s_data, s_stride, count);
                }
                else if (gltfAttribute.first == "JOINTS_0") {
                    const size_t count = gltfAccessor.count;
                    _ASSERT_EXPR(count == primitive.cachedVertices.size(), L"The number of components on all vertices comprising the mesh must be the same.");

                    if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                    {
                        unsigned char* d_data = reinterpret_cast<unsigned char*>(&primitive.cachedVertices.data()->joints);
                        _copy<DirectX::XMINT4>(d_data, d_stride, s_data, s_stride, count);
                    }
                    else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        const USHORT* data = reinterpret_cast<const USHORT*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex)
                        {
                            primitive.cachedVertices.at(accessorIndex).joints.x = static_cast<UINT>(data[accessorIndex * 4 + 0]);
                            primitive.cachedVertices.at(accessorIndex).joints.y = static_cast<UINT>(data[accessorIndex * 4 + 1]);
                            primitive.cachedVertices.at(accessorIndex).joints.z = static_cast<UINT>(data[accessorIndex * 4 + 2]);
                            primitive.cachedVertices.at(accessorIndex).joints.w = static_cast<UINT>(data[accessorIndex * 4 + 3]);
                        }
                    }
                    else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        const BYTE* data = reinterpret_cast<const BYTE*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex)
                        {
                            primitive.cachedVertices.at(accessorIndex).joints.x = static_cast<UINT>(data[accessorIndex * 4 + 0]);
                            primitive.cachedVertices.at(accessorIndex).joints.y = static_cast<UINT>(data[accessorIndex * 4 + 1]);
                            primitive.cachedVertices.at(accessorIndex).joints.z = static_cast<UINT>(data[accessorIndex * 4 + 2]);
                            primitive.cachedVertices.at(accessorIndex).joints.w = static_cast<UINT>(data[accessorIndex * 4 + 3]);
                        }
                    }
                    else
                    {
                        _ASSERT_EXPR(FALSE, L"This component type is unsupported, please convert it yourself if necessary.");
                    }
                }
                if (gltfAttribute.first == "WEIGHTS_0") {
                    const size_t count = gltfAccessor.count;
                    _ASSERT_EXPR(count == primitive.cachedVertices.size(), L"The number of components on all vertices comprising the mesh must be the same.");

                    if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        unsigned char* d_data = reinterpret_cast<unsigned char*>(&primitive.cachedVertices.data()->weights);
                        _copy<DirectX::XMFLOAT4>(d_data, d_stride, s_data, s_stride, count);
                    }
                    else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        std::vector<FLOAT> weights_0(gltfAccessor.count * 4);
                        const USHORT* data = reinterpret_cast<const USHORT*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex) {
                            primitive.cachedVertices.at(accessorIndex).weights.x = static_cast<FLOAT>(data[accessorIndex * 4 + 0]) / 0xFFFF;
                            primitive.cachedVertices.at(accessorIndex).weights.y = static_cast<FLOAT>(data[accessorIndex * 4 + 1]) / 0xFFFF;
                            primitive.cachedVertices.at(accessorIndex).weights.z = static_cast<FLOAT>(data[accessorIndex * 4 + 2]) / 0xFFFF;
                            primitive.cachedVertices.at(accessorIndex).weights.w = static_cast<FLOAT>(data[accessorIndex * 4 + 3]) / 0xFFFF;
                        }
                    }
                    else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        std::vector<FLOAT> weights_0(gltfAccessor.count * 4);
                        const BYTE* data = reinterpret_cast<const BYTE*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex) {
                            primitive.cachedVertices.at(accessorIndex).weights.x = static_cast<FLOAT>(data[accessorIndex * 4 + 0]) / 0xFF;
                            primitive.cachedVertices.at(accessorIndex).weights.y = static_cast<FLOAT>(data[accessorIndex * 4 + 1]) / 0xFF;
                            primitive.cachedVertices.at(accessorIndex).weights.z = static_cast<FLOAT>(data[accessorIndex * 4 + 2]) / 0xFF;
                            primitive.cachedVertices.at(accessorIndex).weights.w = static_cast<FLOAT>(data[accessorIndex * 4 + 3]) / 0xFF;
                        }
                    }
                    else {
                        _ASSERT_EXPR(FALSE, L"This component type is unsupported, please convert it yourself if necessary.");
                    }
                }
                else {
                    //_ASSERT_EXPR(FALSE, L"This attribute is unsupported.");
                }
                primitive.attributes.emplace(gltfAttribute.first, _dxgi_format(gltfAccessor));
            }

            // TODO: たまたま上手くいってる可能性があるので、今後問題が出てきたらここを見直すこと
            for (Mesh::Vertex& vertex : primitive.cachedVertices) {
                // 左手系(Y-Up, Z-Forward)に変換
                vertex.position.z = -vertex.position.z;
                vertex.normal.z = -vertex.normal.z;   // normal.y ではなく z を反転
                vertex.tangent.z = -vertex.tangent.z; // タンジェントの z も反転
            }

            // 【追加】Z軸反転により面の裏表が逆転するため、インデックスの巻き順(CCW -> CW)を逆にする
            if (primitive.indexBufferView.sizeInBytes > 0)
            {
                if (primitive.indexBufferView.format == DXGI_FORMAT_R32_UINT) {
                    UINT* indices = reinterpret_cast<UINT*>(primitive.cachedIndices.data());
                    for (size_t i = 0; i < primitive.cachedIndices.size() / sizeof(UINT); i += 3) {
                        std::swap(indices[i], indices[i + 2]);
                    }
                }
                else if (primitive.indexBufferView.format == DXGI_FORMAT_R16_UINT) {
                    USHORT* indices = reinterpret_cast<USHORT*>(primitive.cachedIndices.data());
                    for (size_t i = 0; i < primitive.cachedIndices.size() / sizeof(USHORT); i += 3) {
                        std::swap(indices[i], indices[i + 2]);
                    }
                }
                else if (primitive.indexBufferView.format == DXGI_FORMAT_R8_UINT) {
                    BYTE* indices = reinterpret_cast<BYTE*>(primitive.cachedIndices.data());
                    for (size_t i = 0; i < primitive.cachedIndices.size() / sizeof(BYTE); i += 3) {
                        std::swap(indices[i], indices[i + 2]);
                    }
                }
            }

            primitive.vertexBufferView.sizeInBytes = static_cast<UINT>(primitive.cachedVertices.size() * sizeof(Mesh::Vertex));

        }

    }
}
void GltfModelRenderer::FetchBatchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel) {
    batchMeshes.resize(gltfModel.materials.size());

    std::function<void(int)> traverse = [&](int nodeIndex)->void {
        const Node& node = nodes.at(nodeIndex);
        if (node.mesh > -1) {
            const DirectX::XMMATRIX globalTransform = DirectX::XMLoadFloat4x4(&node.globalTransform);

            const tinygltf::Mesh& gltfMesh = gltfModel.meshes.at(node.mesh);

            for (const tinygltf::Primitive& gltfPrimitive : gltfMesh.primitives) {
                if (gltfPrimitive.material < 0) {
                    continue;
                }

                BatchMesh& batchMesh = batchMeshes.at(gltfPrimitive.material);
                batchMesh.material = gltfPrimitive.material;
                batchMesh.indexBufferView.format = DXGI_FORMAT_R32_UINT;
                if (gltfPrimitive.indices > -1)
                {
                    const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfPrimitive.indices);
                    const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);

                    std::vector<UINT> cachedIndices(gltfAccessor.count);
                    const size_t vertexOffset = batchMesh.cachedVertices.size();
                    if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        const BYTE* data = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex) {
                            cachedIndices.at(accessorIndex) = static_cast<UINT>(data[accessorIndex] + vertexOffset);
                        }
                    }
                    else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        const USHORT* data = reinterpret_cast<const USHORT*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex)
                        {
                            cachedIndices.at(accessorIndex) = static_cast<UINT>(data[accessorIndex] + vertexOffset);
                        }
                    }
                    else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                    {
                        const UINT* data = reinterpret_cast<const UINT*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex)
                        {
                            cachedIndices.at(accessorIndex) = static_cast<UINT>(data[accessorIndex] + vertexOffset);
                        }
                    }
                    else
                    {
                        _ASSERT_EXPR(false, L"This index format is not supported.");
                    }

                    batchMesh.cachedIndices.insert(batchMesh.cachedIndices.end(), cachedIndices.begin(), cachedIndices.end());
                    batchMesh.indexBufferView.sizeInBytes += static_cast<UINT>(gltfAccessor.count * sizeof(UINT));
                }

                std::vector<BatchMesh::Vertex> cachedVertices;
                if (gltfPrimitive.attributes.size() > 0 && gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end()) {
                    cachedVertices.resize(gltfModel.accessors.at(gltfPrimitive.attributes.at("POSITION")).count);
                }
                else {
                    continue;
                }

                for (std::map<std::string, int>::const_reference gltfAttribute : gltfPrimitive.attributes) {
                    const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfAttribute.second);
                    const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);

                    const unsigned char* s_data = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                    const size_t s_stride = gltfAccessor.ByteStride(gltfBufferView);
                    const size_t d_stride = sizeof(BatchMesh::Vertex);
                    const size_t count = gltfAccessor.count;
                    _ASSERT_EXPR(count == cachedVertices.size(), L"The number of components on all vertices comprising the mesh must be the same.");
                    if (gltfAttribute.first == "POSITION") {
                        unsigned char* d_data = reinterpret_cast<unsigned char*>(&cachedVertices.data()->position);
                        _copy<DirectX::XMFLOAT3>(d_data, d_stride, s_data, s_stride, count);
                    }
                    else if (gltfAttribute.first == "NORMAL") {
                        unsigned char* d_data = reinterpret_cast<unsigned char*>(&cachedVertices.data()->normal);
                        _copy<DirectX::XMFLOAT3>(d_data, d_stride, s_data, s_stride, count);
                    }
                    else if (gltfAttribute.first == "TANGENT") {
                        unsigned char* d_data = reinterpret_cast<unsigned char*>(&cachedVertices.data()->tangent);
                        _copy<DirectX::XMFLOAT4>(d_data, d_stride, s_data, s_stride, count);
                    }
                    else if (gltfAttribute.first == "TEXCOORD_0") {
                        unsigned char* d_data = reinterpret_cast<unsigned char*>(&cachedVertices.data()->texcoord);
                        _copy<DirectX::XMFLOAT2>(d_data, d_stride, s_data, s_stride, count);
                    }
                    else {
                        //_ASSERT_EXPR(FALSE, L"This attribute is unsupported.");
                        OutputDebugStringA((gltfAttribute.first + " is an unsupported attribute.\n").c_str());
                    }
                    batchMesh.attributes.emplace(gltfAttribute.first, _dxgi_format(gltfAccessor));
                }

                for (BatchMesh::Vertex& cachedVertex : cachedVertices) {
                    DirectX::XMStoreFloat3(&cachedVertex.position, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&cachedVertex.position), globalTransform));
                    DirectX::XMStoreFloat3(&cachedVertex.normal, DirectX::XMVector3Normalize(DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&cachedVertex.normal), globalTransform)));
                    float sigma = cachedVertex.tangent.w;
                    cachedVertex.tangent.w = 0;
                    DirectX::XMStoreFloat4(&cachedVertex.tangent, DirectX::XMVector3Normalize(DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat4(&cachedVertex.tangent), globalTransform)));
                    cachedVertex.tangent.w = sigma;

                    // TODO: たまたま上手くいってる可能性があるので、今後問題が出てきたらここを見直すこと
                    // 左手系(Y-Up, Z-Forward)に変換
                    cachedVertex.position.z = -cachedVertex.position.z;
                    cachedVertex.normal.z = -cachedVertex.normal.z;   // normal.y ではなく z を反転
                    cachedVertex.tangent.z = -cachedVertex.tangent.z; // タンジェントも反転
                }

                // 【追加】インデックスの巻き順を逆にする
				std::vector<UINT>& cachedIndices = batchMesh.cachedIndices;
                for (size_t i = 0; i < cachedIndices.size(); i += 3) {
                    std::swap(cachedIndices[i], cachedIndices[i + 2]);
                }

                batchMesh.cachedVertices.insert(batchMesh.cachedVertices.end(), cachedVertices.begin(), cachedVertices.end());
                batchMesh.vertexBufferView.sizeInBytes += static_cast<UINT>(cachedVertices.size() * sizeof(BatchMesh::Vertex));
            }
        }
        for (std::vector<int>::value_type childIndex : node.children) {
            traverse(childIndex);
        }
        };
    for (std::vector<int>::value_type nodeIndex : scenes.at(defaultScene).nodes) {
        traverse(nodeIndex);
    }
}

void GltfModelRenderer::FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel) {
    for (const tinygltf::Material& gltfMaterial : gltfModel.materials) {
        std::vector<Material>::reference material = materials.emplace_back();

        material.name = gltfMaterial.name;

        material.data.emissiveFactor[0] = static_cast<float>(gltfMaterial.emissiveFactor.at(0));
        material.data.emissiveFactor[1] = static_cast<float>(gltfMaterial.emissiveFactor.at(1));
        material.data.emissiveFactor[2] = static_cast<float>(gltfMaterial.emissiveFactor.at(2));

        material.data.alphaMode = gltfMaterial.alphaMode == "OPAQUE" ? 0 : gltfMaterial.alphaMode == "MASK" ? 1 : gltfMaterial.alphaMode == "BLEND" ? 2 : 0;
        material.data.alphaCutoff = static_cast<float>(gltfMaterial.alphaCutoff);
        material.data.doubleSided = gltfMaterial.doubleSided ? 1 : 0;

        material.data.pbrMetallicRoughness.baseColorFactor[0] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(0));
        material.data.pbrMetallicRoughness.baseColorFactor[1] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(1));
        material.data.pbrMetallicRoughness.baseColorFactor[2] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(2));
        material.data.pbrMetallicRoughness.baseColorFactor[3] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(3));
        material.data.pbrMetallicRoughness.baseColorTexture.index = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
        material.data.pbrMetallicRoughness.baseColorTexture.texcoord = gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord;
        material.data.pbrMetallicRoughness.metallicFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
        material.data.pbrMetallicRoughness.roughnessFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
        material.data.pbrMetallicRoughness.metallicRoughnessTexture.index = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
        material.data.pbrMetallicRoughness.metallicRoughnessTexture.texcoord = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

        material.data.normalTexture.index = gltfMaterial.normalTexture.index;
        material.data.normalTexture.texcoord = gltfMaterial.normalTexture.texCoord;
        material.data.normalTexture.scale = static_cast<float>(gltfMaterial.normalTexture.scale);

        material.data.occlusionTexture.index = gltfMaterial.occlusionTexture.index;
        material.data.occlusionTexture.texcoord = gltfMaterial.occlusionTexture.texCoord;
        material.data.occlusionTexture.strength = static_cast<float>(gltfMaterial.occlusionTexture.strength);

        material.data.emissiveTexture.index = gltfMaterial.emissiveTexture.index;
        material.data.emissiveTexture.texcoord = gltfMaterial.emissiveTexture.texCoord;
    }
}

void GltfModelRenderer::FetchTextures(ID3D11Device* device, const tinygltf::Model& gltfModel) {
    for (const tinygltf::Texture& gltfTexture : gltfModel.textures) {
        Texture& texture = textures.emplace_back();
        texture.name = gltfTexture.name;
        texture.source = gltfTexture.source;
    }
    for (const tinygltf::Image& gltfImage : gltfModel.images) {
        Image& image = images.emplace_back();
        image.name = gltfImage.name;
        image.width = gltfImage.width;
        image.height = gltfImage.height;
        image.component = gltfImage.component;
        image.bits = gltfImage.bits;
        image.pixelType = gltfImage.pixel_type;
        image.mimeType = gltfImage.mimeType;
        image.uri = gltfImage.uri;
        image.asIs = gltfImage.as_is;

        if (gltfImage.bufferView > -1) {
            const tinygltf::BufferView& bufferView = gltfModel.bufferViews.at(gltfImage.bufferView);
            const tinygltf::Buffer& buffer = gltfModel.buffers.at(bufferView.buffer);
            const unsigned char* data = buffer.data.data() + bufferView.byteOffset;
            image.cacheData.resize(bufferView.byteLength);
            memcpy_s(image.cacheData.data(), image.cacheData.size(), data, bufferView.byteLength);
        }
    }
}

void GltfModelRenderer::FetchAnimations(const tinygltf::Model& gltfModel) {
    for (const tinygltf::Skin& transmissionSkin : gltfModel.skins) {
        Skin& skin = skins.emplace_back();
        const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(transmissionSkin.inverseBindMatrices);
        const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);
        _ASSERT_EXPR(gltfAccessor.type == TINYGLTF_TYPE_MAT4, L"");

        skin.inverseBindMatrices.resize(gltfAccessor.count);
        memcpy(skin.inverseBindMatrices.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT4X4));

        skin.joints = transmissionSkin.joints;
    }

    for (const tinygltf::Animation& gltfAnimation : gltfModel.animations) {
        Animation& animation = animations.emplace_back();
        animation.name = gltfAnimation.name;
        for (const tinygltf::AnimationSampler& gltfSampler : gltfAnimation.samplers) {
            Animation::Sampler& sampler = animation.samplers.emplace_back();
            sampler.input = gltfSampler.input;
            sampler.output = gltfSampler.output;
            sampler.interpolation = gltfSampler.interpolation;

            const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfSampler.input);
            const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);
            _ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, L"");
            _ASSERT_EXPR(gltfAccessor.type == TINYGLTF_TYPE_SCALAR, L"");
            const std::pair<std::unordered_map<int, std::vector<float>>::iterator, bool>& timelines = animation.timelines.emplace(gltfSampler.input, gltfAccessor.count);
            if (timelines.second) {
                memcpy(timelines.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(FLOAT));
            }
        }
        for (const tinygltf::AnimationChannel& gltfChannel : gltfAnimation.channels) {
            Animation::Channel& channel = animation.channels.emplace_back();
            channel.sampler = gltfChannel.sampler;
            channel.targetNode = gltfChannel.target_node;
            channel.targetPath = gltfChannel.target_path;

            const tinygltf::AnimationSampler& gltfSampler = gltfAnimation.samplers.at(gltfChannel.sampler);
            const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfSampler.output);
            const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);
            if (gltfChannel.target_path == "scale") {
                _ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, L"");
                _ASSERT_EXPR(gltfAccessor.type == TINYGLTF_TYPE_VEC3, L"");

                const std::pair<std::unordered_map<int, std::vector<DirectX::XMFLOAT3>>::iterator, bool>& scales = animation.scales.emplace(gltfSampler.output, gltfAccessor.count);
                if (scales.second) {
                    memcpy(scales.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT3));
                }
            }
            else if (gltfChannel.target_path == "rotation") {
                _ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, L"");
                _ASSERT_EXPR(gltfAccessor.type == TINYGLTF_TYPE_VEC4, L"");

                const std::pair<std::unordered_map<int, std::vector<DirectX::XMFLOAT4>>::iterator, bool>& rotations = animation.rotations.emplace(gltfSampler.output, gltfAccessor.count);
                if (rotations.second) {
                    memcpy(rotations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT4));
                }
            }
            else if (gltfChannel.target_path == "translation") {
                _ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, L"");
                _ASSERT_EXPR(gltfAccessor.type == TINYGLTF_TYPE_VEC3, L"");

                const std::pair<std::unordered_map<int, std::vector<DirectX::XMFLOAT3>>::iterator, bool>& translations = animation.translations.emplace(gltfSampler.output, gltfAccessor.count);
                if (translations.second) {
                    memcpy(translations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT3));
                }
            }
            else if (gltfChannel.target_path == "weights") {
                //_ASSERT_EXPR(FALSE, L"");
            }
            else {
                _ASSERT_EXPR(FALSE, L"");
            }
        }
    }
    // Find a longest animations duration in timeline of each channel.
    for (decltype(animations)::reference animation : animations)
    {
        for (decltype(animation.timelines)::reference timelines : animation.timelines)
        {
            animation.duration = std::max<float>(animation.duration, timelines.second.back());
        }
    }
}
void GltfModelRenderer::Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes) {
    _ASSERT_EXPR(animations.size() > animationIndex, L"");
    _ASSERT_EXPR(animatedNodes.size() == nodes.size(), L"");

    std::function<size_t(const std::vector<float>&, float, float&)> indexof = [](const std::vector<float>& timelines, float time, float& interpolationFactor)->size_t {
        const size_t keyframeCount = timelines.size();
        if (time > timelines.at(keyframeCount - 1)) {
            interpolationFactor = 1.0f;
            return keyframeCount - 2;
        }
        else if (time < timelines.at(0)) {
            interpolationFactor = 0.0f;
            return 0;
        }
        size_t keyframeIndex = 0;
        for (size_t timeIndex = 1; timeIndex < keyframeCount; ++timeIndex) {
            if (time < timelines.at(timeIndex)) {
                keyframeIndex = std::max<size_t>(0LL, timeIndex - 1);
                break;
            }
        }
        interpolationFactor = (time - timelines.at(keyframeIndex + 0)) / (timelines.at(keyframeIndex + 1) - timelines.at(keyframeIndex + 0));
        return keyframeIndex;
        };
    // アニメーションが存在している場合のみ処理
    if (animations.size() > 0)
    {
        //追加
        float blendRate = 1.0f;
        if (isBlendStart && time < animationBlendTime) {
            blendRate = time / animationBlendTime;
            blendRate *= blendRate;
        }

        const Animation& animation{ animations.at(animationIndex) };

        // アニメーションの各チャネルを処理
        for (vector<Animation::Channel>::const_reference channel : animation.channels)
        {
            const Animation::Sampler& sampler{ animation.samplers.at(channel.sampler) };
            const vector<float>& timeline{ animation.timelines.at(sampler.input) };

            // キーフレームがなければスキップ
            if (timeline.size() == 0)
            {
                continue;
            }

            float interpolationFactor{};
            size_t keyframeIndex{ indexof(timeline, time, interpolationFactor) };

            float rate = blendRate < 1.f ? blendRate : interpolationFactor;

            // 対象のプロパティ（スケール・回転・位置）に応じて補間と適用を行う
            if (channel.targetPath == "scale")
            {
                const vector<XMFLOAT3>& scales{ animation.scales.at(sampler.output) };

                XMVECTOR S0 = XMLoadFloat3((blendRate < 1.f) ? &animatedNodes.at(channel.targetNode).scale : &scales.at(keyframeIndex + 0));
                XMVECTOR S1 = XMLoadFloat3(&scales.at(keyframeIndex + 1));

                // 線形補間でスケールを求めてノードに格納
                XMStoreFloat3(&animatedNodes.at(channel.targetNode).scale,
                    XMVectorLerp(S0, S1, rate));
            }
            else if (channel.targetPath == "rotation")
            {
                const vector<XMFLOAT4>& rotations{ animation.rotations.at(sampler.output) };

                XMVECTOR R0 = XMLoadFloat4((blendRate < 1.f) ? &animatedNodes.at(channel.targetNode).rotation : &rotations.at(keyframeIndex + 0));
                XMVECTOR R1 = XMLoadFloat4(&rotations.at(keyframeIndex + 1));

                // 球面線形補間（Slerp）で回転を補間し、正規化して適用
                XMStoreFloat4(&animatedNodes.at(channel.targetNode).rotation,
                    XMQuaternionNormalize(XMQuaternionSlerp(R0, R1, rate)));
            }
            else if (channel.targetPath == "translation")
            {
                const vector<XMFLOAT3>& translations{ animation.translations.at(sampler.output) };

                XMVECTOR T0 = XMLoadFloat3((blendRate < 1.f) ? &animatedNodes.at(channel.targetNode).translation : &translations.at(keyframeIndex + 0));
                XMVECTOR T1 = XMLoadFloat3(&translations.at(keyframeIndex + 1));

                // 線形補間で位置を求めてノードに格納
                XMStoreFloat3(&animatedNodes.at(channel.targetNode).translation,
                    XMVectorLerp(T0, T1, rate));
            }
            else if (channel.targetPath == "weight") {

            }
            else {

            }
        }
        // アニメーション後にノードのワールド変換を更新
        CumulateTransforms(animatedNodes);
    }
}

void GltfModelRenderer::CreateAndUploadResources(ID3D11Device* device) {
    HRESULT hr;
    D3D11_BUFFER_DESC bufferDesc{};
    D3D11_SUBRESOURCE_DATA subResourceData{};

	// Clear previous resources
	buffers.clear();
    textureResourceViews.clear();

    // Create and upload vertex and index buffers on GPU
    if (staticBatching) {
        for (BatchMesh& batchMesh : batchMeshes)
        {
            if (batchMesh.indexBufferView.sizeInBytes > 0)
            {
                batchMesh.indexBufferView.buffer = static_cast<int>(buffers.size());
                bufferDesc.ByteWidth = batchMesh.indexBufferView.sizeInBytes;
                bufferDesc.Usage = D3D11_USAGE_DEFAULT;
                bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                bufferDesc.CPUAccessFlags = 0;
                bufferDesc.MiscFlags = 0;
                bufferDesc.StructureByteStride = 0;
                subResourceData.pSysMem = batchMesh.cachedIndices.data();
                subResourceData.SysMemPitch = 0;
                subResourceData.SysMemSlicePitch = 0;
                hr = device->CreateBuffer(&bufferDesc, &subResourceData, buffers.emplace_back().GetAddressOf());
                _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

#ifndef _DEBUG
                //batchMesh.cachedIndices.clear();
#endif // !_DEBUG
            }

            if (batchMesh.vertexBufferView.sizeInBytes > 0)
            {
                batchMesh.vertexBufferView.buffer = static_cast<int>(buffers.size());
                bufferDesc.ByteWidth = batchMesh.vertexBufferView.sizeInBytes;
                bufferDesc.Usage = D3D11_USAGE_DEFAULT;
                bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                bufferDesc.CPUAccessFlags = 0;
                bufferDesc.MiscFlags = 0;
                bufferDesc.StructureByteStride = 0;
                subResourceData.pSysMem = batchMesh.cachedVertices.data();
                subResourceData.SysMemPitch = 0;
                subResourceData.SysMemSlicePitch = 0;
                hr = device->CreateBuffer(&bufferDesc, &subResourceData, buffers.emplace_back().GetAddressOf());
                _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

#ifndef _DEBUG
                //batchMesh.cachedVertices.clear();
#endif // !_DEBUG

            }
        }
    }
    else
    {
        for (Mesh& mesh : meshes)
        {
            for (Mesh::Primitive& primitive : mesh.primitives)
            {
                if (primitive.indexBufferView.sizeInBytes > 0)
                {
                    primitive.indexBufferView.buffer = static_cast<int>(buffers.size());
                    bufferDesc.ByteWidth = primitive.indexBufferView.sizeInBytes;
                    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
                    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                    bufferDesc.CPUAccessFlags = 0;
                    bufferDesc.MiscFlags = 0;
                    bufferDesc.StructureByteStride = 0;
                    subResourceData.pSysMem = primitive.cachedIndices.data();
                    subResourceData.SysMemPitch = 0;
                    subResourceData.SysMemSlicePitch = 0;
                    hr = device->CreateBuffer(&bufferDesc, &subResourceData, buffers.emplace_back().GetAddressOf());
                    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

#ifndef _DEBUG
                    //primitive.cachedIndices.clear();
#endif // !_DEBUG
                }

                if (primitive.vertexBufferView.sizeInBytes > 0)
                {
                    primitive.vertexBufferView.buffer = static_cast<int>(buffers.size());
                    bufferDesc.ByteWidth = primitive.vertexBufferView.sizeInBytes;
                    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
                    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                    bufferDesc.CPUAccessFlags = 0;
                    bufferDesc.MiscFlags = 0;
                    bufferDesc.StructureByteStride = 0;
                    subResourceData.pSysMem = primitive.cachedVertices.data();
                    subResourceData.SysMemPitch = 0;
                    subResourceData.SysMemSlicePitch = 0;
                    hr = device->CreateBuffer(&bufferDesc, &subResourceData, buffers.emplace_back().GetAddressOf());
                    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

#ifndef _DEBUG
                    //primitive.cachedVertices.clear();
#endif // !_DEBUG
                }
            }
        }
    }

    // Create and upload materials on GPU
    std::vector<Material::CBuffer> materialData;
    for (const Material& material : materials) {
        materialData.emplace_back(material.data);
    }
    Microsoft::WRL::ComPtr<ID3D11Buffer> materialBuffer;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Material::CBuffer) * materialData.size());
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(Material::CBuffer);
    subResourceData.pSysMem = materialData.data();
    subResourceData.SysMemPitch = 0;
    subResourceData.SysMemSlicePitch = 0;
    hr = device->CreateBuffer(&bufferDesc, &subResourceData, materialBuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
    shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(materialData.size());
    hr = device->CreateShaderResourceView(materialBuffer.Get(), &shaderResourceViewDesc, materialResourceView.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

    // Create and upload textures on GPU
    for (Image& image : images) {
        if (image.cacheData.size() > 0) {
            ID3D11ShaderResourceView* textureResourceView = NULL;
            hr = LoadTextureFromMemory(device, image.cacheData.data(), image.cacheData.size(), &textureResourceView);
            if (hr == S_OK) {
                textureResourceViews.emplace_back().Attach(textureResourceView);
            }
            image.cacheData.clear();
        }
        else {
            const std::filesystem::path path(filePath);
            ID3D11ShaderResourceView* shaderResourceView = NULL;
            std::wstring filePath = path.parent_path().concat(L"/").wstring() + std::wstring(image.uri.begin(), image.uri.end());
            hr = LoadTextureFromFile(device, filePath.c_str(), &shaderResourceView, NULL);
            if (hr == S_OK) {
                textureResourceViews.emplace_back().Attach(shaderResourceView);
            }
        }
    }

    std::string dir = EnginePaths::ShadersDataDir;
    if (staticBatching) {
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        CreateVertexShaderFromCSO(device, (dir + "gltf_model_static_batching_vs.cso").c_str(), vertexShader.ReleaseAndGetAddressOf(), inputLayout.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
    }
    else {
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"JOINTS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        CreateVertexShaderFromCSO(device, (dir + "gltf_model_vs.cso").c_str(), vertexShader.ReleaseAndGetAddressOf(), inputLayout.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
    }
    CreatePixelShaderFromCSO(device, (dir + "gltf_model_ps.cso").c_str(), pixelShader.ReleaseAndGetAddressOf());

    //CascadedShadowMaps
    CreateVertexShaderFromCSO(device, (dir + "gltf_model_csm_vs.cso").c_str(), vertexShaderCsm.ReleaseAndGetAddressOf(), NULL, NULL, 0);
    CreateGeometryShaderFromCSO(device, (dir + "gltf_model_csm_gs.cso").c_str(), geometryShaderCsm.ReleaseAndGetAddressOf());

    bufferDesc.ByteWidth = sizeof(PrimitiveConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, primitiveCbuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

    bufferDesc.ByteWidth = sizeof(PrimitiveJointConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, primitiveJointCbuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	if (Renderer::material == nullptr)
    {
        Renderer::material = std::make_shared<::Material>();
        // Set default material
        //material->SetShaderOnly();

        // Set default shader
        std::shared_ptr<PixelShader> ps = ResourceManager::GetShader<PixelShader>("gltf_model_ps");
        material->SetShader(device, ps);
    }
	// Set not bind cbuffer
    material->SetNotBindCBuffer({
            "SCENE_CONSTANT_BUFFER", "LIGHT_CONSTANT_BUFFER", "PRIMITIVE_CONSTANT_BUFFER",
        });
}

void GltfModelRenderer::Update(float deltaTime)
{
#if 1
    if (animations.size() <= animationIndex) return;//アニメーションが無かったらスルー

	if (!IsAnimationEnable()) return;//アニメーションが無効ならスルー

	// ノードが存在している場合のみ処理
    if (nodes.size() > 0)
    {
		// アニメーションの更新
        Animate(animationIndex, time += (deltaTime * timeRate), nodes);

		// アニメーションの時間を取得
		float animationDuration = animations.at(animationIndex).duration;

		// アニメーションイベントの処理
        for (AnimationEvent::Event& event : animationEvent.events)
        {
			// イベントがまだ発火していない場合
            if (!event.isCalled)
            {
                // イベント発火時間に達したら
                if (time >= event.time)
                {
                    // イベントコールバック関数を呼び出す
                    if (event.func)
                    {
                        event.func();
                    }
                    // イベント発火済みにする
                    event.isCalled = true;
                }
			}
		}

        //アニメーションが最後に到達したら
        if (animationDuration < time)
        {
            if (loop) {
                time = 0;
                isBlendStart = false;
            }
            else {
                isAnimationCompleted = true;
            }
        }
    }
#else
    if (animations.size() <= animationIndex) return;

    if (nodes.size() > 0)
    {
        time += (deltaTime * timeRate);

        if (useRange)
        {
            if (time < rangeStart)
            {
                time = rangeStart;
            }
            if (time > rangeEnd)
            {
                if (loop)
                {
                    time = rangeStart;
                    isBlendStart = false;
                }
                else
                {
                    isAnimationCompleted = true;
                    return;
                }
            }
        }

        Animate(animationIndex, time, nodes);

        if (!useRange && animations.at(animationIndex).duration < time) {
            if (loop) {
                time = 0;
                isBlendStart = false;
            }
            else {
                isAnimationCompleted = true;
            }
        }
    }
#endif
}

void GltfModelRenderer::Render(RenderContext* rtx)
{
	// VertexシェーダーとPixelシェーダーが存在しない場合は処理を抜ける
    if (vertexShader == nullptr || pixelShader == nullptr)
    {
        return;
	}

    // Pre Render
    if (preRenderFunc)
    {
        preRenderFunc(rtx);
    }

    ID3D11DeviceContext* immediateContext = rtx->immediateContext;

    immediateContext->PSSetShaderResources(0, 1, materialResourceView.GetAddressOf());

    immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);
    immediateContext->IASetInputLayout(inputLayout.Get());
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if (Renderer::material != nullptr) {
        Renderer::material->Apply(rtx);
    }

    if (staticBatching) {

        for (const BatchMesh& batchMesh : batchMeshes) {
            UINT stride = sizeof(BatchMesh::Vertex);
            UINT offset = 0;
            immediateContext->IASetVertexBuffers(0, 1, buffers.at(batchMesh.vertexBufferView.buffer).GetAddressOf(), &stride, &offset);

            PrimitiveConstants primitiveData = {};
            primitiveData.material = batchMesh.material;
            primitiveData.hasTangent = batchMesh.has("TANGENT");
            primitiveData.skin = -1;
            // TODO: コンポーネントの設計が変わったら変える
            primitiveData.world = GetOwner()->transform->GetWorld();


            immediateContext->UpdateSubresource(primitiveCbuffer.Get(), 0, 0, &primitiveData, 0, 0);
            immediateContext->VSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
            immediateContext->PSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());

            const Material& material = materials.at(batchMesh.material);
            const int textureIndices[] = {
                material.data.pbrMetallicRoughness.baseColorTexture.index,
                material.data.pbrMetallicRoughness.metallicRoughnessTexture.index,
                material.data.normalTexture.index,
                material.data.emissiveTexture.index,
                material.data.occlusionTexture.index,
            };
            ID3D11ShaderResourceView* nullShaderResourceView{};
            std::vector<ID3D11ShaderResourceView*> shaderResourceViews(_countof(textureIndices));
            for (int textureIndex = 0; textureIndex < shaderResourceViews.size(); ++textureIndex) {
                shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ? textureResourceViews.at(textures.at(textureIndices[textureIndex]).source).Get() : nullShaderResourceView;
            }
            immediateContext->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());

            if (batchMesh.indexBufferView.buffer > -1) {
                // INTERLEAVED_GLTF_MODEL
                immediateContext->IASetIndexBuffer(buffers.at(batchMesh.indexBufferView.buffer).Get(), batchMesh.indexBufferView.format, 0);
                immediateContext->DrawIndexed(batchMesh.indexBufferView.sizeInBytes / _sizeof_component(batchMesh.indexBufferView.format), 0, 0);
            }
            else {
                // INTERLEAVED_GLTF_MODEL
                immediateContext->Draw(batchMesh.vertexBufferView.sizeInBytes / batchMesh.vertexBufferView.strideInBytes, 0);
            }
        }
    }
    else
    {
        //nodes
        //const std::vector<node>& nodes = animated_nodes.size() > 0 ? animated_nodes : interleaved_gltf_model::nodes;

        std::function<void(int)> traverse = [&](int nodeIndex)->void {
            const Node& node = nodes.at(nodeIndex);
            if (node.skin > -1) {
                const Skin& skin = skins.at(node.skin);
                _ASSERT_EXPR(skin.joints.size() <= PRIMITIVE_MAX_JOINTS, L"The size of the joint array is insufficient, please expand it.");
                PrimitiveJointConstants primitiveJointData{};
                for (size_t jointIndex = 0; jointIndex < skin.joints.size(); ++jointIndex) {

                    DirectX::XMFLOAT4X4 skin_inverse_bind_matrix;
                    DirectX::XMFLOAT4X4 node_global_transform;
                    DirectX::XMFLOAT4X4 joint_global_transform;
                    DirectX::XMStoreFloat4x4(&skin_inverse_bind_matrix, DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices.at(jointIndex)));
                    DirectX::XMStoreFloat4x4(&node_global_transform, DirectX::XMLoadFloat4x4(&nodes.at(skin.joints.at(jointIndex)).globalTransform));
                    DirectX::XMStoreFloat4x4(&joint_global_transform, DirectX::XMLoadFloat4x4(&node.globalTransform));

                    DirectX::XMStoreFloat4x4(&primitiveJointData.matrices[jointIndex],
                        DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices.at(jointIndex)) *
                        DirectX::XMLoadFloat4x4(&nodes.at(skin.joints.at(jointIndex)).globalTransform) *
                        DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&node.globalTransform))
                    );
                }
                immediateContext->UpdateSubresource(primitiveJointCbuffer.Get(), 0, 0, &primitiveJointData, 0, 0);
                immediateContext->VSSetConstantBuffers(6, 1, primitiveJointCbuffer.GetAddressOf());
            }
            if (node.mesh > -1) {
                const Mesh& mesh = meshes.at(node.mesh);
                for (const Mesh::Primitive& primitive : mesh.primitives) {
                    // INTERLEAVED_GLTF_MODEL
                    UINT stride = sizeof(Mesh::Vertex);
                    UINT offset = 0;
                    immediateContext->IASetVertexBuffers(0, 1, buffers.at(primitive.vertexBufferView.buffer).GetAddressOf(), &stride, &offset);

                    PrimitiveConstants primitiveData = {};
                    primitiveData.material = primitive.material;
                    primitiveData.hasTangent = primitive.has("TANGENT");
                    primitiveData.skin = node.skin;
                    

                    DirectX::XMFLOAT4X4 worldMatrix = GetOwner()->transform->GetWorld();
                    // TODO: コンポーネントの設計が変わったら変える
                    DirectX::XMStoreFloat4x4(&primitiveData.world, DirectX::XMLoadFloat4x4(&node.globalTransform) * DirectX::XMLoadFloat4x4(&worldMatrix));
                    immediateContext->UpdateSubresource(primitiveCbuffer.Get(), 0, 0, &primitiveData, 0, 0);
                    immediateContext->VSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
                    immediateContext->PSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());

                    const Material& material = materials.at(primitive.material);
                    const int textureIndices[] = {
                        material.data.pbrMetallicRoughness.baseColorTexture.index,
                        material.data.pbrMetallicRoughness.metallicRoughnessTexture.index,
                        material.data.normalTexture.index,
                        material.data.emissiveTexture.index,
                        material.data.occlusionTexture.index,
                    };
                    ID3D11ShaderResourceView* nullShaderResourceView{};
                    std::vector<ID3D11ShaderResourceView*> shaderResourceViews(_countof(textureIndices));
                    for (int textureIndex = 0; textureIndex < shaderResourceViews.size(); ++textureIndex) {
                        shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ? textureResourceViews.at(textures.at(textureIndices[textureIndex]).source).Get() : nullShaderResourceView;
                    }
                    immediateContext->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());

                    if (primitive.indexBufferView.buffer > -1) {
                        // INTERLEAVED_GLTF_MODEL
                        immediateContext->IASetIndexBuffer(buffers.at(primitive.indexBufferView.buffer).Get(), primitive.indexBufferView.format, 0);
                        immediateContext->DrawIndexed(primitive.indexBufferView.sizeInBytes / _sizeof_component(primitive.indexBufferView.format), 0, 0);
                    }
                    else {
                        // INTERLEAVED_GLTF_MODEL
                        immediateContext->Draw(primitive.vertexBufferView.sizeInBytes / primitive.vertexBufferView.strideInBytes, 0);
                    }
                }
            }
            for (std::vector<int>::value_type childIndex : node.children) {
                traverse(childIndex);
            }
            };
        for (std::vector<int>::value_type nodeIndex : scenes.at(defaultScene).nodes) {
            traverse(nodeIndex);
        }
    }

    // Post Render
    if (postRenderFunc)
    {
        postRenderFunc(rtx);
    }
}

void GltfModelRenderer::CastShadow(RenderContext* rtx)
{
    ID3D11DeviceContext* immediateContext = rtx->immediateContext;

    immediateContext->PSSetShaderResources(0, 1, materialResourceView.GetAddressOf());

    immediateContext->VSSetShader(vertexShaderCsm.Get(), NULL, 0);
    immediateContext->GSSetShader(geometryShaderCsm.Get(), NULL, 0);
    immediateContext->PSSetShader(NULL, NULL, 0);

    immediateContext->IASetInputLayout(inputLayout.Get());
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if (staticBatching) {

        for (const BatchMesh& batchMesh : batchMeshes) {
            UINT stride = sizeof(BatchMesh::Vertex);
            UINT offset = 0;
            immediateContext->IASetVertexBuffers(0, 1, buffers.at(batchMesh.vertexBufferView.buffer).GetAddressOf(), &stride, &offset);

            PrimitiveConstants primitiveData = {};
            primitiveData.material = batchMesh.material;
            primitiveData.hasTangent = batchMesh.has("TANGENT");
            primitiveData.skin = -1;
            // TODO: コンポーネントの設計が変わったら変える
            primitiveData.world = GetOwner()->transform->GetWorld();
            immediateContext->UpdateSubresource(primitiveCbuffer.Get(), 0, 0, &primitiveData, 0, 0);
            immediateContext->VSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
            immediateContext->PSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
            
            if (batchMesh.indexBufferView.buffer > -1) {
                // INTERLEAVED_GLTF_MODEL
                immediateContext->IASetIndexBuffer(buffers.at(batchMesh.indexBufferView.buffer).Get(), batchMesh.indexBufferView.format, 0);
                immediateContext->DrawIndexedInstanced(batchMesh.indexBufferView.sizeInBytes / _sizeof_component(batchMesh.indexBufferView.format), 4, 0, 0, 0);
            }
            else {
                // INTERLEAVED_GLTF_MODEL
                immediateContext->DrawInstanced(batchMesh.vertexBufferView.sizeInBytes / batchMesh.vertexBufferView.strideInBytes, 4, 0, 0);
            }
        }
    }
    else {
        //nodes
        //const std::vector<node>& nodes = animated_nodes.size() > 0 ? animated_nodes : interleaved_gltf_model::nodes;

        std::function<void(int)> traverse = [&](int nodeIndex)->void {
            const Node& node = nodes.at(nodeIndex);
            if (node.skin > -1) {
                const Skin& skin = skins.at(node.skin);
                _ASSERT_EXPR(skin.joints.size() <= PRIMITIVE_MAX_JOINTS, L"The size of the joint array is insufficient, please expand it.");
                PrimitiveJointConstants primitiveJointData{};
                for (size_t jointIndex = 0; jointIndex < skin.joints.size(); ++jointIndex) {
                    DirectX::XMStoreFloat4x4(&primitiveJointData.matrices[jointIndex],
                        DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices.at(jointIndex)) *
                        DirectX::XMLoadFloat4x4(&nodes.at(skin.joints.at(jointIndex)).globalTransform) *
                        DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&node.globalTransform))
                    );
                }
                immediateContext->UpdateSubresource(primitiveJointCbuffer.Get(), 0, 0, &primitiveJointData, 0, 0);
                immediateContext->VSSetConstantBuffers(6, 1, primitiveJointCbuffer.GetAddressOf());
            }
            if (node.mesh > -1) {
                const Mesh& mesh = meshes.at(node.mesh);
                for (const Mesh::Primitive& primitive : mesh.primitives) {
                    // INTERLEAVED_GLTF_MODEL
                    UINT stride = sizeof(Mesh::Vertex);
                    UINT offset = 0;
                    immediateContext->IASetVertexBuffers(0, 1, buffers.at(primitive.vertexBufferView.buffer).GetAddressOf(), &stride, &offset);

                    PrimitiveConstants primitiveData = {};
                    primitiveData.material = primitive.material;
                    primitiveData.hasTangent = primitive.has("TANGENT");
                    primitiveData.skin = node.skin;
                    
                    DirectX::XMFLOAT4X4 worldMatrix = GetOwner()->transform->GetWorld();
                    
                    // TODO: コンポーネントの設計が変わったら変える
                    DirectX::XMStoreFloat4x4(&primitiveData.world, DirectX::XMLoadFloat4x4(&node.globalTransform) * DirectX::XMLoadFloat4x4(&worldMatrix));
                    immediateContext->UpdateSubresource(primitiveCbuffer.Get(), 0, 0, &primitiveData, 0, 0);
                    immediateContext->VSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
                    
                    if (primitive.indexBufferView.buffer > -1) {
                        // INTERLEAVED_GLTF_MODEL
                        immediateContext->IASetIndexBuffer(buffers.at(primitive.indexBufferView.buffer).Get(), primitive.indexBufferView.format, 0);
                        immediateContext->DrawIndexedInstanced(primitive.indexBufferView.sizeInBytes / _sizeof_component(primitive.indexBufferView.format), 4, 0, 0, 0);
                    }
                    else {
                        // INTERLEAVED_GLTF_MODEL
                        immediateContext->DrawInstanced(primitive.vertexBufferView.sizeInBytes / primitive.vertexBufferView.strideInBytes, 4, 0, 0);
                    }
                }
            }
            for (std::vector<int>::value_type childIndex : node.children) {
                traverse(childIndex);
            }
            };
	    if (!scenes.empty()) {
		    for (std::vector<int>::value_type nodeIndex : scenes.at(defaultScene).nodes) {
		    	traverse(nodeIndex);
		    }
	    }
    }

    //使ったシェーダーをリセット
    immediateContext->PSSetShader(NULL, NULL, 0);
    immediateContext->VSSetShader(NULL, NULL, 0);
    immediateContext->GSSetShader(NULL, NULL, 0);
}

void GltfModelRenderer::DrawProperty()
{
#ifdef USE_IMGUI
	// 静的バッチング用フラグチェックボックス
	ImGui::Checkbox("StaticBatching", &editorStaticBatchingFlag);
	// 読み込むモデルのパス
    ImGui::Text("FilePath: %s", filePath.c_str());
	// ファイル選択ボタン
    ImGui::SameLine();
    if (ImGui::Button("...")) {
		char filepath[260] = {};
		if (Dialog::OpenFileName(filepath, sizeof(filepath),
            "GLTF Model\0*.gltf;*.glb;*.cereal;*.batchCereal\0All Files\0*.*\0") == DialogResult::OK)
		{
			filePath = filepath;
            if (filePath.find(".batchCereal") != std::string::npos)
            {
                editorStaticBatchingFlag = true;
            }
			else if (filePath.find(".cereal") != std::string::npos)
            {
                editorStaticBatchingFlag = false;
			}
            if (!filePath.empty())
            {
                LoadModel(Graphics::GetDevice(), filePath, editorStaticBatchingFlag);
			}
		}
    }

    ImGui::Checkbox("AnimationEnable", &animationEnable);
    ImGui::Checkbox("BlendAnimationEnable", &blendEnable);
    if (blendEnable)
    {
        ImGui::SliderFloat("animationBlendTime", &animationBlendTime, 0.f, 20.f);
    }
    ImGui::Checkbox("isLoop", &loop);
    int i = 0;
    for (Animation& animation : animations) {
        ImGui::Text(std::to_string(i).c_str());
        ImGui::SameLine();
        if (ImGui::Button(animation.name.c_str())) {
            SetAnimation(animation.name, blendEnable);
        }
        i++;
    }

    // 基底クラスの呼び出し
    Renderer::DrawProperty();


#endif // USE_IMGUI
}

json GltfModelRenderer::Serialize() const
{
    json jsonData = Renderer::Serialize();
    jsonData["filePath"] = filePath;
    jsonData["staticBatching"] = staticBatching;
    return jsonData;
}

void GltfModelRenderer::Deserialize(const json& jsonData)
{
    Renderer::Deserialize(jsonData);
    filePath = jsonData.value("filePath", filePath);
    staticBatching = jsonData.value("staticBatching", staticBatching);
	// モデルの再読み込み
    LoadModel(Graphics::GetDevice(), filePath, staticBatching);
}