#include "pch.h"
#include "BuildSettings.h"
#include "Engine/Utils/JsonFileHandler.h"
#include "Engine/Editor/Console.h"

void BuildSettings::Load(const std::string& path)
{
	// ビルド設定の読み込み
	json j;
	if (JsonFileHandler::LoadJsonFromFile(j, path))
	{
		try
		{
			appName = j.value("appName", appName);
			iconPath = j.value("iconPath", iconPath);
			outputDir = j.value("outputDir", outputDir);
			zipToolPath = j.value("zipToolPath", zipToolPath);
			
			copyItems.clear();
			if (j.contains("copyItems") && j["copyItems"].is_array())
			{
				for (const auto& item : j["copyItems"]) {
					CopyItem copyItem{
						.src = item.value("src", ""),
						.type = item.value("type", ""),
						.exclude = item.value("exclude", std::vector<std::string>{})
					};
					copyItems.push_back(copyItem);
				}
			}
		}
		catch (const std::exception& e)
		{
			LOG_ERROR(std::string("Error parsing build settings: ") + e.what());
		}
	}
	else
	{
		LOG_ERROR("Failed to load build settings.");
	}
}

void BuildSettings::Save(const std::string& path) const
{
	// ビルド設定の保存
	json j;
	j["appName"] = appName;
	j["iconPath"] = iconPath;
	j["outputDir"] = outputDir;
	j["zipToolPath"] = zipToolPath;
	j["copyItems"] = json::array();
	for (const auto& item : copyItems) {
		json copyItem;
		copyItem["src"] = item.src;
		copyItem["type"] = item.type;
		copyItem["exclude"] = item.exclude;
		j["copyItems"].push_back(copyItem);
	}
	JsonFileHandler::SaveJsonToFile(j, path);
}