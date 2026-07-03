#pragma once
#include <string>
#include <memory>
#include <future>
#include <queue>
#include "Engine/Rendering/Pipeline/RenderContext.h"
#include "Engine/Utils/JsonFileHandler.h"
#include "Scene.h"

/**
 * @file
 * @brief シーンの生成・切替・更新・描画を管理するマネージャ。
 * @details シーンの登録、同期/非同期ロード、ローディングシーン制御、
 *          ライフサイクル呼び出し（Update/Render/Draw など）を提供します。
 */
class SceneManager
{
public:
	/** @brief シーンの状態列挙型。*/
	enum class State
	{
		Editing, // エディットモード
		Playing, // プレイモード
		Paused, // ポーズモード
		EditToPlay, // エディットからプレイへ移行中
		PlayToEdit, // プレイからエディットへ移行中
	};
	/** @brief シーンの状態。*/
#ifdef _DEBUG
	static inline State state = State::Editing;
#else
	static inline State state = State::Playing;
#endif // _DEBUG

	/** @brief シーンエントリ構造体。*/
	struct SceneEntry
	{
		bool enabled;
		std::string name;
		std::string path;

	private:
#ifdef USE_IMGUI
		// シリアライズ対象外のフィールド
		friend class BuildSettingsWindow;
		bool selected = false; // ビルド設定ウィンドウでの選択状態  
#endif // USE_IMGUI
	};
	/** @brief 登録されているシーンのエントリリスト。*/
	static inline std::vector<SceneEntry> sceneEntries;

	/** @brief シーンデータ構造体。*/
	struct SceneData
	{
		std::string sceneName;
		json sceneJson;
	};
	/** @brief 前回のシーンデータ。*/
	static inline SceneData previousData;
	/** @brief ランタイム用シーン名の接尾辞。*/
	static inline std::string runtimeSuffix = "_runtime";
private:
	friend class Framework;
	friend class EditorGUI;
	friend class BuildSettingsWindow;
	/** @brief 最初に起動するシーン名。*/
	static inline std::string firstSceneName;
	/** @brief 登録されているシーン名のリスト。*/
	static inline std::vector<std::string> sceneNames;
	/** @brief 現在のシーン。*/
	static inline std::unique_ptr<Scene> currentScene;
	/** @brief 次のシーン（ロード中）。*/
	static inline std::unique_ptr<Scene> nextScene;
	/** @brief ローディングシーン名。*/
	static inline std::string loadingSceneName = "EmptyScene";
	static inline std::string editorLoadingSceneName = "EmptyScene";
	/** @brief ロード待ちのシーン名。*/
	static inline std::string pendingSceneName;
	/** @brief シーンロード待ちキュー。*/
	static inline std::queue<std::unique_ptr<Scene>> sceneLoadQueue;
	/** @brief 非同期ロードの未来オブジェクト。*/
	static inline std::future<void> future;
public:
	/** @brief シーンマネージャを初期化します。*/
	static void Initialize();
	/** @brief フレームの開始処理を呼び出します。*/
	static void BeginFrame();
	/** @brief フレームの終了処理を呼び出します。*/
	static void EndFrame();
	
	/**
	 * @brief フレーム更新を行います。
	 * @param deltaTime 経過時間（秒）。
	 */
	static void Update(float deltaTime);
	/**
	 * @brief 3D 描画の前処理を呼び出します。
	 * @param rtx レンダーコンテキスト。
	 */
	static void BeginRendering(RenderContext* rtx);
	/**
	 * @brief 3D 描画処理（本処理）を呼び出します。
	 * @param rtx レンダーコンテキスト。
	 */
	static void Render(RenderContext* rtx);
	/**
	 * @brief 3D 描画の後処理を呼び出します。
	 * @param rtx レンダーコンテキスト。
	 */
	static void EndRendering(RenderContext* rtx);
	/**
	 * @brief 2D 描画処理を呼び出します。
	 * @param rtx レンダーコンテキスト。
	 */
	static void Draw(RenderContext* rtx);
	/**
	 * @brief GUI 描画処理を呼び出します。
	 * @param acceptRendering シーンレンダリング受け入れ判定。
	 */
	static void DrawGUI(RenderContext* sceneRtx, RenderContext* gameRtx);
	/**
	 * @brief マネージャとシーンの終了処理を行います。
	 */
	static void Finalize();

	/* シーンマネージャの状態を保存します。*/
	static void SaveSettings();
public:
	/**
	 * @brief 最初に起動するシーンを設定します。
	 * @param name シーン名（登録名）。
	 */
	static void SetFirstScene(const std::string& name);

	/**
	 * @brief 最初のシーン名を更新します。
	 * @details sceneEntries の最初の有効なシーンを firstSceneName に設定します。
	 */
	static void UpdateFirstSceneName();

	/**
	 * @brief 最初のシーンをロードして切り替えます。
	 */
	static void LoadFirstScene();

	/**
	 * @brief ローディングシーンを設定します。
	 * @param name ローディング表示に用いるシーン名。
	 * @details 既存のシーン名リストから重複を避けるために除外します。
	 */
	static void SetLoadingScene(const std::string& name) {
		loadingSceneName = name;
		/*for (auto it = sceneNames.begin(); it != sceneNames.end(); it++) {
			if (it->c_str() == loadingSceneName) {
				sceneNames.erase(it);
				break;
			}
		}*/
	}

	/**
	 * @brief シーンを即時に切り替えます（強制）。
	 * @param nextSceneName 次のシーン名。
	 */
	static void ChangeScene(const std::string& nextSceneName);

	/**
	 * @brief シーン変更を要求します（ロード完了までローディングシーンを表示）。
	 * @param nextSceneName ロード完了後に遷移するシーン名。
	 */
	static void LoadScene(const std::string& nextSceneName);

	/**
	 * @brief シーンを事前に非同期ロードします。
	 * @param loadSceneName 事前ロードするシーン名。
	 */
	static void LoadSceneAsync(const std::string& loadSceneName);

	/**
	 * @brief 次のシーンへの遷移を許可します。
	 */
	static void AllowTransition();

	/**
	 * @brief 非同期ロードが完了したかを返します。
	 */
	static bool IsLoadingComplete();

	/**
	 * @brief シーンの遷移中かどうかを返します。
	 */
	static bool IsTransition();

	/**
	 * @brief 現在のシーンを取得します。
	 * @return 現在のシーンポインタ。
	 */
	static inline Scene* GetCurrentScene() { return currentScene.get(); }

	/**
	 * @brief ロード中のシーンが存在しない場合は現在のシーンを返します。
	 * @return ロード中のシーンまたは現在のシーンのポインタ。
	 */
	static inline Scene* GetLoadingSceneOrCurrentScene() {
		if (nextScene) return nextScene.get();
		if (currentScene) return currentScene.get();
		return nullptr;
	}

	/**
	 * @brief 現在アクティブなシーン（ロード中シーン含む）を取得します。
	 * @return アクティブシーンの配列。
	 */
	static std::vector<Scene*> GetActiveScenes();

	/** @brief 編集モードに入る処理。*/
	static void EnterEdit();

	/** @brief プレイモードに入る処理。*/
	static void EnterPlay();

	/** @brief ポーズモードに入る処理。*/
	static void EnterPause();

	/** @brief プレイモードを再開する処理。*/
	static void ResumePlay();

	/**
	 * @brief シーンを登録します。
	 * @param path 登録するシーンのファイルパス。
	 */
	static void Register(const std::string& path);

	/* シーンマネージャの状態をシリアライズします。*/
	static json Serialize();

	/* シーンマネージャの状態をデシリアライズします。*/
	static void Deserialize(const json& j);

private:
	/** @brief ランタイム用シーンファイルをクリーンアップします。*/
	static void CleanRuntimeFiles();
};