#pragma once
#define NOMINMAX
#include <string>
#include <vector>

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

class GltfModel
{
    std::string filePath;
public:
    GltfModel(ID3D11Device* device, const std::string& filePath);
    virtual ~GltfModel() = default;

    struct CbScene
    {
        std::string name;
        std::vector<int> nodes; // Array of 'root' nodes
    };
    std::vector<CbScene> scenes;

    struct Skin
    {
        std::vector<DirectX::XMFLOAT4X4> inverseBindMatrices;
        std::vector<int> joints;
    };
    std::vector<Skin> skins;

    // GLTFのアニメーションデータを表す構造体
    struct Animation
    {
        // アニメーションの名前（任意）
        std::string name;
        // アニメーション全体の長さ（最大のタイムライン値）
        float duration{ 0.0f };

        // チャンネル：ノードのどのプロパティに、どのサンプラーを適用するかを定義
        struct Channel
        {
            // 使用するサンプラー（samplers のインデックス）
            int sampler{ -1 };

            // 対象となるノード（animatedNodesのインデックス）
            int targetNode{ -1 };

            // 対象となるプロパティ名（例："translation", "rotation", "scale"）
            std::string targetPath;
        };
        // アニメーションに含まれるチャンネルのリスト
        std::vector<Channel> channels;

        // サンプラー：時間と値の対応を定義し、補間方法も持つ
        struct Sampler
        {
            // 入力（時間配列）の ID（timelines のキー）
            int input{ -1 };

            // 出力（値配列）の ID（scales/rotations/translations のキー）
            int output{ -1 };

            // 補間方法（"LINEAR", "STEP", "CUBICSPLINE" など）
            std::string interpolation;
        };
        // アニメーションに使用されるサンプラーのリスト
        std::vector<Sampler> samplers;

        // 各サンプラーの input に対応する時間値（キーフレームの時間）
        std::unordered_map<int /* sampler.input */, std::vector<float>> timelines;

        // 各サンプラーの output に対応するスケール値
        std::unordered_map<int /* sampler.output */, std::vector<DirectX::XMFLOAT3>> scales;

        // 各サンプラーの output に対応する回転値（クォータニオン）
        std::unordered_map<int /* sampler.output */, std::vector<DirectX::XMFLOAT4>> rotations;

        // 各サンプラーの output に対応する位置（平行移動）値
        std::unordered_map<int /* sampler.output */, std::vector<DirectX::XMFLOAT3>> translations;
    };

    // GLTFファイルから読み込まれたアニメーションの一覧
    std::vector<Animation> animations;


    struct Node
    {
        std::string name;
        int skin{ -1 };// index of skin referenced by this node
        int mesh{ -1 };// index of mesh referenced by this node

        std::vector<int> children; // An array of indices of child nodes of this node

        // Local transforms
        DirectX::XMFLOAT4 rotation{ 0,0,0,1 };
        DirectX::XMFLOAT3 scale{ 1,1,1 };
        DirectX::XMFLOAT3 translation{ 0,0,0 };

        DirectX::XMFLOAT4X4 globalTransform{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
    };
    std::vector<Node> nodes;

    struct BufferView
    {
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        size_t strideInBytes{ 0 };
        size_t sizeInBytes{ 0 };
        size_t Count() const
        {
            return sizeInBytes / strideInBytes;
        }
    };
    struct Mesh
    {
        std::string name;
        struct Primitive
        {
            int material;
            std::map<std::string, BufferView> vertexBufferViews;
            BufferView indexBufferView;
        };
        std::vector<Primitive> primitives;
    };
    std::vector<Mesh> meshes;

    struct TextureInfo
    {
        int index = -1;
        int texcoord = 0;
    };
    struct NormalTextureInfo
    {
        int index = -1;
        int texcoord = 0;
        float scale = 1;
    };
    struct OcclusionTextureInfo
    {
        int index = -1;
        int texcoord = 0;
        float strength = 1;
    };
    struct PBRMaterialRoughness
    {
        float baseColorFactor[4] = { 1,1,1,1 };
        TextureInfo baseColorTexture;
        float metallicFactor = 1;
        float roughnessFactor = 1;
        TextureInfo metallicRoughnessTexture;
    };
    struct Material
    {
        std::string name;
        // TODO:01ここ追加
        Microsoft::WRL::ComPtr<ID3D11PixelShader> replacedPixelShader{ nullptr };//かスラムシェーダー
        struct CBuffer
        {
            float emissiveFactor[3] = { 0,0,0 };
            int alphaMode = 0; //"OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
            float alphaCutoff = 0.5f;
            bool doubleSided = false;

            PBRMaterialRoughness pbrMetallicRoughness;

            NormalTextureInfo normalTexture;
            OcclusionTextureInfo occlusionTexture;
            TextureInfo emissiveTexture;
        };
        CBuffer data;
    };
    std::vector<Material> materials;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> materialResourceView;

    struct Texture
    {
        std::string name;
        int source{ -1 };
    };
    std::vector<Texture> textures;
    struct Image
    {
        std::string name;
        int width{ -1 };
        int height{ -1 };
        int component{ -1 };
        int bits{ -1 };
        int pixelType{ -1 };
        int bufferView;
        std::string mimeType;
        std::string uri;
        bool asIs{ false };
    };
    std::vector<Image> images;
    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureResourceViews;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;


    //定数バッファ----------------------------------------------------

    struct PrimitiveConstants
    {
        DirectX::XMFLOAT4X4 world;
        int material{ -1 };
        int hasTangent{ 0 };
        int skin{ -1 };
        int num{ -1 };
    };
    PrimitiveConstants primitiveData{};

    Microsoft::WRL::ComPtr<ID3D11Buffer> primitive_cbuffer;

    static const size_t PRIMITIVE_MAX_JOINTS = 512;
    struct PrimitiveJointConstants
    {
        DirectX::XMFLOAT4X4 matrices[PRIMITIVE_MAX_JOINTS];
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveJoint_cbuffer;

    //---------------------------------------------------------------------

    //アニメーション追加読み込み
    bool AppendAnimations(const std::string& filePath);

    void UpdateAnimation(float elapsedTime);

    void SetAnimation(int index, bool blend = true) {
        animationIndex = index;
        time = 0;
        timeRate = 1.0f;
        isBlendStart = blend;
        isAnimationCompleted = false;
    }
    float time = 0;
    float timeRate = 1.0f;
    float animationBlendTime = 1.2f;
    int animationIndex = 0;
    
    //アニメーションの再生範囲を設定
    void SetAnimationRange(float start, float end) 
    {
        rangeStart = start;
        rangeEnd = end;
        useRange = true;
    }
    float rangeStart = 0.0f;
    float rangeEnd = 0.0f;
    bool useRange = false;
    //指定のノード取得
    Node* FindNode(const std::string& name) {
        //指定のノードの名前が存在するか検索
        for (Node& node : nodes) {
            if (node.name == name) {
                return &node;
            }
        }
        //指定のノードが見つからなかったらnullを返す
        return nullptr;
    }

    //アニメーションが終端に到達したか（ループなしのみ）
    bool IsAnimationCompleted() const { return isAnimationCompleted; }

    bool loop = true;//ループ設定

    void Render(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& world,
        const std::vector<Node>& animatedNodes = {}, ID3D11PixelShader* replacePixelShader = nullptr, ID3D11VertexShader* replaceVertexShader = nullptr);

    void SecondRender(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& world,
        const std::vector<Node>& animatedNodes = {}, ID3D11PixelShader* replacePixelShader = nullptr, ID3D11VertexShader* replaceVertexShader = nullptr);

    void Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes);
private:
    bool isBlendStart = false;
    bool isAnimationCompleted = false;
private:

    BufferView MakeBufferView(const tinygltf::Accessor& accessor);

    void CumulateTransforms(std::vector<Node>& nodes);

    void FetchNodes(const tinygltf::Model& gltfModel);

    void FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel);

    void FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel);

    void FetchTextures(ID3D11Device* device, const tinygltf::Model& gltfModel);

    void FetchAnimations(const tinygltf::Model& gltfModel);
};