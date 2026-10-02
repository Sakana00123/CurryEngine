#pragma once
#include <string>

struct SceneViewConfig;

/** @brief EditorConfigManager を表すクラスです。 */
class EditorConfigManager
{
public:
	// EditorConfigManager の初期化
	/**
	 * @brief 初期化します。
	 */
	static void Initialize();
	// EditorConfigManager のシャットダウン
	/**
	 * @brief Shutdown の処理を行います。
	 */
	static void Shutdown();

	/// ----- EditorConfigManager API -----

	static void LoadConfig();
	/**
	 * @brief SaveConfig の処理を行います。
	 */
	static void SaveConfig();
	/**
	 * @brief LastOpenedScene を設定します。
	 */
	static void SetLastOpenedScene(const std::string& scenePath);
	/**
	 * @brief GetLastOpenedScene に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	static std::string GetLastOpenedScene();

	/**
	 * @brief GetSceneViewConfig に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	static SceneViewConfig* GetSceneViewConfig();

private:
	static inline std::string lastOpenedScenePath;
	static inline SceneViewConfig* viewConfig;
};