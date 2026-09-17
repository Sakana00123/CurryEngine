#include "pch.h"
#include "SceneManager.h"
#include "Scene.h"
#ifdef USE_IMGUI
#include <imgui.h>

#include "Engine/Editor/SceneViewWindow.h"
#include "Engine/Editor/GameViewWindow.h"

#endif // USE_IMGUI
#include "Engine/Events/EventSystem.h"
#include "Engine/Editor/EditorGUI.h"
#include "Engine/Editor/AssetBrowser.h"
#include "Engine/Editor/HlslEditor.h"
#include "Engine/Editor/Console.h"
#include "Engine/Editor/SceneParametersEditor.h"
#include "Engine/Editor/BuildSettingsWindow.h"
#include "Engine/Effects/EffectManager.h"

#include "Engine/Audio/BeatManager.h"
#include "Engine/Physics/Physics.h"
#include "Engine/Rendering/Camera/CameraSystem.h"
#include <Engine\EditorConfig\EditorConfigManager.h>

void SceneManager::Initialize()
{
	// ランタイム用シーンファイルを削除
	CleanRuntimeFiles();

	// デシリアライズ
	json settingsJson;
#ifdef _DEBUG
	std::string settingsPath = "./ProjectSettings/settings.json";
#else
	std::string settingsPath = "./Settings/settings.bin";
#endif // _DEBUG

	if (JsonFileHandler::LoadJsonFromFile(settingsJson, settingsPath))
	{
		Deserialize(settingsJson);
	}
	else
	{
		LOG_ERROR("Failed to load settings.");
	}
}

void SceneManager::BeginFrame()
{
	if (currentScene) currentScene->BeginFrame();
}

void SceneManager::EndFrame()
{
	if (currentScene) currentScene->EndFrame();
}

void SceneManager::Finalize()
{
	// シーンマネージャの状態をシリアライズして保存
#ifdef _DEBUG
	SaveSettings();
#endif // _DEBUG


	// 現在のシーンを終了
	if (currentScene) currentScene->Finalize();
	currentScene.reset();

	// ランタイム用シーンファイルを削除
	CleanRuntimeFiles();
}

void SceneManager::SaveSettings()
{
	// シーンマネージャの状態をシリアライズして保存
	json settingsJson = Serialize();
	JsonFileHandler::SaveJsonToFile(settingsJson, "./ProjectSettings/settings.json");
	JsonFileHandler::SaveJsonToFile(settingsJson, "./Settings/settings.bin");
}

void SceneManager::SetFirstScene(const std::string& name)
{
	// 最初のシーンが配列の最初に来るようにする
	if (auto it = std::find_if(SceneManager::sceneEntries.begin(), SceneManager::sceneEntries.end(),
		[&](const SceneManager::SceneEntry& entry) { return entry.name == name; }); it != SceneManager::sceneEntries.end())
	{
		std::iter_swap(SceneManager::sceneEntries.begin(), it);
	}
	UpdateFirstSceneName();
}


void SceneManager::UpdateFirstSceneName()
{
	// シーンエントリの最初の要素を最初のシーン名として設定
	if (!sceneEntries.empty())
	{
		firstSceneName = sceneEntries.front().name;
	}
	else
	{
		firstSceneName.clear();
	}
}

void SceneManager::LoadFirstScene()
{
	std::string sceneToLoad = state == State::Editing ? EditorConfigManager::GetLastOpenedScene() : firstSceneName;
	if (!sceneToLoad.empty())
	{
#ifdef _DEBUG
		ChangeScene(sceneToLoad);
#else
		LoadScene(sceneToLoad);
#endif // _DEBUG

	}
	else
	{
		LOG_ERROR("First scene name is not set.");
	}
}

void SceneManager::Update(float deltaTime)
{
	// 非同期ロードが開始されておらず、かつロード待ちシーンが存在するなら非同期ロードを開始
	if (!future.valid() && !sceneLoadQueue.empty())
	{
		// 非同期でシーンの初期化を開始
		nextScene = std::move(sceneLoadQueue.front());
		sceneLoadQueue.pop();
		future = std::async(std::launch::async, &Scene::Initialize, nextScene.get());
	}
	// 非同期ロードが完了しており、かつ次のシーンが存在し、遷移可能ならシーンを切り替え
	if (nextScene != nullptr && IsLoadingComplete() && nextScene->canTransition)
	{
		if (currentScene) {
			currentScene->Finalize();
			currentScene.reset();

			// EventSystemの状態をリセットして、遷移中の入力を無効化
			EventSystem::GetCurrent()->Reset();

			// エフェクトを全停止
			EffectManager::StopAll();

			// 物理エンジンのクリーンアップ
			Physics::Clean();
		}
		future.get();
		currentScene = std::move(nextScene);

		// 選択中オブジェクトを解除
		EventSystem::GetCurrent()->SetSelectedGameObject(nullptr);

		// カメラシステムへ通知
		currentScene->cameraSystem.ResolveMainCamera();

		// シーン開始処理
		currentScene->Start();

		// ビートマネージャ初期化
		BeatManager::Initialize();

		if (!pendingSceneName.empty())
		{
			LoadSceneAsync(pendingSceneName);
			pendingSceneName.clear();
		}
	}

	// シーン遷移中は更新処理をスキップ
	/*if (IsTransition())
	{
		LOG_WARNING("Scene transition in progress. Update skipped.");
		return;
	}*/

	// 現在のシーンを更新
	if (currentScene != nullptr) {
		switch (state)
		{
		case State::Editing:
		{
			// エディタ用更新
			currentScene->Update(0.0f);
			break;
		};
		case State::Paused:
		{
			// ポーズ中更新
			break;
		};
		case State::Playing:
		{
			// ゲーム実行中更新
			currentScene->Update(deltaTime);
			// ビートマネージャ更新
			BeatManager::Update(deltaTime);
			break;
		};
		default:
			break;
		}
	}
}

void SceneManager::BeginRendering(RenderContext* rtx)
{
	if (currentScene != nullptr)
	{
		currentScene->BeginRendering(rtx);
	}
}

void SceneManager::Render(RenderContext* rtx)
{
	if (currentScene != nullptr)
	{
		currentScene->Render(rtx);
	}
}

void SceneManager::EndRendering(RenderContext* rtx)
{
	if (currentScene != nullptr)
	{
		currentScene->EndRendering(rtx);
	}
}

void SceneManager::Draw(RenderContext* rtx)
{
	if (currentScene != nullptr)
	{
		currentScene->Draw(rtx);
	}
}

void SceneManager::DrawGUI(RenderContext* sceneRtx, RenderContext* gameRtx)
{
#ifdef USE_IMGUI
	
	// メニューバー
	float mainMenuBarHeight = EditorGUI::DrawMainMenu();

	// ツールバー
	float toolBarHeight = EditorGUI::DrawToolbar(mainMenuBarHeight);

	// Docking設定
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	float dockY = viewport->Pos.y + mainMenuBarHeight + toolBarHeight;
	float dockHeight = viewport->Size.y - mainMenuBarHeight - toolBarHeight;

	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, dockY));
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, dockHeight));
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoNavFocus;
	
	// -------------------- Docking host --------------------
	{
		ImGui::Begin("MainDockspace", nullptr, window_flags);

		ImGui::PopStyleVar(3);

		// Dockspace本体
		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

		ImGui::End();
	}

	// -------------------- Scene View Window --------------------
	{
		CurryEngine::SceneViewWindow::Get().Draw(sceneRtx, currentScene.get());
	}

	// -------------------- Game View Window --------------------
	{
		CurryEngine::GameViewWindow::Get().Draw(gameRtx, currentScene.get());
	}

	// ------------------- 汎用ウィンドウ --------------------
	SceneParametersEditor::DrawGUI();

	// ------------------- アセットブラウザ --------------------
	AssetBrowser::DrawGUI();

	// ------------------- HLSLエディタ ---------------------
	//HlslEditor::DrawGUI();

	// ------------------- Consoleウィンドウ ---------------------
	Console::DrawGUI();

	// -------------------- 他ウィンドウ --------------------

	BuildSettingsWindow::Get().DrawGUI();


	if (currentScene != nullptr) {
		currentScene->objectManager->DrawHierarchy();
		currentScene->objectManager->DrawProperty();
	}
#endif // USE_IMGUI
}

void SceneManager::ChangeScene(const std::string& name)
{
	std::unique_ptr<Scene> loadScene = std::make_unique<Scene>();
	loadScene->name = name;
	loadScene->canTransition = true;
	sceneLoadQueue.push(std::move(loadScene));
}

void SceneManager::LoadScene(const std::string& name)
{
	//_ASSERT_EXPR(reflections().find(name) != reflections().end(), L"指定したシーン名は存在しません。");
	std::string loadSceneName = state == State::Playing ? loadingSceneName : editorLoadingSceneName;

	pendingSceneName = name;
	ChangeScene(loadSceneName);
}

void SceneManager::LoadSceneAsync(const std::string& loadSceneName)
{
	//_ASSERT_EXPR(reflections().find(loadSceneName) != reflections().end(), L"指定したシーン名は存在しません。");
	//std::unique_ptr<Scene> loadScene = reflections().at(loadSceneName)();
	std::unique_ptr<Scene> loadScene = std::make_unique<Scene>();
	loadScene->name = loadSceneName;
	loadScene->canTransition = false;
	sceneLoadQueue.push(std::move(loadScene));
}

void SceneManager::AllowTransition()
{
	if (nextScene)
	{
		nextScene->canTransition = true;
	}
}

bool SceneManager::IsLoadingComplete()
{
	if (!future.valid()) return false;
	return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

bool SceneManager::IsTransition() 
{
	if (!future.valid()) return false;
	return future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
}

std::vector<Scene*> SceneManager::GetActiveScenes()
{
	std::vector<Scene*> scenes;
	if (currentScene) scenes.push_back(currentScene.get());
	if (nextScene) scenes.push_back(nextScene.get());
	return scenes;
}


void SceneManager::EnterEdit()
{
	ZoneScopedN("SceneManager::EnterEdit");
	// 現在のシーンが存在しなければ何もしない。
	if (currentScene == nullptr) return;

	// 現在の状態を確認し、エディットモードへ移行する。
	if (state == State::Playing)
	{
		state = State::PlayToEdit;
	}
	else
	{
		// 既にエディットモードの場合は何もしない。
		return;
	}

	// 前回のシーンデータが存在しなければ何もしない。(初回の編集モード移行時など)
	if (previousData.sceneName.empty()) return;

	// ランタイム用シーンファイルを削除する。
	CleanRuntimeFiles();

	// シーン名を元に戻す。
	currentScene->name = previousData.sceneName;

	// シーンを読み込み直す。
	ChangeScene(currentScene->name);
}

void SceneManager::EnterPlay()
{
	ZoneScopedN("SceneManager::EnterPlay");
	// 現在のシーンが存在しなければ何もしない。
	if (currentScene == nullptr) return;

	// 現在の状態を確認し、プレイモードへ移行する。
	if (state == State::Editing)
	{
		state = State::EditToPlay;
	}
	else
	{
		// 既にプレイモードの場合は何もしない。
		return;
	}

	// 前回のシーンデータをクリア
	previousData = SceneData{};

	// 現在の状態を保存しておく。
	previousData.sceneName = currentScene->name; // シーン名を保存
	currentScene->Serialize(previousData.sceneJson);

	// シーンをランタイム用ファイルとして保存する。
	JsonFileHandler::SaveJsonToFile(previousData.sceneJson, "./Assets/Scenes/" + currentScene->name + runtimeSuffix + ".scene", JsonIOFormat::Text, FileAttribute::Hidden);

	// シーンを読み込み直す。(ランタイム用シーンとして)
	ChangeScene(currentScene->name);
}

void SceneManager::EnterPause()
{
	state = State::Paused;
}

void SceneManager::ResumePlay()
{
	state = State::Playing;
}

void SceneManager::Register(const std::string& path)
{
	std::string name = fs::path(path).stem().string();
	sceneNames.push_back(name);
	SceneEntry entry;
	entry.enabled = true;
	entry.name = name;
	entry.path = path;
	sceneEntries.push_back(entry);
	UpdateFirstSceneName();
}

json SceneManager::Serialize()
{
	ZoneScopedN("SceneManager::Serialize");
	json j;
	// エディタで、最初に開くシーンを保存する。
	std::string editorFirstSceneName;
	if (state == State::Editing) {
		editorFirstSceneName = currentScene ? currentScene->name : "";
	}
	else if (state == State::Playing) {
		editorFirstSceneName = previousData.sceneName;
	}
	//j["firstScene"] = editorFirstSceneName;
	EditorConfigManager::SetLastOpenedScene(editorFirstSceneName);

	// ビルド設定のシーンリストを保存する。
	json scenesInBuild;
	{
		// ランタイムで、最初に開くシーンを保存する。
		scenesInBuild["firstScene"] = firstSceneName;

		json sceneListJson = json::array();
		int index = 0;
		for (const auto& entry : sceneEntries)
		{
			json sceneEntry;
			sceneEntry["enabled"] = entry.enabled;
			sceneEntry["name"] = entry.name;
			sceneEntry["path"] = entry.path;
			sceneEntry["index"] = index++;
			sceneListJson.push_back(sceneEntry);
		}
		json loadingSceneEntry;
		loadingSceneEntry["name"] = loadingSceneName;
		scenesInBuild["loadingScene"] = loadingSceneEntry;

		scenesInBuild["scenes"] = sceneListJson;
	}
	j["scenesInBuild"] = scenesInBuild;

	return j;
}

void SceneManager::Deserialize(const json& j)
{
	ZoneScopedN("SceneManager::Deserialize");
	// ビルド設定のシーンリストを読み込む。
	{
		json scenesInBuild = j.value("scenesInBuild", json::object());
		// ランタイムで、最初に開くシーンを読み込む。
		firstSceneName = scenesInBuild.value("firstScene", "");
		loadingSceneName = scenesInBuild.value("loadingScene", json::object()).value("name", "");
		json sceneListJson = scenesInBuild.value("scenes", json::array());
		for (const auto& sceneEntry : sceneListJson)
		{
			SceneEntry entry;
			entry.enabled = sceneEntry.value("enabled", true);
			entry.name = sceneEntry.value("name", "");
			entry.path = sceneEntry.value("path", "");
			sceneEntries.push_back(entry);
		}

	}

#ifdef _DEBUG
	// エディタで、最初に開くシーンを読み込む。
	//editorFirstSceneName = j.value("firstScene", "");
	//SetEditorFirstScene(editorFirstSceneName);
#else
	SetFirstScene(firstSceneName);
#endif // _DEBUG

}

void SceneManager::CleanRuntimeFiles()
{
	ZoneScopedN("SceneManager::CleanRuntimeFiles");
	// ランタイム用シーンファイルを削除する。
	for (auto& entry : fs::directory_iterator("./Assets/Scenes/")) {
		std::string stem = entry.path().stem().string();
		if (stem.size() < runtimeSuffix.size()) continue; // 接尾辞より短いファイル名はスキップ
		if (stem.substr(stem.size() - runtimeSuffix.size(), std::string::npos) == runtimeSuffix) {
			fs::remove(entry.path());
		}
	}
}
