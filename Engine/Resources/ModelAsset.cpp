#include "pch.h"
#include "ModelAsset.h"
#include "Engine/Core/Misc.h"
#include "Texture.h"
#include <filesystem>
#include "Engine/Utils/GltfImporter.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

bool ModelAsset::LoadFromFile(const std::string& path)
{
	// ファイルからモデルデータを読み込む処理をここに実装します。
	_path = path;
	CurryEngine::Utils::GltfImporter importer;
    if (!importer.Import(path, *this)) {
        return false;
	}
    
    // リソースの作成とアップロード
	auto device = Graphics::GetDevice();
    CreateAndUploadResources(device);
	return true;
}

bool ModelAsset::Reload()
{
	// ホットリロードのための再読み込み処理をここに実装します。
	return LoadFromFile(_path);
}



void ModelAsset::CumulateTransforms(std::vector<Node>& nodes)
{
	// ノードの階層構造をトラバースして、ローカル変換を累積してグローバル変換を計算。
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

void ModelAsset::CreateAndUploadResources(ID3D11Device* device)
{
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
            const std::filesystem::path path(_path);
            ID3D11ShaderResourceView* shaderResourceView = NULL;
            std::wstring filePath = path.parent_path().concat(L"/").wstring() + std::wstring(image.uri.begin(), image.uri.end());
            hr = LoadTextureFromFile(device, filePath.c_str(), &shaderResourceView, NULL);
            if (hr == S_OK) {
                textureResourceViews.emplace_back().Attach(shaderResourceView);
            }
        }
    }
}