#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Rendering/Renderers/Renderer.h"
#include "Engine/Core/Math/BoundingBox.h"
//#include "Engine/Rendering/Renderers/SkinningData.h"
#include "Engine/Editor/Timeline.h"

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

#include "Engine/Audio/BeatManager.h"

struct AnimationEvent
{
    struct Event
    {
        float time = 0.0f; // イベント発生時間
        std::function<void()> func; // イベント関数
        bool isCalled = false; // イベントが呼び出されたか(内部用。設定不要)
    };

    std::vector<Event> events; // イベントリスト
};

class GltfModelRenderer : public Renderer
{
	C_REFLECT(GltfModelRenderer)

	C_PROPERTY()
    std::string filePath;

	C_PROPERTY()
    bool animationEnable = true;
	C_PROPERTY()
    bool blendEnable = true;

    bool isBlendStart = false;
    bool isAnimationCompleted = false;
	C_PROPERTY()
    bool enableShadow = true;
    bool onlyShadow = false;

    std::function<void(RenderContext*)> preRenderFunc;
    std::function<void(RenderContext*)> postRenderFunc;

    std::unordered_map<std::string, std::shared_ptr<tinygltf::Model>> gltfModels;

    // アニメーション中のイベント設定
    AnimationEvent animationEvent;

#ifdef _DEBUG
    bool editorStaticBatchingFlag = false;
#endif // _DEBUG

public:
    //Math::BoundingBox boundingBox;
    Math::BoundingBox CalculateAABB() const override;
public:
    void SetPreRenderFunction(const std::function<void(RenderContext*)>& func) {
        preRenderFunc = func;
    }
    void SetPostRenderFunction(const std::function<void(RenderContext*)>& func) {
        postRenderFunc = func;
    }

    void SetEnableShadow(bool enable) {
        enableShadow = enable;
    }
    bool IsEnableShadow() const {
        return enableShadow;
    }

    void SetEnableOnlyShadow(bool enable) {
        onlyShadow = enable;
    }
    bool IsEnableOnlyShadow() const {
        return onlyShadow;
    }

    // ピクセルシェーダーの差し替え
    void ReplacePixelShader(ID3D11Device* device, const char* filePath);
    void ReplaceVertexShader(ID3D11Device* device, const char* filePath);
    void ReplaceCSMVertexShader(ID3D11Device* device, const char* filePath);

    // アニメーション再生
    void SetAnimation(int index, bool blend = true, const AnimationEvent& animEvent = {}) {
        animationIndex = index; // アニメーションインデックスを設定
        time = 0; // アニメーション時間をリセット
        timeRate = 1.0f; // 再生速度をリセット
        isBlendStart = blend; // ブレンド開始フラグを設定
        isAnimationCompleted = false; // アニメーション完了フラグをリセット
        animationEvent = animEvent; // アニメーションイベントを設定
		time = BeatManager::GetTimeInCurrentBeat(); // ビートに同期させる
    }
    // アニメーション再生
    void SetAnimation(const std::string& name, bool blend = true, const AnimationEvent& animEvent = {})
    {
        SetAnimation(GetAnimationIndex(name), blend, animEvent);
    }

    // アニメーションの再生速度を設定
    void SetAnimationTimeRate(float rate) { timeRate = rate; }
    // アニメーションの再生速度を取得
    float GetAnimationTimeRate() const { return timeRate; }

    // アニメーションのブレンド時間を設定
    void SetAnimationBlendTime(float blendTime) { animationBlendTime = blendTime; }
    // アニメーションのブレンド時間を取得
    float GetAnimationBlendTime() const { return animationBlendTime; }

    // アニメーションのインデックスを名前から取得
    int GetAnimationIndex(const std::string& name) const {
        for (int i = 0; i < animations.size(); i++) {
            if (animations[i].name == name) {
                return i;
            }
        }
        return -1;
    }

    void SetStartAnimationTimer(float time)
    {
        this->time = time;
    }

    // 現在のアニメーション名を取得
    std::string GetCurrentAnimationName() const
    {
        if (animations.size() == 0) return "";
        if (animationIndex < 0 || animationIndex >= animations.size()) return "";
        return animations[animationIndex].name;
    }

    // アニメーションの総数を取得
    int GetAnimationCount() const { return static_cast<int>(animations.size()); }

    // 指定したインデックスのアニメーション名を取得
    std::string GetAnimationName(int index) const {
        if (index < 0 || index >= animations.size()) return "";
        return animations[index].name;
    }

    // アニメーションの長さを取得
    float GetAnimationDuration(int index) const {
        if (index < 0 || index >= animations.size()) return 0.0f;
        return animations[index].duration;
    }

    // アニメーションの長さを取得
    float GetCurrentAnimationDuration() const {
        return GetAnimationDuration(animationIndex);
    }

    // アニメーションが完了したか
    bool IsAnimationCompleted() const { return isAnimationCompleted; }

    // アニメーションの有効/無効を設定
    void SetAnimationEnable(bool enable) { animationEnable = enable; }

    // アニメーションが有効か
    bool IsAnimationEnable() const { return animationEnable; }

    // ブレンドの有効/無効を設定
    void SetBlendEnable(bool enable) { blendEnable = enable; }
    // ブレンドが有効か
    bool IsBlendEnable() const { return blendEnable; }
    // ループ設定
    void SetLoop(bool loop) { this->loop = loop; }
    // ループ取得
    bool IsLoop() const { return loop; }

    //指定のノード取得
    struct Node;
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

    // モデルのジョイントのワールド空間の position を返す関数
    DirectX::XMFLOAT3 GetJointWorldPosition(/*size_t nodeIndex,*/const std::string& name, const std::vector<Node>& animatedNodes, const DirectX::XMFLOAT4X4& transform)
    {
        // 該当するノードを探す
        for (auto needNode : animatedNodes)
        {
            if (needNode.name == name)
            {
                DirectX::XMFLOAT3 position = { 0,0,0 };
                const Node& node = needNode;
                DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&node.globalTransform) * DirectX::XMLoadFloat4x4(&transform);
                DirectX::XMStoreFloat3(&position, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&position), M));
                return position;
            }
        }

        // もしなければ
        _ASSERT("Node's name is mistake or here is not your want nodes!!");

        return { 0.0f,0.0f,0.0f };

    }
public:
    GltfModelRenderer();
    virtual ~GltfModelRenderer() = default;

	// モデルの読み込み
	void LoadModel(ID3D11Device* device, const std::string& filePath, bool staticBatching);

    void Initialize() override;
    void Update(float deltaTime) override;
    void Render(RenderContext* rtx) override;

    void CastShadow(RenderContext* rtx);
    void DrawProperty() override;

	// シリアライズ
	json Serialize() const override;

	// デシリアライズ
	void Deserialize(const json& jsonData) override;


    struct Scene {
        std::string name;
        std::vector<int> nodes; //Array of 'root' nodes

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("name", name),
                cereal::make_nvp("nodes", nodes)
            );
        }
    };
    std::vector<Scene> scenes;
    int defaultScene = 0;

    struct Node {
        std::string name;
        int skin = -1; // index of skin refereanced by this node
        int mesh = -1; // index of mesh refereanced by this node

        std::vector<int> children; // An array of indices of child nodes of this node

        //Local transforms
        DirectX::XMFLOAT4 rotation = { 0,0,0,1 };
        DirectX::XMFLOAT3 scale = { 1,1,1 };
        DirectX::XMFLOAT3 translation = { 0,0,0 };

        DirectX::XMFLOAT4X4 globalTransform = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("name", name),
                cereal::make_nvp("skin", skin),
                cereal::make_nvp("mesh", mesh),
                cereal::make_nvp("children", children),
                cereal::make_nvp("rotation", rotation),
                cereal::make_nvp("scale", scale),
                cereal::make_nvp("translation", translation),
                cereal::make_nvp("globalTransform", globalTransform)
            );
        }
    };
    std::vector<Node> nodes;

    struct IndexBufferView {
        int buffer = -1;
        UINT sizeInBytes = 0;
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("buffer", buffer),
                cereal::make_nvp("sizeInBytes", sizeInBytes),
                cereal::make_nvp("format", format)
            );
        }
    };
    struct VertexBufferView {
        int buffer = -1;
        UINT sizeInBytes = 0;
        UINT strideInBytes = 0;

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("buffer", buffer),
                cereal::make_nvp("sizeInBytes", sizeInBytes),
                cereal::make_nvp("strideInBytes", strideInBytes)
            );
        }
    };
    struct Mesh {
        struct Vertex {
            DirectX::XMFLOAT3 position = { 0,0,0 };
            DirectX::XMFLOAT3 normal = { 0,0,1 };
            DirectX::XMFLOAT4 tangent = { 1,0,0,1 };
            DirectX::XMFLOAT2 texcoord = { 0,0 };
            DirectX::XMUINT4 joints = { 0,0,0,0 };
            DirectX::XMFLOAT4 weights = { 1,0,0,0 };

            template<class T>
            void serialize(T& archive) {
                archive(
                    cereal::make_nvp("position", position),
                    cereal::make_nvp("normal", normal),
                    cereal::make_nvp("tangent", tangent),
                    cereal::make_nvp("texcoord", texcoord),
                    cereal::make_nvp("joints", joints),
                    cereal::make_nvp("weights", weights)
                );
            }
        };

        std::string name;

        struct Primitive {
            int material;

            std::vector<unsigned char> cachedIndices;
            IndexBufferView indexBufferView;

            std::vector<Vertex> cachedVertices;
            VertexBufferView vertexBufferView;

            std::unordered_map<std::string, DXGI_FORMAT> attributes;

            bool has(const char* attribute) const {
                return attributes.find(attribute) != attributes.end();
            }

            template<class T>
            void serialize(T& archive) {
                archive(
                    cereal::make_nvp("material", material),
                    cereal::make_nvp("cachedIndices", cachedIndices),
                    cereal::make_nvp("indexBufferView", indexBufferView),
                    cereal::make_nvp("cachedVertices", cachedVertices),
                    cereal::make_nvp("vertexBufferView", vertexBufferView),
                    cereal::make_nvp("attributes", attributes)
                );
            }
        };
        std::vector<Primitive> primitives;

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("name", name),
                cereal::make_nvp("primitives", primitives)
            );
        }
    };
    std::vector<Mesh> meshes;

    struct BatchMesh {
        struct Vertex {
            DirectX::XMFLOAT3 position = { 0,0,0 };
            DirectX::XMFLOAT3 normal = { 0,0,1 };
            DirectX::XMFLOAT4 tangent = { 1,0,0,1 };
            DirectX::XMFLOAT2 texcoord = { 0,0 };

            template<class T>
            void serialize(T& archive) {
                archive(
                    cereal::make_nvp("position", position),
                    cereal::make_nvp("normal", normal),
                    cereal::make_nvp("tangent", tangent),
                    cereal::make_nvp("texcoord", texcoord)
                );
            }
        };

        int material;

        std::vector<UINT> cachedIndices;
        IndexBufferView indexBufferView;

        std::vector<Vertex> cachedVertices;
        VertexBufferView vertexBufferView;

        std::unordered_map<std::string, DXGI_FORMAT> attributes;

        bool has(const char* attribute) const {
            return attributes.find(attribute) != attributes.end();
        }

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("material", material),
                cereal::make_nvp("cachedIndices", cachedIndices),
                cereal::make_nvp("indexBufferView", indexBufferView),
                cereal::make_nvp("cachedVertices", cachedVertices),
                cereal::make_nvp("vertexBufferView", vertexBufferView),
                cereal::make_nvp("attributes", attributes)
            );
        }
    };
    std::vector<BatchMesh> batchMeshes;
	bool staticBatching; // 静的バッチングを行うか(初期化以降途中で変更しないこと)

    std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> buffers;

    struct TextureInfo {
        int index = -1; // required.
        int texcoord = 0; // The set index of texture's TEXCOORD attribute used for texture coordinate mapping.

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("index", index),
                cereal::make_nvp("texcoord", texcoord)
            );
        }
    };

    struct NormalTextureInfo {
        int index = -1; // required.
        int texcoord = 0; // The set index of texture's TEXCOORD attribute used for texture coordinate mapping.
        float scale = 1; // scacledNormal = normalize((<sampled normal texture value> * 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("index", index),
                cereal::make_nvp("texcoord", texcoord),
                cereal::make_nvp("scale", scale)
            );
        }
    };
    struct OcclusionTextureInfo {
        int index = -1; // required.
        int texcoord = 0; // The set index of texture's TEXCOORD attribute used for texture coordinate mapping.
        float strength = 1; // A scalar parameter controlling the amount of occlusion applied. A value of `0.0` means no occlusion. A value of `1.0` means full occlusion. This value affects the final occlusion value as: `1.0 + strength * (<sampled occlusion texture value> - 1.0)`.

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("index", index),
                cereal::make_nvp("texcoord", texcoord),
                cereal::make_nvp("strength", strength)
            );
        }
    };
    struct PbrMetallicRoughness {
        float baseColorFactor[4] = { 1,1,1,1 }; // len = 4. default [1,1,1,1]
        TextureInfo baseColorTexture;
        float metallicFactor = 1;  // default 1
        float roughnessFactor = 1; // default 1
        TextureInfo metallicRoughnessTexture;

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("baseColorFactor", baseColorFactor),
                cereal::make_nvp("baseColorTexture", baseColorTexture),
                cereal::make_nvp("metallicFactor", metallicFactor),
                cereal::make_nvp("roughnessFactor", roughnessFactor),
                cereal::make_nvp("metallicRoughnessTexture", metallicRoughnessTexture)
            );
        }
    };
    struct Material {
        std::string name;
        struct CBuffer {
            float emissiveFactor[3] = { 0,0,0 }; //length 3. default
            int alphaMode = 0; // "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
            float alphaCutoff = 0.5f; // default 0.5
            int doubleSided = 0; // default false;

            PbrMetallicRoughness pbrMetallicRoughness;

            NormalTextureInfo normalTexture;
            OcclusionTextureInfo occlusionTexture;
            TextureInfo emissiveTexture;

            template<class T>
            void serialize(T& archive) {
                archive(
                    cereal::make_nvp("emissiveFactor", emissiveFactor),
                    cereal::make_nvp("alphaMode", alphaMode),
                    cereal::make_nvp("alphaCutoff", alphaCutoff),
                    cereal::make_nvp("doubleSided", doubleSided),
                    cereal::make_nvp("pbrMetallicRoughness", pbrMetallicRoughness),
                    cereal::make_nvp("normalTexture", normalTexture),
                    cereal::make_nvp("occlusionTexture", occlusionTexture),
                    cereal::make_nvp("emissiveTexture", emissiveTexture)
                );
            }
        };
        CBuffer data;

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("name", name),
                cereal::make_nvp("data", data)
            );
        }
    };
    std::vector<Material> materials;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> materialResourceView;

    struct Texture {
        std::string name;
        int source = -1;

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("name", name),
                cereal::make_nvp("source", source)
            );
        }
    };
    std::vector<Texture> textures;
    struct Image {
        std::string name;
        int width = -1;
        int height = -1;
        int component = -1;
        int bits = -1;				// bit depth per channel. 8(byte), 16 or 32.
        int pixelType = -1;			// pixel type(TINYGLTF_COMPONENT_TYPE_***). usually UBYTE(bits = 8) or USHORT(bits = 16)
        std::string mimeType;		// (required if no uri) ["image/jpeg", "image/png", "image/bmp", "image/gif"]
        std::string uri;			// (required if no mimeType) uri is not decoded(e.g. whitespace may be represented as %20)

        bool asIs = false;

        std::vector<unsigned char> cacheData;

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("name", name),
                cereal::make_nvp("width", width),
                cereal::make_nvp("height", height),
                cereal::make_nvp("component", component),
                cereal::make_nvp("bits", bits),
                cereal::make_nvp("pixelType", pixelType),
                cereal::make_nvp("mimeType", mimeType),
                cereal::make_nvp("uri", uri),
                cereal::make_nvp("asIs", asIs),
                cereal::make_nvp("cacheData", cacheData)
            );
        }
    };
    std::vector<Image> images;
    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureResourceViews;

    struct Skin {
        std::vector<DirectX::XMFLOAT4X4> inverseBindMatrices;
        std::vector<int> joints;

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("inverseBindMatrices", inverseBindMatrices),
                cereal::make_nvp("joints", joints)
            );
        }
    };
    std::vector<Skin> skins;

    struct Animation {
        std::string name;
        float duration = 0.0f;

        struct Channel {
            int sampler = -1; // required
            int targetNode = -1; // required (index of the node to target)
            std::string targetPath; // required in ["translation", "rotation", "scale", "weights"]

            template<class T>
            void serialize(T& archive) {
                archive(
                    cereal::make_nvp("sampler", sampler),
                    cereal::make_nvp("targetNode", targetNode),
                    cereal::make_nvp("targetPath", targetPath)
                );
            }
        };
        std::vector<Channel> channels;

        struct Sampler {
            int input = -1;
            int output = -1;
            std::string interpolation;

            template<class T>
            void serialize(T& archive) {
                archive(
                    cereal::make_nvp("input", input),
                    cereal::make_nvp("output", output),
                    cereal::make_nvp("interpolation", interpolation)
                );
            }
        };
        std::vector<Sampler> samplers;

        std::unordered_map<int/*sampler.input*/, std::vector<float>> timelines;
        std::unordered_map<int/*sampler.output*/, std::vector<DirectX::XMFLOAT3>> scales;
        std::unordered_map<int/*sampler.output*/, std::vector<DirectX::XMFLOAT4>> rotations;
        std::unordered_map<int/*sampler.output*/, std::vector<DirectX::XMFLOAT3>> translations;

        template<class T>
        void serialize(T& archive) {
            archive(
                cereal::make_nvp("name", name),
                cereal::make_nvp("duration", duration),
                cereal::make_nvp("channels", channels),
                cereal::make_nvp("samplers", samplers),
                cereal::make_nvp("timelines", timelines),
                cereal::make_nvp("scales", scales),
                cereal::make_nvp("rotations", rotations),
                cereal::make_nvp("translations", translations)
            );
        }
    };
    std::vector<Animation> animations;



private:
    void CumulateTransforms(std::vector<Node>& nodes);
    void FetchNodes(const tinygltf::Model& gltfModel);
    void FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel);
    void FetchBatchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel);
    void FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel);
    void FetchTextures(ID3D11Device* device, const tinygltf::Model& gltfModel);
    void FetchAnimations(const tinygltf::Model& gltfModel);

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShaderCsm;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShaderCsm;

    struct PrimitiveConstants {
        DirectX::XMFLOAT4X4 world;
        int material{ -1 };
        int hasTangent{ 0 };
        int skin{ -1 };
        int num{ -1 }; // ?
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveCbuffer;


    static const size_t PRIMITIVE_MAX_JOINTS = 512;
    struct PrimitiveJointConstants {
        DirectX::XMFLOAT4X4 matrices[PRIMITIVE_MAX_JOINTS];
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveJointCbuffer;

    void CreateAndUploadResources(ID3D11Device* device);

	friend class RhythmAnimationController;
    void Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes);

public:
    float time = 0; // アニメーションの経過時間(秒)
    float timeRate = 1.0f; // アニメーションの再生速度
    float animationBlendTime = 1.2f; // アニメーションのブレンド時間(秒)
    int animationIndex = 0; // 現在のアニメーションインデックス
    bool loop = true;//ループ設定

};