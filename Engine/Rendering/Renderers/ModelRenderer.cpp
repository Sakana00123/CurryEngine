#include "pch.h"
#include "ModelRenderer.h"

#include <Engine/Rendering/Pipeline/Graphics.h>
#include <Engine/Rendering/Pipeline/RenderContext.h>
#include <Engine/Core/EnginePaths.h>
#include <Engine/Resources/ResourceManager.h>
#include <Engine/Resources/Shader.h>

using namespace DirectX;

// ============================================================
// SetModelAsset
// ============================================================

void ModelRenderer::SetModelAsset(std::shared_ptr<AssetModel> asset)
{
    m_asset = asset;

    auto device = Graphics::GetDevice();

    // GPU バッファが未作成なら作る
    if (!m_asset->meshes.empty() && !m_asset->meshes[0].IsUploaded())
    {
        m_asset->UploadToGPU(device);
    }

    CreateShaders(device);
    CreateConstantBuffers(device);
    EnsureDefaultMaterial(device);
}

// ============================================================
// Update（将来 Animator に移管するまでの仮置き）
// ============================================================

void ModelRenderer::Update(float elapsedTime)
{
    if (!m_asset) return;
    if (m_asset->animations.empty()) return;
	if (animationIndex < 0 || animationIndex >= static_cast<int>(m_asset->animations.size())) return;

    // アニメーション時間を進める
    const AssetModel::Animation& anim = m_asset->animations[animationIndex];
    time += elapsedTime * timeRate;
    if (loop && time > anim.duration) time = std::fmod(time, anim.duration);

    // --- キーフレーム補間でノードの TRS を更新 ---
    auto lerpF3 = [](const XMFLOAT3& a, const XMFLOAT3& b, float t) -> XMFLOAT3 {
        return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
        };
    auto slerpQ = [](const XMFLOAT4& a, const XMFLOAT4& b, float t) -> XMFLOAT4 {
        XMFLOAT4 out;
        XMStoreFloat4(&out, XMQuaternionSlerp(XMLoadFloat4(&a), XMLoadFloat4(&b), t));
        return out;
        };

    // サンプラーから現在時刻の値を線形補間で取る汎用ラムダ
    auto findFrame = [&](const std::vector<float>& timeline, float t, int& i0, int& i1, float& blend)
        {
            if (timeline.size() == 1) { i0 = i1 = 0; blend = 0.0f; return; }
            // 二分探索でもよいが、クリップが短い前提でリニアサーチ
            for (int i = 0; i < static_cast<int>(timeline.size()) - 1; ++i)
            {
                if (t <= timeline[i + 1])
                {
                    i0 = i;
                    i1 = i + 1;
                    float span = timeline[i1] - timeline[i0];
                    blend = span > 0.0f ? (t - timeline[i0]) / span : 0.0f;
                    return;
                }
            }
            // 末端クランプ
            i0 = i1 = static_cast<int>(timeline.size()) - 1;
            blend = 0.0f;
        };

    for (const AssetModel::Animation::Channel& ch : anim.channels)
    {
        if (ch.nodeIndex < 0 || ch.samplerIndex < 0) continue;
        AssetModel::Node& node = m_asset->nodes[ch.nodeIndex];
        const AssetModel::Animation::Sampler& s = anim.samplers[ch.samplerIndex];

        int i0, i1; float blend;
        findFrame(s.timelines, time, i0, i1, blend);

        if (ch.targetPath == "translation" && !s.translations.empty())
            node.translation = lerpF3(s.translations[i0], s.translations[i1], blend);
        else if (ch.targetPath == "rotation" && !s.rotations.empty())
            node.rotation = slerpQ(s.rotations[i0], s.rotations[i1], blend);
        else if (ch.targetPath == "scale" && !s.scales.empty())
            node.scale = lerpF3(s.scales[i0], s.scales[i1], blend);
    }

    // TRS 更新後に globalTransform を再計算
    m_asset->CumulateTransforms();
}

// ============================================================
// Draw
// ============================================================

void ModelRenderer::Draw(RenderContext* rtx)
{
    if (!m_asset) return;
    if (!m_vsStatic || !m_vsSkinned) return;

    // TODO: ワールド行列は将来 Transform コンポーネントから受け取る
    XMFLOAT4X4 worldMatrix;
    XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());

    ID3D11DeviceContext* ctx = rtx->immediateContext;
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if (m_asset->staticBatching && !m_asset->batchedMeshes.empty())
    {
        DrawBatched(rtx, worldMatrix);
    }
    else
    {
        DrawNodes(rtx, worldMatrix);
    }
}

// ============================================================
// DrawNodes — ノードツリーを再帰走査
// ============================================================

void ModelRenderer::DrawNodes(RenderContext* rtx, const XMFLOAT4X4& worldMatrix)
{
    for (int rootIndex : m_asset->rootNodes)
    {
        DrawNode(rtx, rootIndex, worldMatrix);
    }
}

void ModelRenderer::DrawNode(RenderContext* rtx, int nodeIndex, const XMFLOAT4X4& worldMatrix)
{
    const AssetModel::Node& node = m_asset->nodes[nodeIndex];

    // 1ノードが複数メッシュを参照するケースに対応するため、meshIndices を全部回す
    for (size_t slot = 0; slot < node.meshIndices.size(); ++slot)
    {
        const int meshIndex = node.meshIndices[slot];
        if (meshIndex < 0) continue;

        // skinIndices は meshIndices と同じ並びで対応する（無ければ -1 扱い）
        const int skinIndex = (slot < node.skinIndices.size()) ? node.skinIndices[slot] : -1;

        const AssetModel::MeshData& mesh = m_asset->meshes[meshIndex];

        // --- InputLayout / VS の切り替え ---
        ID3D11DeviceContext* ctx = rtx->immediateContext;
        if (mesh.isSkinned)
        {
            ctx->VSSetShader(m_vsSkinned.Get(), nullptr, 0);
            ctx->IASetInputLayout(m_ilSkinned.Get());
        }
        else
        {
            ctx->VSSetShader(m_vsStatic.Get(), nullptr, 0);
            ctx->IASetInputLayout(m_ilStatic.Get());
        }

        // --- ジョイント行列を更新（スキニングメッシュのみ） ---
        if (mesh.isSkinned && skinIndex >= 0)
        {
            UpdateJointCB(rtx, skinIndex, nodeIndex);
        }

        // --- プリミティブ定数バッファを更新 ---
        PrimitiveConstants primData{};
        primData.materialIndex = mesh.materialIndex;
        primData.hasTangent = mesh.hasTangent ? 1 : 0;
        primData.skinIndex = skinIndex;
        XMStoreFloat4x4(
            &primData.world,
            XMLoadFloat4x4(&node.globalTransform) * XMLoadFloat4x4(&worldMatrix));

        ctx->UpdateSubresource(m_primitiveCB.Get(), 0, nullptr, &primData, 0, 0);
		static const UINT cbSlot = 5; // VS/PS 定数バッファスロット 5 を使用
        ctx->VSSetConstantBuffers(cbSlot, 1, m_primitiveCB.GetAddressOf());
        ctx->PSSetConstantBuffers(cbSlot, 1, m_primitiveCB.GetAddressOf());

        // --- マテリアル適用 ---
        if (mesh.materialIndex >= 0 &&
            mesh.materialIndex < static_cast<int>(m_asset->materials.size()))
        {
            m_asset->materials[mesh.materialIndex]->Apply(rtx);
        }
        else if (m_material)
        {
            m_material->Apply(rtx);
        }

        DrawMesh(rtx, mesh, primData);
    }

    for (int childIndex : node.children)
    {
        DrawNode(rtx, childIndex, worldMatrix);
    }
}

// ============================================================
// DrawBatched — staticBatching 用
// ============================================================

void ModelRenderer::DrawBatched(RenderContext* rtx, const XMFLOAT4X4& worldMatrix)
{
    ID3D11DeviceContext* ctx = rtx->immediateContext;

    // バッチメッシュはすべて StaticVertex
    ctx->VSSetShader(m_vsStatic.Get(), nullptr, 0);
    ctx->IASetInputLayout(m_ilStatic.Get());

    for (const AssetModel::BatchedMesh& batch : m_asset->batchedMeshes)
    {
        if (!batch.vertexBuffer) continue;

        UINT stride = batch.vertexStride;
        UINT offset = 0;
        ctx->IASetVertexBuffers(0, 1, batch.vertexBuffer.GetAddressOf(), &stride, &offset);

        PrimitiveConstants primData{};
        primData.materialIndex = batch.materialIndex;
        primData.hasTangent = 1; // バッチ構築時にタンジェントを保証する前提
        primData.skinIndex = -1;
        primData.world = worldMatrix;

        ctx->UpdateSubresource(m_primitiveCB.Get(), 0, nullptr, &primData, 0, 0);
		static const UINT cbSlot = 5; // VS/PS 定数バッファスロット 5 を使用
        ctx->VSSetConstantBuffers(cbSlot, 1, m_primitiveCB.GetAddressOf());
        ctx->PSSetConstantBuffers(cbSlot, 1, m_primitiveCB.GetAddressOf());

        if (batch.materialIndex >= 0 &&
            batch.materialIndex < static_cast<int>(m_asset->materials.size()))
        {
            m_asset->materials[batch.materialIndex]->Apply(rtx);
        }
        else if (m_material)
        {
            m_material->Apply(rtx);
        }

        if (batch.indexBuffer)
        {
            ctx->IASetIndexBuffer(batch.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            ctx->DrawIndexed(batch.indexCount, 0, 0);
        }
        else
        {
            const UINT vertexCount =
                batch.vertexStride > 0
                ? (batch.vertexStride == sizeof(AssetModel::StaticVertex)
                    ? static_cast<UINT>(m_asset->batchedMeshes.size()) // fallback
                    : 0)
                : 0;
            ctx->Draw(vertexCount, 0);
        }
    }
}

// ============================================================
// DrawMesh — 頂点バッファ・インデックスバッファを IA にセットして描画
// ============================================================

void ModelRenderer::DrawMesh(
    RenderContext* rtx,
    const AssetModel::MeshData& mesh,
    const PrimitiveConstants& /*primData*/)
{
    if (!mesh.vertexBuffer) return;

    ID3D11DeviceContext* ctx = rtx->immediateContext;

    UINT stride = mesh.vertexStride;
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);

    if (mesh.indexBuffer)
    {
        // uint32_t インデックス固定（AssetModel::UploadMesh で DXGI_FORMAT_R32_UINT として作成）
        ctx->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        ctx->DrawIndexed(mesh.indexCount, 0, 0);
    }
    else
    {
        UINT vertexCount = mesh.isSkinned
            ? static_cast<UINT>(mesh.skinnedVertices.size())
            : static_cast<UINT>(mesh.staticVertices.size());
        ctx->Draw(vertexCount, 0);
    }
}

// ============================================================
// UpdateJointCB — スキニング用ジョイント行列を定数バッファに書き込む
// ============================================================

void ModelRenderer::UpdateJointCB(RenderContext* rtx, int skinIndex, int nodeIndex)
{
    const AssetModel::Skin& skin = m_asset->skins[skinIndex];

    _ASSERT_EXPR(
        skin.joints.size() <= MAX_JOINTS,
        L"ModelRenderer: joint count exceeds MAX_JOINTS");

    PrimitiveJointConstants jointData{};

    const XMMATRIX invNodeGlobal = XMMatrixInverse(
        nullptr,
        XMLoadFloat4x4(&m_asset->nodes[nodeIndex].globalTransform));

    for (size_t ji = 0; ji < skin.joints.size(); ++ji)
    {
        const int jointNodeIndex = skin.joints[ji];
        if (jointNodeIndex < 0) continue; // 未解決ボーンはスキップ

        // ジョイント行列 = InverseBindMatrix * JointGlobalTransform * InvNodeGlobal
        XMStoreFloat4x4(
            &jointData.matrices[ji],
            XMLoadFloat4x4(&skin.inverseBindMatrices[ji]) *
            XMLoadFloat4x4(&m_asset->nodes[jointNodeIndex].globalTransform) *
            invNodeGlobal);
    }

    ID3D11DeviceContext* ctx = rtx->immediateContext;
    ctx->UpdateSubresource(m_jointCB.Get(), 0, nullptr, &jointData, 0, 0);
    ctx->VSSetConstantBuffers(6, 1, m_jointCB.GetAddressOf());
}

// ============================================================
// 初期化ヘルパー
// ============================================================

void ModelRenderer::CreateShaders(ID3D11Device* device)
{
    const std::string dir = EnginePaths::ShadersDataDir;

    // --- StaticVertex 用 InputLayout ---
    D3D11_INPUT_ELEMENT_DESC ilStatic[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // --- SkinnedVertex 用 InputLayout ---
    D3D11_INPUT_ELEMENT_DESC ilSkinned[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "JOINTS",   0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "WEIGHTS",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    CreateVertexShaderFromCSO(
        device,
        (dir + "model_static_vs.cso").c_str(),
        m_vsStatic.ReleaseAndGetAddressOf(),
        m_ilStatic.ReleaseAndGetAddressOf(),
        ilStatic, _countof(ilStatic));

    CreateVertexShaderFromCSO(
        device,
        (dir + "model_skinned_vs.cso").c_str(),
        m_vsSkinned.ReleaseAndGetAddressOf(),
        m_ilSkinned.ReleaseAndGetAddressOf(),
        ilSkinned, _countof(ilSkinned));

    // CSM（カスケードシャドウマップ）
    CreateVertexShaderFromCSO(
        device,
        (dir + "model_csm_vs.cso").c_str(),
        m_vsCsm.ReleaseAndGetAddressOf(),
        nullptr, nullptr, 0);

    CreateGeometryShaderFromCSO(
        device,
        (dir + "model_csm_gs.cso").c_str(),
        m_gsCsm.ReleaseAndGetAddressOf());
}

void ModelRenderer::CreateConstantBuffers(ID3D11Device* device)
{
    HRESULT hr{};
    D3D11_BUFFER_DESC desc{};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    desc.ByteWidth = sizeof(PrimitiveConstants);
    hr = device->CreateBuffer(&desc, nullptr, m_primitiveCB.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

    desc.ByteWidth = sizeof(PrimitiveJointConstants);
    hr = device->CreateBuffer(&desc, nullptr, m_jointCB.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void ModelRenderer::EnsureDefaultMaterial(ID3D11Device* device)
{
    if (m_material) return; // 外部から設定済みなら触らない

    m_material = std::make_shared<Material>();

    auto setupMaterial = [&](std::shared_ptr<Material> material)
        {
            auto ps = ResourceManager::GetShader<PixelShader>("model_ps");
            if (ps) material->SetShader(device, ps);

            // モデルレンダラーが自前で管理しない定数バッファはバインドを抑制
            material->SetNotBindCBuffer({
                "SCENE_CONSTANT_BUFFER",
                "LIGHT_CONSTANT_BUFFER",
                "PRIMITIVE_CONSTANT_BUFFER",
                });
        };
	// デフォルトマテリアルを設定
	setupMaterial(m_material);

	// モデルアセットのマテリアルも設定しておく
    for (auto& mat : m_asset->materials)
    {
        setupMaterial(mat);
	}

}