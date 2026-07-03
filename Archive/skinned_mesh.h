#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <fbxsdk.h>

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>

struct CbScene
{
	struct Node
	{
		uint64_t unique_id{ 0 };
		std::string name;
		FbxNodeAttribute::EType attribute{ FbxNodeAttribute::EType::eUnknown };
		int64_t parent_index{ -1 };

		template<class T>
		void serialize(T& archive)
		{
			archive(unique_id, name, attribute, parent_index);
		}
	};
	std::vector<Node> nodes;

	template<class T>
	void serialize(T& archive)
	{
		archive(nodes);
	}

	int64_t IndexOf(uint64_t unique_id) const
	{
		int64_t index{ 0 };
		for (const Node& node : nodes)
		{
			if (node.unique_id == unique_id)
			{
				return index;
			}
			++index;
		}
		return -1;
	}
};

struct BoneInfluence
{
	uint32_t boneIndex;
	float boneWeight;
};
using BoneInfluencesPerControlPoint = std::vector<BoneInfluence>;

struct Animation
{
	std::string name;
	float samplingRate{ 0 };

	struct KeyFrame
	{
		struct Node
		{
			// 'globalTransform' is used to convert from local space of node to global space of scene.
			DirectX::XMFLOAT4X4 globalTransform{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };

			// The transformation data of a node includes its translation, rotation and scaling vectors
			// with respect to its parent.
			DirectX::XMFLOAT3 scaling{ 1,1,1 };
			DirectX::XMFLOAT4 rotation{ 0,0,0,1 };//Rotation	quaternion
			DirectX::XMFLOAT3 translation{ 0,0,0 };

			template<class T>
			void serialize(T& archive)
			{
				archive(globalTransform, scaling, rotation, translation);
			}
		};
		std::vector<Node> nodes;

		template<class T>
		void serialize(T& archive)
		{
			archive(nodes);
		}
	};
	std::vector<KeyFrame> sequence;

	template<class T>
	void serialize(T& archive)
	{
		archive(name, samplingRate, sequence);
	}
};

class SkinnedMesh
{
public:
	struct Skeleton
	{
		struct Bone
		{
			uint64_t uniqueID{ 0 };
			std::string name;
			// 'parentIndex' is index that refers to the parent bone's position in the array that contains itself.
			int64_t parentIndex{ -1 }; // -1 : the bone is orphan
			// 'nodeIndex' is an index that refers to the node array of the scene.
			int64_t nodeIndex{ 0 };

			// 'offsetTransform' is used to convert from model(mesh) space to bone(node) scene.
			DirectX::XMFLOAT4X4 offsetTransform{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };

			bool IsOrphan() const { return parentIndex < 0; };

			template<class T>
			void serialize(T& archive)
			{
				archive(uniqueID, name, parentIndex, nodeIndex, offsetTransform);
			}
		};
		std::vector<Bone> bones;
		template<class T>
		void serialize(T& archive)
		{
			archive(bones);
		}

		int64_t IndexOf(uint64_t uniqueID) const
		{
			int64_t index{ 0 };
			for (const Bone& bone : bones)
			{
				if (bone.uniqueID == uniqueID) return index;
				++index;
			}
			return -1;
		}
	};

	
	std::vector<Animation> animationClips;

	static const int MAX_BONE_INFLUENCES{ 4 };
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal{ 0,1,0 };
		DirectX::XMFLOAT4 tangent{ 1,0,0,1 };
		DirectX::XMFLOAT2 texcoord{ 0,0 };
		FLOAT boneWeights[MAX_BONE_INFLUENCES]{ 1,0,0,0 };
		INT boneIndices[MAX_BONE_INFLUENCES]{};

		template<class T>
		void serialize(T& archive)
		{
			archive(position, normal, tangent, texcoord, boneWeights, boneIndices);
		}
	};
	static const int MAX_BONES{ 256 };
	struct Constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 material_color;
		DirectX::XMFLOAT4X4 bone_transforms[MAX_BONES]{ { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 } };
	};
	struct Mesh
	{
		uint64_t unique_id{ 0 };
		std::string name;
		// 'node_index' is an index that refers to the node array of the scene.
		int64_t node_index{ 0 };

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		struct Subset
		{
			uint64_t material_unique_id{ 0 };
			std::string material_name;

			uint32_t start_index_location{ 0 };
			uint32_t index_count{ 0 };

			template<class T>
			void serialize(T& archive)
			{
				archive(material_unique_id, material_name, start_index_location, index_count);
			}
		};
		std::vector<Subset> subsets;


		//Transform
		DirectX::XMFLOAT4X4 defaultGlobalTransform{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };

		Skeleton bindPose;

		DirectX::XMFLOAT3 boundingBox[2]
		{
			{+D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX},
			{-D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX}
		};

		template<class T>
		void serialize(T& archive)
		{
			archive(unique_id, name, node_index, vertices, indices, subsets, defaultGlobalTransform,
				bindPose, boundingBox);
		}

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer; 
		friend class SkinnedMesh;
	};
	std::vector<Mesh> meshes;

	struct Material
	{
		uint64_t unique_id{ 0 };
		std::string name;
		DirectX::XMFLOAT4 Ka{ 0.2f, 0.2f, 0.2f, 1.0f };
		DirectX::XMFLOAT4 Kd{ 0.8f, 0.8f, 0.8f, 1.0f };
		DirectX::XMFLOAT4 Ks{ 1.0f,1.0f,1.0f,1.0f };
		std::string texture_filenames[4];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[4];

		template<class T>
		void serialize(T& archive)
		{
			archive(unique_id, name, Ka, Kd, Ks, texture_filenames);
		}
	};
	std::unordered_map<uint64_t, Material> materials;

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader; 
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;
public:
	SkinnedMesh(ID3D11Device* device, const char* fbx_filename, bool triangulate = false, float samplingRate = 0);
	virtual ~SkinnedMesh() = default;

	//アニメーション追加読み込み
	bool AppendAnimations(const char* animationFilename, float samplingRate);

	void BlendAnimations(const Animation::KeyFrame* keyframes[2], float factor,
		Animation::KeyFrame& keyframe);

	void UpdateAnimation(Animation::KeyFrame& keyframe);

	void Render(ID3D11DeviceContext* immediate_context,
		const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color,
		const Animation::KeyFrame* keyframe);

protected:
	void FetchMeshes(FbxScene* fbxScene, std::vector<Mesh>& meshes);

	void FetchMaterials(FbxScene* fbxScene, std::unordered_map<uint64_t, Material>& materials);

	void FetchSkeleton(FbxMesh* fbxMesh, Skeleton& bindPose);

	void FetchAnimations(FbxScene* fbxScene, std::vector<Animation>& animationClips,
		float samplingRate /*If this value is 0, the animation data will be sampled at the default frame rate.*/);

	void CreateComObjects(ID3D11Device* device, const char* fbxFilename);
	
	CbScene scene_view;
};