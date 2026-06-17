#include "pch.h"
#include "GltfImporter.h"
#include <Engine\Core\Misc.h>
#include <Engine\Rendering\Pipeline\Graphics.h>

#include <filesystem>
#include <fstream>

// TODO: MeshColliderのための正しいデータを作るために追加した処理のせいで、スキニングするときに変なことになってたので、直すこと。
//#define TEST_CONVERT

namespace CurryEngine
{
	namespace Utils
	{
		using Scene = ModelAsset::Scene;
		using Node = ModelAsset::Node;
		using Mesh = ModelAsset::Mesh;
		using BatchMesh = ModelAsset::BatchMesh;
		using Texture = ModelAsset::Texture;
		using Material = ModelAsset::Material;
		using Image = ModelAsset::Image;
		using Skin = ModelAsset::Skin;
		using Animation = ModelAsset::Animation;

        bool _NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*) {
            return true;
        }

		bool GltfImporter::Import(const std::string& path, ModelAsset& asset)
		{
            bool staticBatching = asset.staticBatching;
			std::string filePath = path;
            auto device = Graphics::GetDevice();
			auto& scenes = asset.scenes;
			auto& defaultScene = asset.defaultScene;
			auto& nodes = asset.nodes;
			auto& materials = asset.materials;
			auto& batchMeshes = asset.batchMeshes;
			auto& meshes = asset.meshes;
			auto& textures = asset.textures;
			auto& images = asset.images;
			auto& skins = asset.skins;
			auto& animations = asset.animations;
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
            else
            {
                tinygltf::TinyGLTF tinyGltf;
                tinyGltf.SetImageLoader(_NullLoadImageData, nullptr);

                std::shared_ptr<tinygltf::Model> gltfModel;
                gltfModel = std::make_shared<tinygltf::Model>();

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


				FetchScenes(*gltfModel, asset);
                FetchNodes(*gltfModel, asset);
                FetchMaterials(device, *gltfModel, asset);
                FetchTextures(device, *gltfModel, asset);

#ifdef SUPPORT_BATCHING
                if (staticBatching) {
                    FetchBatchMeshes(device, *gltfModel, asset);
                }
                else
#endif // SUPPORT_BATCHING
                {
                    FetchMeshes(device, *gltfModel, asset);
					FetchSkins(*gltfModel, asset);
                    FetchAnimations(*gltfModel, asset);
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

			return true;
		}


		std::vector<std::string> GltfImporter::GetSupportedExtensions() const
		{
			return { ".gltf", ".glb", ".cereal"/*, ".batchcereal"*/ };
		}

        void GltfImporter::FetchScenes(const tinygltf::Model& gltfModel, ModelAsset& asset)
        {
            auto& scenes = asset.scenes;
            for (const tinygltf::Scene& gltfScene : gltfModel.scenes) {
                Scene& scene = scenes.emplace_back();
                scene.name = gltfScene.name;
                scene.nodes = gltfScene.nodes;
            }
            asset.defaultScene = gltfModel.defaultScene < 0 ? 0 : gltfModel.defaultScene;
		}

		void GltfImporter::FetchNodes(const tinygltf::Model& gltfModel, ModelAsset& asset)
		{
			// GLTF ノードの情報を ModelAsset のノード構造に変換して格納する処理をここに実装します。
            auto& nodes = asset.nodes;
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

#ifdef TEST_CONVERT
                // TODO: たまたま上手くいってる可能性があるので、今後問題が出てきたらここを見直すこと
                node.translation.z = -node.translation.z; // glTFの右手系を左手系に変換
                node.rotation.x = -node.rotation.x; // glTFの右手系を左手系に変換
                node.rotation.y = -node.rotation.y; // glTFの右手系を左手系に変換  
#endif // TEST_CONVERT
            }

			// ノードの階層構造を考慮して、グローバル変換行列を計算する。
			asset.CumulateTransforms(nodes);
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

        void GltfImporter::FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel, ModelAsset& asset)
        {
            // GLTF メッシュの情報を ModelAsset のメッシュ構造に変換して格納する処理をここに実装します。
			auto& meshes = asset.meshes;
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

#ifdef TEST_CONVERT
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
#endif // TEST_CONVERT

                    primitive.vertexBufferView.sizeInBytes = static_cast<UINT>(primitive.cachedVertices.size() * sizeof(Mesh::Vertex));

                }

            }
        }

#ifdef SUPPORT_BATCHING
        void GltfImporter::FetchBatchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel, ModelAsset& asset)
        {
            // GLTF バッチメッシュの情報を ModelAsset のバッチメッシュ構造に変換して格納する処理をここに実装します。
            auto& batchMeshes = asset.batchMeshes;
            auto& nodes = asset.nodes;
            const auto& scenes = asset.scenes;
            auto defaultScene = asset.defaultScene;

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

#ifdef TEST_CONVERT
                            // TODO: たまたま上手くいってる可能性があるので、今後問題が出てきたらここを見直すこと
                                        // 左手系(Y-Up, Z-Forward)に変換
                            cachedVertex.position.z = -cachedVertex.position.z;
                            cachedVertex.normal.z = -cachedVertex.normal.z;   // normal.y ではなく z を反転
                            cachedVertex.tangent.z = -cachedVertex.tangent.z; // タンジェントも反転  
#endif // TEST_CONVERT
                        }

#ifdef TEST_CONVERT
                        // 【追加】インデックスの巻き順を逆にする
                        std::vector<UINT>& cachedIndices = batchMesh.cachedIndices;
                        for (size_t i = 0; i < cachedIndices.size(); i += 3) {
                            std::swap(cachedIndices[i], cachedIndices[i + 2]);
                        }
#endif // TEST_CONVERT

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
#endif // SUPPORT_BATCHING

        void GltfImporter::FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel, ModelAsset& asset)
        {
            // GLTF マテリアルの情報を ModelAsset のマテリアル構造に変換して格納する処理をここに実装します。
			auto& materials = asset.materials;
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

        void GltfImporter::FetchTextures(ID3D11Device* device, const tinygltf::Model& gltfModel, ModelAsset& asset)
        {
            // GLTF テクスチャの情報を ModelAsset のテクスチャ構造に変換して格納する処理をここに実装します。
			auto& textures = asset.textures;
			auto& images = asset.images;
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

        void GltfImporter::FetchSkins(const tinygltf::Model& gltfModel, ModelAsset& asset)
        {
			// GLTF スキンの情報を ModelAsset のスキン構造に変換して格納する処理をここに実装します。
			auto& skins = asset.skins;
            for (const tinygltf::Skin& transmissionSkin : gltfModel.skins) {
                Skin& skin = skins.emplace_back();
                const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(transmissionSkin.inverseBindMatrices);
                const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);
                _ASSERT_EXPR(gltfAccessor.type == TINYGLTF_TYPE_MAT4, L"");

                skin.inverseBindMatrices.resize(gltfAccessor.count);
                memcpy(skin.inverseBindMatrices.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT4X4));

                skin.joints = transmissionSkin.joints;
            }
        }

        void GltfImporter::FetchAnimations(const tinygltf::Model& gltfModel, ModelAsset& asset)
        {
            // GLTF アニメーションの情報を ModelAsset のアニメーション構造に変換して格納する処理をここに実装します。
			auto& animations = asset.animations;
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
            for (decltype(asset.animations)::reference animation : animations)
            {
                for (decltype(animation.timelines)::reference timelines : animation.timelines)
                {
                    animation.duration = std::max<float>(animation.duration, timelines.second.back());
                }
            }
        }

	}
}