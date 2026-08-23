#include "pch.h"
#include "AnimationClip.h"
#include <fstream>

bool AnimationClip::LoadFromFile(const std::string& path)
{
	_path = path;
	std::filesystem::path filePath(path);

	if (std::filesystem::exists(filePath))
	{
		// バイナリ形式での読み込み
		std::ifstream file(filePath.string().c_str(), std::ios::binary);
		if (!file.is_open())
		{
			std::cerr << "Failed to open file: " << filePath << std::endl;
			return false;
		}
		cereal::BinaryInputArchive deserializer(file);
		try
		{
			deserializer(
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
		catch (const cereal::Exception& e)
		{
			LOG_ERROR(std::format("Failed to deserialize AnimationClip from file: {}. Error: {}", filePath.string(), e.what()));
			return false;
		}
		return true;
	}
	else
	{
		LOG_ERROR(std::format("File does not exist: {}", filePath.string()));
		return false;
	}

}

bool AnimationClip::SaveToFile(const std::filesystem::path& path) const
{
	std::ofstream file(path.string().c_str(), std::ios::binary);
	if (!file.is_open())
	{
		LOG_ERROR(std::format("Failed to open file for writing: {}", path.string()));
		return false;
	}
	cereal::BinaryOutputArchive serializer(file);
	try
	{
		serializer(
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
	catch (const cereal::Exception& e)
	{
		LOG_ERROR(std::format("Failed to serialize AnimationClip to file: {}. Error: {}", path.string(), e.what()));
		return false;
	}
	return true;
}


void AnimationClip::Sample(float time, std::vector<NodePose>& out, float weight) const
{
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

	float blendRate = weight < 1.f ? weight : 1.f;
    {
        // アニメーションの各チャネルを処理
        for (std::vector<Channel>::const_reference channel : channels)
        {
            const Sampler& sampler{ samplers.at(channel.sampler) };
            const std::vector<float>& timeline{ timelines.at(sampler.input) };

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
                const std::vector<XMFLOAT3>& l_scales{ scales.at(sampler.output) };

                XMVECTOR S0 = XMLoadFloat3((blendRate < 1.f) ? &out.at(channel.targetNode).scale : &l_scales.at(keyframeIndex + 0));
                XMVECTOR S1 = XMLoadFloat3(&l_scales.at(keyframeIndex + 1));

                // 線形補間でスケールを求めてノードに格納
                XMStoreFloat3(&out.at(channel.targetNode).scale,
                    XMVectorLerp(S0, S1, rate));
            }
            else if (channel.targetPath == "rotation")
            {
                const std::vector<XMFLOAT4>& l_rotations{ rotations.at(sampler.output) };

                XMVECTOR R0 = XMLoadFloat4((blendRate < 1.f) ? &out.at(channel.targetNode).rotation : &l_rotations.at(keyframeIndex + 0));
                XMVECTOR R1 = XMLoadFloat4(&l_rotations.at(keyframeIndex + 1));

                // 球面線形補間（Slerp）で回転を補間し、正規化して適用
                XMStoreFloat4(&out.at(channel.targetNode).rotation,
                    XMQuaternionNormalize(XMQuaternionSlerp(R0, R1, rate)));
            }
            else if (channel.targetPath == "translation")
            {
                const std::vector<XMFLOAT3>& l_translations{ translations.at(sampler.output) };

                XMVECTOR T0 = XMLoadFloat3((blendRate < 1.f) ? &out.at(channel.targetNode).translation : &l_translations.at(keyframeIndex + 0));
                XMVECTOR T1 = XMLoadFloat3(&l_translations.at(keyframeIndex + 1));

                // 線形補間で位置を求めてノードに格納
                XMStoreFloat3(&out.at(channel.targetNode).translation,
                    XMVectorLerp(T0, T1, rate));
            }
            else if (channel.targetPath == "weight") {

            }
            else {

            }
        }
    }
}
