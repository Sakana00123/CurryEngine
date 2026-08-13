#include "pch.h"
#include "AnimationClip.h"
#include <fstream>

namespace
{
	const char* TrackTypeToString(TrackType type)
	{
		switch (type) { case TrackType::Value: return "Value"; case TrackType::Event: return "Event"; case TrackType::Curve: return "Curve"; case TrackType::State: return "State"; default: return "Unknown"; }
	}

	bool TrackTypeFromString(const std::string& value, TrackType& type)
	{
		if (value == "Value") { type = TrackType::Value; return true; }
		if (value == "Event") { type = TrackType::Event; return true; }
		if (value == "Curve") { type = TrackType::Curve; return true; }
		if (value == "State") { type = TrackType::State; return true; }
		return false;
	}
}

//REGISTER_RESOURCE(AnimationClip, "AnimationClip")


void ValueTrack::Evaluate(float prevTime, float currentTime, AnimationContext& context)
{
	
}

void ValueTrack::Sort()
{
	std::sort(keys.begin(), keys.end(),
		[](const ValueKeyframe& a, const ValueKeyframe& b)
		{
			return a.time < b.time;
		});
}

void EventTrack::Evaluate(float prevTime, float currentTime, AnimationContext& context)
{
	// イベントトラックの評価ロジックをここに実装
}

void CurveTrack::Evaluate(float prevTime, float currentTime, AnimationContext& context)
{
	// カーブトラックの評価ロジックをここに実装
}

void StateTrack::Evaluate(float prevTime, float currentTime, AnimationContext& context)
{
	// ステートトラックの評価ロジックをここに実装
}


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
