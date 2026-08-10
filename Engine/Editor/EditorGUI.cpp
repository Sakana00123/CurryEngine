#include "pch.h"
#include "EditorGUI.h"

#include <ranges>

#include "Engine/Scenes/Scene.h"
#include "Engine/Core/GameObject.h"
#include "Engine/UI/Canvas.h"
#include "Dialog.h"
#include "Engine/Rendering/Renderers/GltfModelRenderer.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Rendering/Camera/CameraComponent.h"
#include "AssetBrowser.h"
#include "HlslEditor.h"
#include "Console.h"
#include "SceneParametersEditor.h"
#include "FileOpener.h"
#include "EffectEditor.h"
#include "Engine/Utils/JsonFileHandler.h"
#include "ImGuiTheme.h"
#include "Engine/Scripting/ScriptSystem.h"

#include "Engine/Core/ObjectManager.h"
#include "Engine/Factory/GameObjectFactory.h"

#include "Engine/Resources/ResourceManager.h"
#include "Engine/Resources/Texture.h"
#include "Engine/EditorSupport/EditorSelection.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

#include "BuildSettingsWindow.h"
#include <Engine\EditorConfig\EditorConfigManager.h>
#include "Engine/EditorConfig/SceneViewconfig.h"

float EditorGUI::DrawMainMenu()
{
	float mainMenuBarHeight = 0.0f;
#ifdef USE_IMGUI
	//上のメニューバー
	if (ImGui::BeginMainMenuBar())
	{
		// メインメニューバーの高さを取得
		mainMenuBarHeight = ImGui::GetWindowHeight();
#if 1
		// ------------- ファイルメニュー ---------------
		if (ImGui::BeginMenu("File"))
		{
			DrawFileMenu();
			ImGui::EndMenu();
		}
#endif
		// ------------- シーンメニュー ---------------
		if (ImGui::BeginMenu("Scene"))
		{
			DrawSceneMenu();
			ImGui::EndMenu();
		}

		// -------------- 生成メニュー ---------------
		if (ImGui::BeginMenu("GameObject"))
		{
			DrawGameObjectMenu();
			ImGui::EndMenu();
		}
		// ------------ ウィンドウ -----------
		if (ImGui::BeginMenu("Window"))
		{
			DrawWindowMenu();
		}

		ImGui::EndMainMenuBar();
	}
	
	// Fileメニューのショートカットキー処理
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_N)) {
		CreateNewScene();
	}
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_O)) {
		OpenScene();
	}
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)) {
		SaveScene();
	}
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S | ImGuiMod_Shift)) {
		SaveSceneAs();
	}

#endif // USE_IMGUI
	return mainMenuBarHeight;
}

float EditorGUI::DrawToolbar(float offsetY)
{
	// ツールバー
	float toolbarHeight = 40.0f;
#ifdef USE_IMGUI
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 pos = ImVec2(viewport->Pos.x, viewport->Pos.y + offsetY);
	ImVec2 size = ImVec2(viewport->Size.x, toolbarHeight);

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::Begin("##Toolbar", nullptr, flags);

	// ボタンの合計幅を計算
	float buttonSize = 22.0f;
	float padding = 8.0f;
	float startY = (toolbarHeight - buttonSize) * 0.5f - 4.0f;
	int buttonCount = 1;
	float totalWidth = (static_cast<float>(buttonCount) * buttonSize) + 
		static_cast<float>(buttonCount - 1) * padding;
	// ウィンドウの中央に配置
	float centerX = ImGui::GetWindowWidth() * 0.5f;
	ImGui::SetCursorPosX(centerX - totalWidth * 0.5f);
	ImGui::SetCursorPosY(startY);

	bool isPlaying = (SceneManager::state == SceneManager::State::Playing);

	auto iconTex = ResourceManager::GetOrLoad<AssetTexture>("./Data/Icon/editorIcons.png");
	ImVec2 playIconUV0 = ImVec2(0.0f, 0.0f);
	ImVec2 playIconUV1 = ImVec2(0.25f, 0.25f);
	ImVec2 stopIconUV0 = ImVec2(0.25f, 0.0f);
	ImVec2 stopIconUV1 = ImVec2(0.5f, 0.25f);
	ImVec4 backGroundColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	bool isTransitioning = SceneManager::IsTransition();
	if (isTransitioning)
	{
		// トランジション中はボタンを無効化
		ImGui::BeginDisabled();
	}

	if (!isPlaying) {

		if (ImGui::ImageButton("##play", iconTex->GetSRV(), ImVec2(buttonSize, buttonSize), playIconUV0, playIconUV1, backGroundColor, tintColor))
		{
			// プレイ処理
			SceneManager::EnterPlay();
		}
	}
	else {
		if (ImGui::ImageButton("##stop", iconTex->GetSRV(), ImVec2(buttonSize, buttonSize), stopIconUV0, stopIconUV1, backGroundColor, tintColor))
		{
			// ストップ処理
			SceneManager::EnterEdit();
		}
	}

	// バーの左端にリロードボタンを配置
	float reloadButtonX = 10.0f;
	ImGui::SetCursorPosX(reloadButtonX);
	ImGui::SetCursorPosY(startY);
	if (ImGui::Button("Build Scripts"))
	{
		// リロード処理
		ScriptSystem::RequestScriptBuildAndReload();
	}

	// 右端にリロードボタンを配置
	ImGui::SameLine();
	if (ImGui::Button("HotReload"))
	{
		// リロード処理
		ResourceManager::UpdateHotReload();
	}

	if (isTransitioning)
	{
		ImGui::EndDisabled();
	}


	ImGui::End();
#endif // USE_IMGUI
	return toolbarHeight;
}

float EditorGUI::DrawSceneViewToolbar()
{
	float toolbarHeight = 30.0f;
#ifdef USE_IMGUI
	// ツールバーの内容をここに描画
	ImGui::BeginChild("SceneToolbar", ImVec2(0, toolbarHeight), false);
	{
		// ここにシーンビューのツールバーの内容を描画
		// ギズモのLocal/World切り替えや、スナップ設定などを配置
		SceneViewConfig* config = EditorConfigManager::GetSceneViewConfig();
		if (!config) {
			ImGui::EndChild();
			return toolbarHeight;
		}

		auto iconTex = ResourceManager::GetOrLoad<AssetTexture>("./Data/Icon/editorIcons.png");
		float buttonSize = 22.0f;
		ImVec2 snapIconUV0 = ImVec2(0.5f, 0.0f);
		ImVec2 snapIconUV1 = ImVec2(0.75f, 0.25f);

		ImVec2 linkIconUV0 = ImVec2(0.75f, 0.0f);
		ImVec2 linkIconUV1 = ImVec2(1.0f, 0.25f);

		ImVec2 unLinkIconUV0 = ImVec2(0.0f, 0.25f);
		ImVec2 unLinkIconUV1 = ImVec2(0.25f, 0.5f);

		ImVec4 backGroundColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		// ピボットモードの切り替え
		ImGui::SetNextItemWidth(75.0f); // コンボボックスの幅を指定
		if (ImGui::Combo("##Pivot", &config->guizmoPivotMode, "Local\0World\0")) {

		}
		ImGui::SameLine();

		// スナップ設定ボタン
		if (ImGui::ImageButton("##snap", iconTex->GetSRV(), ImVec2(buttonSize, buttonSize), snapIconUV0, snapIconUV1, backGroundColor, tintColor)) {
			ImGui::OpenPopup("SnapSettingsPopup");
		}
		ImGui::SetNextWindowSize(ImVec2(350, 0));
		if (ImGui::BeginPopup("SnapSettingsPopup")) {
			ImGui::Text("Snap Settings");

			// ポップアップの右上に詳細設定ボタンを配置
			{
				float padding = ImGui::GetStyle().WindowPadding.x;
				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - buttonSize - padding);
				ImGui::SetCursorPosY(padding);
				if (ImGui::Button("..."))
				{
					ImGui::OpenPopup("SnapSettingsDetailPopup");
				}
				if (ImGui::BeginPopup("SnapSettingsDetailPopup"))
				{
					// 詳細設定の内容をここに描画
					if (ImGui::MenuItem("Reset"))
					{
						config->ResetToDefault();
					}

					ImGui::EndPopup();
				}
			}

			ImGui::Separator();

			// 移動スナップ設定
			{
				ImGui::Checkbox("Enable Translation Snap", &config->translationSnap.enabled);
				ImGui::BeginDisabled(!config->translationSnap.enabled);
				IMGUI_PROPERTY_BEGIN();
				IMGUI_PROPERTY("Translation");

				if (ImGui::ImageButton("##linkTranslationSnap", iconTex->GetSRV(), ImVec2(16, 16),
					config->translationSnap.snapAllAxes ? linkIconUV0 : unLinkIconUV0,
					config->translationSnap.snapAllAxes ? linkIconUV1 : unLinkIconUV1,
					backGroundColor, tintColor))
				{
					config->translationSnap.snapAllAxes = !config->translationSnap.snapAllAxes;
				}
				ImGui::SameLine();
				if (config->translationSnap.snapAllAxes)
				{
					ImGui::DragFloat("##Translation Snap", &config->translationSnap.snapValue.x, 0.1f, 0.0f, FLT_MAX);
					config->translationSnap.snapValue.y = config->translationSnap.snapValue.x;
					config->translationSnap.snapValue.z = config->translationSnap.snapValue.x;
				}
				else
				{
					ImGui::DragFloat3("##Translation Snap", &config->translationSnap.snapValue.x, 0.1f, 0.0f, FLT_MAX);
				}
				IMGUI_PROPERTY_END();
				ImGui::EndDisabled();
			}

			// 回転スナップ設定
			{
				ImGui::Checkbox("Enable Rotation Snap", &config->rotationSnap.enabled);
				ImGui::BeginDisabled(!config->rotationSnap.enabled);
				IMGUI_PROPERTY_BEGIN();
				IMGUI_PROPERTY("Rotation");
				if (ImGui::ImageButton("##linkRotationSnap", iconTex->GetSRV(), ImVec2(16, 16),
					config->rotationSnap.snapAllAxes ? linkIconUV0 :
					unLinkIconUV0,
					config->rotationSnap.snapAllAxes ? linkIconUV1 :
					unLinkIconUV1,
					backGroundColor, tintColor))
				{
					config->rotationSnap.snapAllAxes = !config->rotationSnap.snapAllAxes;
				}
				ImGui::SameLine();
				if (config->rotationSnap.snapAllAxes)
				{
					ImGui::DragFloat("##Rotation Snap", &config->rotationSnap.snapValue.x, 0.1f, 0.0f, FLT_MAX);
					config->rotationSnap.snapValue.y = config->rotationSnap.snapValue.x;
					config->rotationSnap.snapValue.z = config->rotationSnap.snapValue.x;
				}
				else
				{
					ImGui::DragFloat3("##Rotation Snap", &config->rotationSnap.snapValue.x, 0.1f, 0.0f, FLT_MAX);
				}
				IMGUI_PROPERTY_END();
				ImGui::EndDisabled();
			}

			// スケールスナップ設定
			{
				ImGui::Checkbox("Enable Scale Snap", &config->scaleSnap.enabled);
				ImGui::BeginDisabled(!config->scaleSnap.enabled);
				IMGUI_PROPERTY_BEGIN();
				IMGUI_PROPERTY("Scale");
				if (ImGui::ImageButton("##linkScaleSnap", iconTex->GetSRV(), ImVec2(16, 16),
					config->scaleSnap.snapAllAxes ? linkIconUV0 :
					unLinkIconUV0,
					config->scaleSnap.snapAllAxes ? linkIconUV1 :
					unLinkIconUV1,
					backGroundColor, tintColor))
				{
					config->scaleSnap.snapAllAxes = !config->scaleSnap.snapAllAxes;
				}
				ImGui::SameLine();
				if (config->scaleSnap.snapAllAxes)
				{
					ImGui::DragFloat("##Scale Snap", &config->scaleSnap.snapValue.x, 0.1f, 0.0f, FLT_MAX);
					config->scaleSnap.snapValue.y = config->scaleSnap.snapValue.x;
					config->scaleSnap.snapValue.z = config->scaleSnap.snapValue.x;
				}
				else
				{
					ImGui::DragFloat3("##Scale Snap", &config->scaleSnap.snapValue.x, 0.1f, 0.0f, FLT_MAX);
				}
				IMGUI_PROPERTY_END();
				ImGui::EndDisabled();
			}

			ImGui::EndPopup();
		}

		
	}
	ImGui::EndChild();
#endif // USE_IMGUI
	return toolbarHeight;
}

void EditorGUI::DrawFileMenu()
{
#ifdef USE_IMGUI
	// ------------- ファイルメニュー ---------------
	// 新規シーン
	if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
		// 新規シーン作成
		CreateNewScene();
	}
	// シーンの読み込み・保存
	if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
		// シーン読み込み
		OpenScene();
	}
	if (ImGui::MenuItem("Save", "Ctrl+S")) {
		// シーン保存
		SaveScene();
	}
	if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
		// シーン保存
		SaveSceneAs();
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Build Settings")) {
		BuildSettingsWindow::Get().Show();
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Exit")) {
		// アプリケーション終了
		PostQuitMessage(0);
	}


#endif // USE_IMGUI
}

void EditorGUI::DrawSceneMenu()
{
#ifdef USE_IMGUI
	//シーン変更ボタン

	std::filesystem::directory_iterator directory("./Assets/Scenes");

	// シーン名のリストから特定のシーンを除外するためのセット
	static const std::vector<std::string> ignoredScenes = { 
		"EmptyScene",
		"DefaultScene"
	};

	for (auto& entry : directory)
	{
		if (entry.path().extension() != ".scene") {
			continue; // sceneファイル以外はスキップ
		}
		std::string name = entry.path().stem().string();
		// シーン名が除外リストに含まれている場合はスキップ
		if (std::ranges::find(ignoredScenes, name) != ignoredScenes.end()) {
			continue;
		}
		// メニューアイテムを作成し、クリックされたらシーンを切り替える
		if (ImGui::MenuItem(name.c_str())) {
			SceneManager::ChangeScene(name);
		}
	}
#endif // USE_IMGUI
}

void EditorGUI::DrawGameObjectMenu()
{
#ifdef USE_IMGUI
	Scene* scene = SceneManager::GetCurrentScene();
	if (!scene)	return;
	GameObject* createdObj = nullptr;

	if (ImGui::MenuItem("CreateEmpty")) {
		createdObj = GameObjectFactory::Create(scene, "GameObject");
	}
	auto selection = scene->GetObjectManager()->GetEditorSelection();
	bool hasCanvas = false;
	GameObject* selectedObj = selection->GetPrimarySelected().get();
	Canvas* selectedCanvas = nullptr;
	for (const auto& obj : selection->GetAll()) {
		if (selectedCanvas = obj->GetComponent<Canvas>()) {
			break;
		}
	}
	for (const auto& obj : selection->GetAll()) {
		if (obj->GetComponentInParent<Canvas>()) {
			hasCanvas = true;
			break;
		}
	}


	if (ImGui::BeginMenu("UI")) {
		if (ImGui::MenuItem("Empty2D")) {
			createdObj = GameObjectFactory::CreateUIObject(scene, "GameObject2D", selectedCanvas ? selectedCanvas->GetOwner() : nullptr);
		}
		if (ImGui::MenuItem("Button")) {
			createdObj = GameObjectFactory::CreateButton(scene, "Button", selectedCanvas ? selectedCanvas->GetOwner() : nullptr,
				L"./Data/Default/button.png");
			// ボタンの子にテキストを作成
			GameObjectFactory::CreateText(scene, "Text", createdObj);
		}
		if (ImGui::MenuItem("Text")) {
			createdObj = GameObjectFactory::CreateText(scene, "Text", selectedCanvas ? selectedCanvas->GetOwner() : nullptr);
		}
		if (ImGui::MenuItem("Image")) {
			createdObj = GameObjectFactory::CreateImage(scene, "Image", selectedCanvas ? selectedCanvas->GetOwner() : nullptr);
		}
		if (ImGui::MenuItem("InputField")) {
			createdObj = GameObjectFactory::CreateInputField(scene, "InputField", selectedCanvas ? selectedCanvas->GetOwner() : nullptr);
		}
		//if (ImGui::MenuItem("Slider")) {
		//	createdObj = GameObjectFactory::CreateSlider(scene, "Slider", selectedCanvas ? selectedCanvas->GetOwner() : nullptr);
		//}
		if (ImGui::MenuItem("ScrollView")) {
			createdObj = GameObjectFactory::CreateScrollView(scene, "ScrollView", selectedCanvas ? selectedCanvas->GetOwner() : nullptr);
		}
		if (ImGui::MenuItem("Toggle")) {
			createdObj = GameObjectFactory::CreateToggle(scene, "Toggle", selectedCanvas ? selectedCanvas->GetOwner() : nullptr);
		}
		if (ImGui::MenuItem("Canvas")) {
			createdObj = GameObjectFactory::CreateCanvas(scene, "Canvas");
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("3D")) {
		if (ImGui::MenuItem("Cube")) {
			createdObj = GameObjectFactory::CreateCube(scene, "Cube");
		}
		if (ImGui::MenuItem("Sphere")) {
			createdObj = GameObjectFactory::CreateSphere(scene, "Sphere");
		}
		if (ImGui::MenuItem("Cylinder")) {
			createdObj = GameObjectFactory::CreateCylinder(scene, "Cylinder");
		}
		if (ImGui::MenuItem("GltfModel")) {
			static const char* filter = "Model Files(*.gltf;*.glb;*.cereal\0*.gltf;*.glb;*.cereal\0All Files(*.*)\0*.*;\0\0)";

			char filePath[256] = { 0 };
			HWND hwnd = Graphics::GetHwnd();
			DialogResult result = Dialog::OpenFileName(filePath, sizeof(filePath), filter, nullptr, hwnd);
			if (result == DialogResult::OK) {
				createdObj = GameObjectFactory::Create(scene, "GltfModel");
				createdObj->AddComponent<GltfModelRenderer>()->LoadModel(Graphics::GetDevice(), std::string(filePath), false);
			}
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Camera"))
	{
		if (ImGui::MenuItem("Camera"))
		{
			createdObj = GameObjectFactory::CreateCamera(scene, "Camera");
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Light"))
	{
		if (ImGui::MenuItem("Directional Light"))
		{
			createdObj = GameObjectFactory::CreateDirectionalLight(scene, "Directional Light");
		}
		if (ImGui::MenuItem("Point Light"))
		{
			createdObj = GameObjectFactory::CreatePointLight(scene, "Point Light");
		}
		if (ImGui::MenuItem("Spot Light"))
		{
			createdObj = GameObjectFactory::CreateSpotLight(scene, "Spot Light");
		}
		ImGui::EndMenu();
	}

	if (createdObj)
	{
		// UIオブジェクトの場合は、選択中のオブジェクトがCanvasの子であればその子にする。そうでなければ、選択中のオブジェクトがCanvasを持っていればそのCanvasの子にする。どちらも当てはまらない場合はルートに配置される。
		if (createdObj->GetComponent<RectTransform>() && hasCanvas)
		{
			createdObj->SetParent(selectedObj);
		}

		// HierarchyとInspectorを更新
		createdObj->RefreshActiveInHierarchy();
	}

#endif // USE_IMGUI
}

void EditorGUI::DrawWindowMenu()
{
#ifdef USE_IMGUI
	if (ImGui::MenuItem("AssetBrowser"))
	{
		AssetBrowser::Show();
	}
	//if (ImGui::MenuItem("HLSL Editor"))
	//{
	//	HlslEditor::Show();
	//}
	if (ImGui::MenuItem("Console"))
	{
		Console::Show();
	}
	if (ImGui::MenuItem("SceneParametersEditor"))
	{
		SceneParametersEditor::Show();
	}
	if (ImGui::MenuItem("Profiler"))
	{
		// tracy-profilerを起動する
		std::filesystem::path tracyPath = EnginePaths::TracyProfilerExe;
		tracyPath = std::filesystem::absolute(tracyPath);
		if (std::filesystem::exists(tracyPath)) {
			OpenFileWithDefaultApplication(tracyPath.wstring());
		}
		else {
			LOG_ERROR("Tracy profiler executable not found: " + tracyPath.string());
		}
	}
	if (ImGui::MenuItem("Effect Editor"))
	{
		EffectEditor::Show();
	}
	if (ImGui::MenuItem("ImGui Theme"))
	{
		ImGuiTheme::Show();
	}
	ImGui::EndMenu();
#endif // USE_IMGUI
}

void EditorGUI::CreateNewScene()
{
	// 新規シーン作成のためのモーダルを表示
	AssetBrowser::ShowNewSceneCreationModal("./Assets/Scenes");
}

void EditorGUI::OpenScene()
{
	// シーン読み込み
	static const char* filter = "Scene Files(*.scene)\0*.scene\0All Files(*.*)\0*.*;\0\0";
	char filePath[256] = { 0 };
	HWND hwnd = Graphics::GetHwnd();
	DialogResult result = Dialog::OpenFileName(filePath, sizeof(filePath), filter, nullptr, hwnd);
	if (result == DialogResult::OK) {
		std::filesystem::path path(filePath);
		// シーンをJSONファイルから読み込み
		SceneManager::ChangeScene(path.stem().string());
	}
}

void EditorGUI::SaveScene()
{
	// シーン保存
	Scene* scene = SceneManager::GetCurrentScene();
	if (scene) {
		std::filesystem::path sceneFilePath = "./Assets/Scenes/" + scene->name + ".scene";
		// シーンをJSON化
		json j;
		scene->Serialize(j);
		// JSONファイルに保存
		JsonFileHandler::SaveJsonToFile(j, sceneFilePath.string());
		// 保存完了のログを出力
		LOG_INFO("Scene saved: " + sceneFilePath.string());

		// バイナリファイルとしても保存
		{
			sceneFilePath.replace_extension(".bin");
			JsonFileHandler::SaveJsonToFile(j, sceneFilePath.string(), JsonIOFormat::Binary);
		}
	}
}

void EditorGUI::SaveSceneAs()
{
	// シーン保存
	Scene* scene = SceneManager::GetCurrentScene();
	if (scene) {
		// ファイルダイアログ表示
		const char* filter = "Scene Files(*.scene)\0*.scene\0All Files(*.*)\0*.*;\0\0";
		char filePath[256] = { 0 };
		DialogResult result = Dialog::SaveFileName(filePath, sizeof(filePath), filter);
		if (result != DialogResult::OK) return; // キャンセルされた場合は保存を中止
		std::filesystem::path sceneFilePath(filePath);
		// 拡張子が.sceneでない場合、.sceneを追加
		if (sceneFilePath.extension() != ".scene")
		{
			sceneFilePath.replace_extension(".scene");
		}
		// シーンをJSON化
		json j;
		scene->name = sceneFilePath.stem().string(); // シーン名をファイル名に合わせる
		scene->Serialize(j);
		// JSONファイルに保存
		JsonFileHandler::SaveJsonToFile(j, sceneFilePath.string());
		// 保存完了のログを出力
		LOG_INFO("Scene saved: " + sceneFilePath.string());

		// バイナリファイルとしても保存
		{
			sceneFilePath.replace_extension(".bin");
			JsonFileHandler::SaveJsonToFile(j, sceneFilePath.string(), JsonIOFormat::Binary);
		}
	}
}
