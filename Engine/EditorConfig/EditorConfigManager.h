#pragma once
#include <string>

struct SceneViewConfig;

class EditorConfigManager
{
public:
	// EditorConfigManager の初期化
	static void Initialize();
	// EditorConfigManager のシャットダウン
	static void Shutdown();

	/// ----- EditorConfigManager API -----

	static void LoadConfig();
	static void SaveConfig();
	static void SetLastOpenedScene(const std::string& scenePath);
	static std::string GetLastOpenedScene();

	static SceneViewConfig* GetSceneViewConfig();

private:
	static inline std::string lastOpenedScenePath;
	static inline SceneViewConfig* viewConfig;
};