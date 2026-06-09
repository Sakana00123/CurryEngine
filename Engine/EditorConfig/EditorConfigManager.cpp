#include "pch.h"
#include "EditorConfigManager.h"
#include "Engine/Utils/JsonFileHandler.h"
#include "Engine/Core/EnginePaths.h"

#include "IEditorConfig.h"
#include "SceneViewConfig.h"

#define SAFE_DELETE(rawPtr) \
	if (rawPtr) { \
		delete rawPtr; \
		rawPtr = nullptr; \
	}

#define CREATE_DEFAULT_CONFIG_DATA_PTR(_Type, _RawPtr) \
	SAFE_DELETE(_RawPtr) \
	_RawPtr = new _Type; \
	_RawPtr->ResetToDefault();

// JSON に書き込む際に、該当するポインタが存在すればシリアライズするマクロ
#define SERIALIZE_CONFIG_DATA_PTR(_RawPtr, _Json, _Key) \
	if (_RawPtr) { \
		_Json[_Key] = _RawPtr->Serialize(); \
	}

// JSON から読み込む際に、該当するキーが存在すればデシリアライズするマクロ
#define DESERIALIZE_CONFIG_DATA_PTR(_RawPtr, _Json, _Key) \
	if (_Json.contains(_Key)) { \
		_RawPtr->Deserialize(_Json[_Key]); \
	}

// LoadConfig 内で、デフォルトの設定を作成してから JSON から読み込むためのマクロ(キーは_RawPtrの名前になります)
#define SETUP_CONFIG_DATA_PTR(_Type, _RawPtr, _Json) \
	CREATE_DEFAULT_CONFIG_DATA_PTR(_Type, _RawPtr) \
	DESERIALIZE_CONFIG_DATA_PTR(_RawPtr, _Json, #_RawPtr)

void EditorConfigManager::Initialize()
{
	LoadConfig();
}

void EditorConfigManager::Shutdown()
{
	SaveConfig();
	SAFE_DELETE(viewConfig);
}

void EditorConfigManager::LoadConfig()
{
	json configJson;
	if (JsonFileHandler::LoadJsonFromFile(configJson, EnginePaths::EditorConfigFile))
	{
		if (configJson.contains("lastOpenedScenePath"))
		{
			lastOpenedScenePath = configJson["lastOpenedScenePath"].get<std::string>();
		}
		SETUP_CONFIG_DATA_PTR(SceneViewConfig, viewConfig, configJson);
	}
	else
	{
		LOG_ERROR("Failed to load editor config.");
	}
}

void EditorConfigManager::SaveConfig()
{
	json configJson;
	{
		configJson["lastOpenedScenePath"] = lastOpenedScenePath;
		SERIALIZE_CONFIG_DATA_PTR(viewConfig, configJson, "viewConfig");
	}

	JsonFileHandler::SaveJsonToFile(configJson, EnginePaths::EditorConfigFile);
}

std::string EditorConfigManager::GetLastOpenedScene()
{
	return lastOpenedScenePath;
}

void EditorConfigManager::SetLastOpenedScene(const std::string& scenePath)
{
	lastOpenedScenePath = scenePath;
}

SceneViewConfig* EditorConfigManager::GetSceneViewConfig()
{
	return viewConfig;
}