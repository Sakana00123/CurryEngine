#include "pch.h"
#include "GltfModelRenderer.h"
#include "Engine/Core/GameObject.h"

#include "Engine/Core/Misc.h"
#include "Engine/Editor/Dialog.h"
#include "Engine/Resources/Texture.h"
#include "Engine/Resources/Shader.h"
#include "Engine/Resources/ResourceManager.h"

#include "Engine/Editor/HlslEditor.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

REGISTER_COMPONENT(GltfModelRenderer, "Rendering")

using Node = ModelAsset::Node;
using Mesh = ModelAsset::Mesh;
using BatchMesh = ModelAsset::BatchMesh;
//using Material = ModelAsset::Material;
//using Texture = ModelAsset::Texture;
using Skin = ModelAsset::Skin;
using Animation = ModelAsset::Animation;

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
	auto& nodes = m_asset->nodes;
	auto& scenes = m_asset->scenes;
	auto& defaultScene = m_asset->defaultScene;
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

GltfModelRenderer::GltfModelRenderer()
{
	
}

void GltfModelRenderer::LoadModel(ID3D11Device* device, const std::string& filePath, bool staticBatching)
{
	this->filePath = filePath;

    if (m_asset)
    {
		m_asset.reset();
    }
	m_asset = std::make_shared<ModelAsset>();

	m_asset->staticBatching = staticBatching;

    if (!m_asset->LoadFromFile(filePath)) {
		LOG_ERROR("Failed to load model asset from file: " + filePath);
        return;
	}
	
	// リソースの作成とアップロード
    CreateAndUploadResources(device);

    // AABBの計算
    boundingBox = CalculateAABB();
}

void GltfModelRenderer::SetModelAsset(std::shared_ptr<ModelAsset> asset)
{
    m_asset = asset;
	auto device = Graphics::GetDevice();
    // リソースの作成とアップロード
    CreateAndUploadResources(device);
    // AABBの計算
    boundingBox = CalculateAABB();
}

void GltfModelRenderer::Initialize()
{
    
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

void GltfModelRenderer::Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes) {
	auto& nodes = m_asset->nodes;
	auto& animations = m_asset->animations;
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
        m_asset->CumulateTransforms(animatedNodes);
    }
}

void GltfModelRenderer::CreateAndUploadResources(ID3D11Device* device) {
    std::string dir = EnginePaths::ShadersDataDir;
	bool staticBatching = m_asset->staticBatching;
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

	HRESULT hr{};
    D3D11_BUFFER_DESC bufferDesc{};
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
#if 0
	auto& nodes = m_asset->nodes;
	auto& animations = m_asset->animations;

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
	
	auto& nodes = m_asset->nodes;
	auto& materials = m_asset->materials;
	auto& textures = m_asset->textures;
	auto& images = m_asset->images;
	auto& meshes = m_asset->meshes;
	auto& batchMeshes = m_asset->batchMeshes;
	auto& staticBatching = m_asset->staticBatching;
	auto& materialResourceView = m_asset->materialResourceView;
	auto& buffers = m_asset->buffers;
	auto& skins = m_asset->skins;
	auto& textureResourceViews = m_asset->textureResourceViews;
	auto& scenes = m_asset->scenes;
	auto& defaultScene = m_asset->defaultScene;

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

            const ModelAsset::Material& material = materials.at(batchMesh.material);
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

                    const ModelAsset::Material& material = materials.at(primitive.material);
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
    auto& nodes = m_asset->nodes;
    auto& materials = m_asset->materials;
    auto& textures = m_asset->textures;
    auto& images = m_asset->images;
    auto& meshes = m_asset->meshes;
    auto& batchMeshes = m_asset->batchMeshes;
    auto& staticBatching = m_asset->staticBatching;
    auto& materialResourceView = m_asset->materialResourceView;
    auto& buffers = m_asset->buffers;
    auto& skins = m_asset->skins;
    auto& textureResourceViews = m_asset->textureResourceViews;
    auto& scenes = m_asset->scenes;
    auto& defaultScene = m_asset->defaultScene;

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

#ifdef USE_IMGUI
void GltfModelRenderer::DrawProperty(const PropertyDrawContext& context)
{
	auto& animations = m_asset->animations;

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
    Renderer::DrawProperty(context);


}
#endif // USE_IMGUI

json GltfModelRenderer::Serialize() const
{
    json jsonData = Renderer::Serialize();
    jsonData["filePath"] = filePath;
    jsonData["staticBatching"] = m_asset->staticBatching;
    return jsonData;
}

void GltfModelRenderer::Deserialize(const json& jsonData)
{
    Renderer::Deserialize(jsonData);
    filePath = jsonData.value("filePath", filePath);
    bool staticBatching = jsonData.value("staticBatching", false);
	// モデルの再読み込み
    LoadModel(Graphics::GetDevice(), filePath, staticBatching);
}


void GltfModelRenderer::ApplyPose(const std::vector<NodePose>& poses)
{
	_ASSERT_EXPR(poses.size() == m_asset->nodes.size(), L"Pose size does not match node size.");

    for (size_t i = 0; i < poses.size(); ++i) {
        const NodePose& pose = poses.at(i);
        Node& node = m_asset->nodes.at(i);
        node.translation = pose.translation;
        node.rotation = pose.rotation;
        node.scale = pose.scale;
	}
	m_asset->CumulateTransforms(m_asset->nodes);
}

size_t GltfModelRenderer::GetNodeCount() const
{
	return m_asset->nodes.size();
}
