#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <memory>
#include <Engine/Resources/AssetModel.h>
#include <Engine/Rendering/Material.h>

struct RenderContext;

/**
 * @file ModelRenderer.h
 * @brief AssetModel を受け取り、ノードツリーを走査して描画するレンダラー。
 *
 * 旧 ModelRenderer から以下を変更：
 * - ModelAsset → AssetModel
 * - 独自バッファプール参照 → MeshData が直接持つ vertexBuffer / indexBuffer を使用
 * - materialResourceView / textureResourceViews の独自管理 → Material::Apply() に委譲
 * - staticBatching は AssetModel::batchedMeshes が存在するかで判定
 *
 * 将来的には MeshRenderer（静的）と Animator（スケルタル）に分離予定。
 * 現状はプレビュー用として両方をこのクラスで処理する。
 */
class ModelRenderer
{
public:
    ModelRenderer() = default;
    ~ModelRenderer() = default;

    /**
     * @brief アセットをセットし、シェーダ・定数バッファを初期化する。
     * AssetModel::UploadToGPU() が済んでいない場合はここで呼び出す。
     */
    void SetModelAsset(std::shared_ptr<AssetModel> asset);

    void Update(float elapsedTime);
    void Draw(RenderContext* rtx, const XMMATRIX& world = XMMatrixIdentity());

    // --- アニメーション制御（将来 Animator コンポーネントに移管予定） ---
    float time = 0.0f;  //!< アニメーション経過時間（秒）
    float timeRate = 1.0f;  //!< 再生速度倍率
    float animationBlendTime = 1.2f;//!< ブレンド時間（秒）
    int   animationIndex = 0;     //!< 再生するアニメーションのインデックス
    bool  loop = true;  //!< ループ再生

    std::shared_ptr<AssetModel> m_asset;   //!< モデルアセット
    std::shared_ptr<Material>   m_material;//!< 描画に使うマテリアル（外部から上書き可）

private:
    // --- D3D11 オブジェクト ---

    // 通常描画用
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsStatic;   //!< 静的メッシュ用 VS
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsSkinned;  //!< スキニング用 VS
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_ilStatic;   //!< StaticVertex 用 IL
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_ilSkinned;  //!< SkinnedVertex 用 IL

    // カスケードシャドウマップ用
    Microsoft::WRL::ComPtr<ID3D11VertexShader>   m_vsCsm;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_gsCsm;

    // --- 定数バッファ ---

    /**
     * @brief プリミティブごとの定数バッファ（VS/PS スロット 0）。
     * ワールド行列・マテリアルインデックス・スキンインデックスを渡す。
     */
    struct PrimitiveConstants
    {
        DirectX::XMFLOAT4X4 world;
        int  materialIndex = -1;
        int  hasTangent = 0;
        int  skinIndex = -1;
        int  _pad = -1;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_primitiveCB;

    /**
     * @brief スキニング用ジョイント行列の定数バッファ（VS スロット 6）。
     */
    static constexpr size_t MAX_JOINTS = 512;
    struct PrimitiveJointConstants
    {
        DirectX::XMFLOAT4X4 matrices[MAX_JOINTS];
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_jointCB;

    // --- 初期化 ---
    void CreateShaders(ID3D11Device* device);
    void CreateConstantBuffers(ID3D11Device* device);
    void EnsureDefaultMaterial(ID3D11Device* device);

    // --- 描画サブルーティン ---

    /** @brief ノードツリーを再帰走査してスキニングメッシュ／静的メッシュを描画する。*/
    void DrawNodes(RenderContext* rtx, const DirectX::XMFLOAT4X4& worldMatrix);

    /** @brief 単一ノードを描画する（DrawNodes の内部再帰関数）。*/
    void DrawNode(
        RenderContext* rtx,
        int nodeIndex,
        const DirectX::XMFLOAT4X4& worldMatrix);

    /** @brief staticBatching が有効なとき batchedMeshes を描画する。*/
    void DrawBatched(RenderContext* rtx, const DirectX::XMFLOAT4X4& worldMatrix);

    /**
     * @brief メッシュ 1 件分の頂点バッファ・インデックスバッファをセットして描画する。
     * @param mesh       描画するメッシュ。
     * @param primData   更新済みの PrimitiveConstants。
     */
    void DrawMesh(
        RenderContext* rtx,
        const AssetModel::MeshData& mesh,
        const PrimitiveConstants& primData);

    /**
     * @brief スキンのジョイント行列を計算して m_jointCB を更新する。
     * @param skinIndex  AssetModel::skins へのインデックス。
     * @param nodeIndex  このスキンを参照しているノードのインデックス（逆行列の基準）。
     */
    void UpdateJointCB(RenderContext* rtx, int skinIndex, int nodeIndex);
};
