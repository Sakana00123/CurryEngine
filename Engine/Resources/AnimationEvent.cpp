#include "pch.h"
#include "AnimationEvent.h"
#include <fstream>
#include "Engine/Utils/JsonFileHandler.h"
#include "Engine/Utils/JsonUtils.h"

namespace CurryEngine::Resources
{
	AnimationEventTrack& AnimationTimeline::AddEventTrack(const std::string& name)
	{
		m_eventTracks.push_back({ name, {} });
		return m_eventTracks.back();
	}

	void AnimationTimeline::RemoveEventTrack(size_t index)
	{
		if (index < m_eventTracks.size())
		{
			m_eventTracks.erase(m_eventTracks.begin() + index);
		}
	}

	bool AnimationTimeline::LoadFromFile(const std::string& path)
	{
		_path = path;
		std::filesystem::path filePath(path);
		filePath.replace_extension(".animtimeline"); // 拡張子を.animtimelineに変更
		if (std::filesystem::exists(filePath))
		{
			json j;
			JsonFileHandler::LoadJsonFromFile(j, filePath.string(), JsonIOFormat::Binary);
			m_targetClip = j.value("targetClip", m_targetClip);
			m_duration = j.value("duration", 0.0f);
			if (j.contains("eventTracks") && j["eventTracks"].is_array())
			{
				m_eventTracks.clear();
				for (const auto& trackJson : j["eventTracks"])
				{
					AnimationEventTrack track;
					track.name = trackJson.value("name", "");
					track.type = trackJson.value("type", AnimationEventType::Custom);
					if (trackJson.contains("keys") && trackJson["keys"].is_array())
					{
						for (const auto& keyJson : trackJson["keys"])
						{
							AnimationEventKey key;
							key.time = keyJson.value("time", 0.0f);
							key.eventName = keyJson.value("eventName", "");
							key.stringParam = keyJson.value("stringParam", "");
							track.keys.push_back(key);
						}
					}
					m_eventTracks.push_back(track);
				}
			}

			return true;
		}
		else
		{
			LOG_ERROR(std::format("File does not exist: {}", filePath.string()));
			return false;
		}
	}

	bool AnimationTimeline::SaveToFile(const std::filesystem::path& path) const
	{
		json j;
		j["targetClip"] = m_targetClip;
		j["duration"] = m_duration;
		j["eventTracks"] = json::array();
		for (const auto& track : m_eventTracks)
		{
			json trackJson;
			trackJson["name"] = track.name;
			trackJson["type"] = track.type;
			trackJson["keys"] = json::array();
			for (const auto& key : track.keys)
			{
				json keyJson;
				keyJson["time"] = key.time;
				keyJson["eventName"] = key.eventName;
				keyJson["stringParam"] = key.stringParam;
				trackJson["keys"].push_back(keyJson);
			}
			j["eventTracks"].push_back(trackJson);
		}
		std::filesystem::path filePath = path;
		filePath.replace_extension(".animtimeline"); // 拡張子を.animtimelineに変更
		JsonFileHandler::SaveJsonToFile(j, filePath.string(), JsonIOFormat::Binary);
		return true;
	}
}
