#pragma once
#include "Resource.h"
#include <vector>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>
#include <wrl\client.h>
#include <d3d11.h>
#include <DirectXMath.h>

/** @brief ModelAsset を表すクラスです。 */
class ModelAsset : public Resource
{
public:
	/**
	 * @brief ModelAsset を構築します。
	 */
	ModelAsset() = default;
	/**
	 * @brief ModelAsset を破棄します。
	 */
	virtual ~ModelAsset() = default;
	// ファイルからロード
	/**
	 * @brief LoadFromFile に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	bool LoadFromFile(const std::string& path) override;
	/**
	 * @brief Reload の処理を行います。
	 */
	bool Reload() override;


    /** @brief Scene を表す構造体です。 */
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

    /** @brief Node を表す構造体です。 */
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

    /** @brief IndexBufferView を表す構造体です。 */
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
    /** @brief VertexBufferView を表す構造体です。 */
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
    /** @brief Mesh を表す構造体です。 */
    struct Mesh {
        /** @brief Vertex を表す構造体です。 */
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

        /** @brief Primitive を表す構造体です。 */
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

    /** @brief BatchMesh を表す構造体です。 */
    struct BatchMesh {
        /** @brief Vertex を表す構造体です。 */
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


    /** @brief TextureInfo を表す構造体です。 */
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

    /** @brief NormalTextureInfo を表す構造体です。 */
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
    /** @brief OcclusionTextureInfo を表す構造体です。 */
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
    /** @brief PbrMetallicRoughness を表す構造体です。 */
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
    /** @brief Material を表す構造体です。 */
    struct Material {
        std::string name;
        /** @brief CBuffer を表す構造体です。 */
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

    /** @brief Texture を表す構造体です。 */
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
    /** @brief Image を表す構造体です。 */
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

    /** @brief Skin を表す構造体です。 */
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

    /** @brief Animation を表す構造体です。 */
    struct Animation {
        std::string name;
        float duration = 0.0f;

        /** @brief Channel を表す構造体です。 */
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

        /** @brief Sampler を表す構造体です。 */
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


    std::vector<Scene> scenes;
    int defaultScene = 0;
    std::vector<Node> nodes;
    std::vector<Mesh> meshes;
    std::vector<BatchMesh> batchMeshes;
    bool staticBatching = false; // 静的バッチングを行うか(初期化以降途中で変更しないこと)
    std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> buffers;
    std::vector<Material> materials;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> materialResourceView;
    std::vector<Texture> textures;
    std::vector<Image> images;
    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureResourceViews;
    std::vector<Skin> skins;
    std::vector<Animation> animations;

	/**
	 * @brief CumulateTransforms の処理を行います。
	 */
	void CumulateTransforms(std::vector<Node>& nodes);
	/**
	 * @brief 新しい要素を生成します。
	 */
	void CreateAndUploadResources(ID3D11Device* device);


};


/** @brief ModelInstance を表す構造体です。 */
struct ModelInstance
{
    std::vector<ModelAsset::Node> nodes;
};
