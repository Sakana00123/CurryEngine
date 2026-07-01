#pragma once
#include "Resource.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <d3d11.h>
#include <filesystem>
#include <wrl/client.h>
#include <DirectXMath.h>
#include "AssetMeta.h"

// 前方宣言
class Material;
class Texture;
struct aiScene;
namespace fs = std::filesystem;

/**
 * @file AssetModel.h
 * @brief フォーマット非依存のモデルアセット。
 *
 * assimp 経由でロードされた 3D モデルデータを保持する。
 * エンジン共通の Material / Texture クラスを直接参照する。
 *
 * GPU リソース（頂点バッファ・インデックスバッファ）は各 MeshData が直接保持する。
 * シリアライズは将来 AssetDatabase 経由の json 化に統一予定。
 */
class AssetModel : public Resource
{
public:
    AssetModel() = default;
    virtual ~AssetModel() = default;

    // -----------------------------------------------------------------------
    // Resource インターフェース
    // -----------------------------------------------------------------------

    /**
     * @brief ファイルパスからロードする（Resource 契約上の実装）。
     * @note 将来は AssetMeta を受け取る形に変更予定。
     *       内部で assimp を呼び出し ImportFromScene() に委譲する。
     */
    bool LoadFromFile(const std::string& path) override;

    /** @brief ホットリロード。*/
    bool Reload() override;

	bool LoadFromMeta(const CurryEngine::Resources::AssetMeta& meta);

    // -----------------------------------------------------------------------
    // 頂点レイアウト
    // -----------------------------------------------------------------------

    /**
     * @brief スキニングなしメッシュの頂点。
     * InputLayout: POSITION / NORMAL / TANGENT / TEXCOORD
     */
    struct StaticVertex
    {
        DirectX::XMFLOAT3 position = { 0, 0, 0 };
        DirectX::XMFLOAT3 normal = { 0, 0, 1 };
        DirectX::XMFLOAT4 tangent = { 1, 0, 0, 1 };
        DirectX::XMFLOAT2 texcoord = { 0, 0 };
    };

    /**
     * @brief スキニングあり（スケルタル）メッシュの頂点。
     * InputLayout: POSITION / NORMAL / TANGENT / TEXCOORD / JOINTS / WEIGHTS
     */
    struct SkinnedVertex
    {
        DirectX::XMFLOAT3 position = { 0, 0, 0 };
        DirectX::XMFLOAT3 normal = { 0, 0, 1 };
        DirectX::XMFLOAT4 tangent = { 1, 0, 0, 1 };
        DirectX::XMFLOAT2 texcoord = { 0, 0 };
        DirectX::XMUINT4  joints = { 0, 0, 0, 0 };
        DirectX::XMFLOAT4 weights = { 1, 0, 0, 0 };
    };

    // -----------------------------------------------------------------------
    // メッシュ
    // -----------------------------------------------------------------------

    /**
     * @brief 1 つのドローコールに対応するメッシュデータ。
     *
     * スキニングの有無で頂点バッファの中身が変わるが、
     * GPU バッファは共通のポインタで保持する。
     * どちらの頂点型を使うかは isSkinned で判定する。
     */
    struct MeshData
    {
        std::string name;

        // --- CPU 側キャッシュ ---
        std::vector<StaticVertex>  staticVertices;  //!< isSkinned == false のとき有効
        std::vector<SkinnedVertex> skinnedVertices; //!< isSkinned == true  のとき有効
        std::vector<uint32_t>      indices;

        // --- GPU バッファ ---
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
        UINT vertexStride = 0; //!< sizeof(StaticVertex) or sizeof(SkinnedVertex)
        UINT indexCount = 0;

        // --- マテリアル参照 ---
        int materialIndex = -1; //!< AssetModel::materials へのインデックス

        // --- フラグ ---
        bool isSkinned = false; //!< スケルタルアニメーション対象かどうか
        bool hasTangent = false; //!< タンジェント属性が存在するか

        // --- スキン構築用（ImportSkins() が参照後に不要になる一時データ） ---
        std::vector<std::string>               boneNames;          //!< ボーン名（ノード名と対応）
        std::vector<DirectX::XMFLOAT4X4>      boneOffsetMatrices; //!< インバースバインドポーズ行列

        /** @brief GPU バッファが作成済みかどうか。*/
        bool IsUploaded() const { return vertexBuffer != nullptr; }
    };

    // -----------------------------------------------------------------------
    // ノード
    // -----------------------------------------------------------------------

    /**
     * @brief シーングラフのノード。
     * ノードは Mesh を 0 または 1 つ参照し、子ノードを持てる。
     * Skin / Animation から参照されるジョイントもノードとして表現される。
     */
    struct Node
    {
        std::string name;

        // --- 階層 ---
        int              parent = -1; //!< 親ノードのインデックス（ルートは -1）
        std::vector<int> children;      //!< 子ノードのインデックス配列

        // --- ローカルトランスフォーム (TRS) ---
        DirectX::XMFLOAT4 rotation = { 0, 0, 0, 1 };
        DirectX::XMFLOAT3 scale = { 1, 1, 1 };
        DirectX::XMFLOAT3 translation = { 0, 0, 0 };

        // --- ワールドトランスフォーム（累積済み） ---
        DirectX::XMFLOAT4X4 globalTransform = {
            1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1
        };

        // --- 参照 ---
        // 1ノードが複数メッシュを持つケース（aiNode::mNumMeshes > 1）に対応するため vector で保持する。
        // meshIndices[i] に対応するスキンは skinIndices[i]（スキン無しは -1）。
        std::vector<int> meshIndices; //!< AssetModel::meshes へのインデックス配列
        std::vector<int> skinIndices; //!< AssetModel::skins  へのインデックス配列（meshIndices と対応、要素数は同じ）
    };

    // -----------------------------------------------------------------------
    // スキン
    // -----------------------------------------------------------------------

    /**
     * @brief スケルタルアニメーション用のスキンデータ。
     * joints は AssetModel::nodes へのインデックス配列。
     */
    struct Skin
    {
        std::string              name;
        std::vector<int>         joints;               //!< ジョイントノードのインデックス
        std::vector<DirectX::XMFLOAT4X4> inverseBindMatrices; //!< ジョイントごとの逆バインド行列
    };

    // -----------------------------------------------------------------------
    // アニメーション
    // -----------------------------------------------------------------------

    /**
     * @brief 1 つのアニメーションクリップ。
     */
    struct Animation
    {
        std::string name;
        float       duration = 0.0f; //!< クリップの長さ（秒）

        /** @brief アニメーションが影響を与えるノードとチャンネルの対応。*/
        struct Channel
        {
            int         nodeIndex = -1; //!< 対象ノードのインデックス
            std::string targetPath;      //!< "translation" / "rotation" / "scale"
            int         samplerIndex = -1;
        };

        /**
         * @brief キーフレームのサンプラー。
         * timelines と values は同じ長さのペアで使う。
         */
        struct Sampler
        {
            std::string              interpolation; //!< "LINEAR" / "STEP" / "CUBICSPLINE"
            std::vector<float>       timelines;     //!< 時刻列（秒）

            // チャンネルの targetPath に応じて どれか 1 つが有効
            std::vector<DirectX::XMFLOAT3> translations;
            std::vector<DirectX::XMFLOAT4> rotations;
            std::vector<DirectX::XMFLOAT3> scales;
        };

        std::vector<Channel> channels;
        std::vector<Sampler> samplers;
    };

    // -----------------------------------------------------------------------
    // アセット全体のデータ
    // -----------------------------------------------------------------------

    /** @brief シーングラフのルートノードインデックス一覧。*/
    std::vector<int>       rootNodes;

    /** @brief 全ノード一覧（インデックスで参照）。*/
    std::vector<Node>      nodes;

    /**
     * @brief 全メッシュ一覧。
     * Node::meshIndices から参照される。1 メッシュ = 1 ドローコール。
     * （旧 Primitive 単位でフラット化して持つ設計）
     */
    std::vector<MeshData>  meshes;

    /**
     * @brief 全マテリアル一覧。
     * エンジン共通の Material クラスを直接保持する。
     * MeshData::materialIndex から参照される。
     */
    std::vector<std::shared_ptr<Material>> materials;

    /**
     * @brief 全テクスチャ一覧。
     * エンジン共通の Texture クラスを直接保持する。
     * Material::SetTexture() で各マテリアルに渡し済みのため、
     * ここでは所有権の管理（ライフタイム延長）のみを目的とする。
     */
    std::vector<std::shared_ptr<Texture>> textures;

    /** @brief 全スキン一覧。Node::skinIndices から参照される。*/
    std::vector<Skin>      skins;

    /** @brief 全アニメーション一覧。*/
    std::vector<Animation> animations;

    // -----------------------------------------------------------------------
    // Static Batching（将来対応）
    // -----------------------------------------------------------------------

    /**
     * @brief スタティックバッチングが有効かどうか。
     * true のとき batchedMesh にバッチ結合済みデータが入る。
     * バッチ構築は ModelBatcher など別クラスに委譲予定。
     * @todo ModelBatcher を実装したら構築処理をそちらへ移す。
     */
    bool staticBatching = false;

    /**
     * @brief バッチ結合済みメッシュ（staticBatching == true のとき使用）。
     * スキニングなし・マテリアルごとにマージされた単一メッシュ。
     */
    struct BatchedMesh
    {
        std::vector<StaticVertex> vertices;
        std::vector<uint32_t>     indices;

        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
        UINT vertexStride = 0;
        UINT indexCount = 0;

        int materialIndex = -1;
    };
    std::vector<BatchedMesh> batchedMeshes;

    // -----------------------------------------------------------------------
    // GPU アップロード
    // -----------------------------------------------------------------------

    /**
     * @brief 全メッシュの GPU バッファを作成・アップロードする。
     * SetModelAsset() から呼び出される想定。
     * @param device D3D11 デバイス。
     */
    void UploadToGPU(ID3D11Device* device);

    /**
     * @brief ノードツリーを走査してglobalTransform を累積計算する。
     * ロード後・アニメーション適用後に呼ぶ。
     */
    void CumulateTransforms();

private:
    /**
     * @brief assimp の aiScene から内部データを構築する。
     * LoadFromFile() から呼び出される。
     * @param scene assimp がパースしたシーン。
     * @param baseDir テクスチャの相対パス解決用ディレクトリ。
     */
    bool ImportFromScene(const aiScene* scene, const std::string& baseDir);

    /** @brief 単一ノードの globalTransform を再帰的に累積する。*/
    void CumulateTransforms(int nodeIndex, const DirectX::XMFLOAT4X4& parentTransform);

    /** @brief MeshData 1 件分の GPU バッファを作成する。*/
    void UploadMesh(ID3D11Device* device, MeshData& mesh);

    // --- ImportFromScene のサブルーティン ---
    void ImportTextures(const aiScene* scene, const std::string& baseDir);
    void ImportMaterials(const aiScene* scene);
    void ImportMeshes(const aiScene* scene);
    void ImportNodes(const aiScene* scene);
    void ImportSkins(const aiScene* scene);
    void ImportAnimations(const aiScene* scene);

    // --- ノード名・テクスチャパス解決用キャッシュ（ロード中のみ有効） ---

    /** @brief ノード名 → nodes インデックス（ImportNodes / ImportSkins / ImportAnimations で使用）。*/
    std::unordered_map<std::filesystem::path, int> m_nodeNameToIndex;

    /** @brief テクスチャ絶対パス → textures インデックス（ImportTextures / ImportMaterials で使用）。*/
    std::unordered_map<std::filesystem::path, int> m_texturePathToIndex;
};