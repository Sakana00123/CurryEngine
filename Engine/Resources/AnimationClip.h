#pragma once
#include "Resource.h"
#include "Engine/Core/Misc.h"
#include "Engine/Utils/JsonUtils.h"
#include <filesystem>
#include <cereal\archives\binary.hpp>
#include <cereal\archives\json.hpp>
#include <cereal\types\string.hpp>
#include <cereal\types\unordered_map.hpp>
#include <cereal\types\vector.hpp>

struct NodePose
{
	DirectX::XMFLOAT3 translation;
	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT3 scale;
};



class AnimationClip : public Resource
{
public:
	std::string name;
	float duration = 0.0f;

	struct Channel {
		int sampler = -1; // required
		int targetNode = -1; // required (index of the node to target)
		std::string targetPath; // required in ["translation", "rotation", "scale", "weights"]

		template<class Archive>
		void serialize(Archive& archive) {
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
		template<class Archive>
		void serialize(Archive& archive) {
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


	bool LoadFromFile(const std::string& path) override;

	bool SaveToFile(const std::filesystem::path& path) const;

	// アニメーションをサンプリングして、指定された時間における各ノードのポーズを取得する
	void Sample(float time, std::vector<NodePose>& out, float weight = 1.0f) const;
};
