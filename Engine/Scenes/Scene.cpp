#include "pch.h"
#include "Scene.h"
#include "SceneManager.h"

#include "Engine/Utils/JsonFileHandler.h"
#include "Engine/Core/ObjectManager.h"
#include "Engine/Rendering/Camera/EditorCamera.h"
#include "Engine/Physics/Physics.h"
#include "Engine/Core/Time.h"
#include "Engine/Utils/UniqueIdGenerator.h"

#include "Engine/Editor/SceneViewWindow.h"
#include "Engine/Editor/ImportSettings/ImportSettingsWindow.h"
#include <Engine\Editor\EffectEditor.h>

Scene::Scene() : canTransition(false)
{
	cameraSystem.Initialize(this);
	objectManager = std::make_unique<ObjectManager>(this);

	for (int i = 0; i < _countof(editorCameras); ++i) {
		editorCameras[i] = std::make_unique<EditorCamera>();
		editorCameras[i]->Initialize();
	}
	editorCameras[EDITOR_CAMERA_SCENE_VIEW]->SetUpdateFlagFunction([]() { return CurryEngine::SceneViewWindow::Get().IsFocused(); }); // シーンビューウィンドウがフォーカスされている場合に更新するように設定
	editorCameras[EDITOR_CAMERA_PREVIEW]->SetUpdateFlagFunction([]() { return CurryEngine::Resources::ImportSettingsWindow::IsOpen(); }); // インポート設定ウィンドウが開いている場合に更新するように設定
	editorCameras[EDITOR_CAMERA_EFFECT_PREVIEW]->SetUpdateFlagFunction([]() { return EffectEditor::IsPreviewFocused(); }); // インポート設定ウィンドウが開いている場合に更新するように設定
}

void Scene::Initialize()
{
	// シーンデータの読み込み
	std::string stem = this->name;
	json j;
#ifdef _DEBUG
	// シーンデータの読み込み
	if (SceneManager::state == SceneManager::State::PlayToEdit)
	{
		// プレイモードからエディットモードに戻る場合、保存しておいたデータを使用
		j = SceneManager::previousData.sceneJson;
	}
	else if (SceneManager::state == SceneManager::State::EditToPlay)
	{
		// シーンファイルパスの構築
		const std::string filePath = "./Assets/Scenes/" + stem + SceneManager::runtimeSuffix + ".scene";
		// 通常のシーンデータ読み込み
		JsonFileHandler::LoadJsonFromFile(j, filePath);
	}
	else
#else
	const std::string filePathBin = "./Assets/Scenes/" + stem + ".bin";
	if (std::filesystem::exists(std::filesystem::path(filePathBin))) {
		// シーンファイルパスの構築
		// バイナリ形式のシーンデータ読み込み
		JsonFileHandler::LoadJsonFromFile(j, filePathBin);
	}
	else
#endif // _DEBUG
	{
		// シーンファイルパスの構築
		const std::string filePath = "./Assets/Scenes/" + stem + ".scene";
		// 通常のシーンデータ読み込み
		JsonFileHandler::LoadJsonFromFile(j, filePath);
	}

	// デシリアライズ
	this->Deserialize(j);

	// シーンの状態遷移処理
	switch (SceneManager::state)
	{
	case SceneManager::State::PlayToEdit:
	{
		// プレイモードからエディットモードに戻る場合、状態をエディット中に設定
		SceneManager::state = SceneManager::State::Editing;
		break;
	}
	case SceneManager::State::EditToPlay:
	{
		// エディットモードからプレイモードに移行する場合、状態をプレイ中に設定
		SceneManager::state = SceneManager::State::Playing;
		break;
	}
	}
}

void Scene::BeginFrame()
{
	// シーン内の全オブジェクトのフレーム開始処理
	objectManager->BeginFrame();
}

void Scene::EndFrame()
{
	// シーン内の全オブジェクトのフレーム終了処理
	objectManager->EndFrame();
}

void Scene::Start()
{
	// シーン内の全オブジェクトの開始処理
	if (isStarted == false)
	{
		isStarted = true;
		objectManager->Start();
	}
}

void Scene::Update(float deltaTime)
{
	// シーン内の全オブジェクトの更新の前処理
	objectManager->PreUpdate(deltaTime);

	// シーン内の全オブジェクトの更新
	objectManager->Update(deltaTime);

	// 固定更新のタイマーを更新
	m_fixedUpdateTimer += deltaTime;

	//// 固定更新の呼び出し
	//while (m_fixedUpdateTimer >= m_fixedUpdateInterval) {
	//	FixedUpdate(m_fixedUpdateInterval); // 固定更新の呼び出し
	//	m_fixedUpdateTimer -= m_fixedUpdateInterval;
	//}
	FixedUpdate(deltaTime);

	// シーン内の全オブジェクトの更新の後処理
	LateUpdate(deltaTime);

	// エディターカメラの更新
	for (int i = 0; i < _countof(editorCameras); ++i) {
		if (editorCameras[i]) {
			editorCameras[i]->Update(deltaTime);
		}
	}
}

void Scene::LateUpdate(float deltaTime)
{
	// シーン内の全オブジェクトの更新の後処理
	objectManager->LateUpdate(deltaTime);
}

void Scene::FixedUpdate(float fixedDeltaTime)
{
	// シーン内の全オブジェクトの固定更新
	objectManager->FixedUpdate(fixedDeltaTime);

	if (!SceneManager::IsTransition())
	{
		// 物理エンジンの固定更新
		Physics::FixedUpdate(fixedDeltaTime);
	}
}

void Scene::Render(RenderContext* rtx)
{
	// シーン内の全オブジェクトの3D描画
	objectManager->Render(rtx);
}

void Scene::Draw(RenderContext* rtx)
{
	// シーン内の全オブジェクトの2D描画
	objectManager->Draw(rtx);
}

void Scene::Finalize()
{
	// シーン内の全オブジェクトの終了化処理
	objectManager->Reset();
	objectManager.reset();
}

GameObject* Scene::GetSceneObject(const std::string& name) {
	return objectManager->FindInObjects(name);
}
void Scene::Destroy(const std::string& name) {
	objectManager->Destroy(name);
}

std::vector<std::shared_ptr<GameObject>> Scene::GetAllSceneObjects() const {
	return objectManager->objects;
}

size_t Scene::GetSceneObjectsSize() const {
	return objectManager->objects.size();
}

GameObject* Scene::FindGameObjectById(const ObjectId& id) const {
	for (const std::weak_ptr<GameObject>& obj : objectManager->GetAll()) {
		if (!obj.expired() && obj.lock()->GetId() == id) {
			return obj.lock().get();
		}
	}
	return nullptr; // 見つからない場合は nullptr を返す
}

std::shared_ptr<GameObject> Scene::FindGameObjectPtrById(const ObjectId& id) const {
	for (const std::weak_ptr<GameObject>& obj : objectManager->GetAll()) {
		if (!obj.expired() && obj.lock()->GetId() == id) {
			return obj.lock();
		}
	}
	return nullptr; // 見つからない場合は nullptr を返す
}

void Scene::Serialize(json& j) const
{
	j["name"] = name;
	//j["canTransition"] = canTransition;
	
	// エディターカメラのシリアライズ
	j["editorCamera"] = GetEditorCamera(EDITOR_CAMERA_SCENE_VIEW)->Serialize();

	// シーン内の全オブジェクトをシリアライズ
	j["objects"] = objectManager->Serialize();

	// 次のインスタンスIDを保存
	//int nextId = GetCurrentInstanceID();
	//LOG_INFO("Serializing scene: saving next instance ID " + std::to_string(nextId));
	//j["nextInstanceId"] = nextId;

}

void Scene::Deserialize(const json& j) {
	name = j.value("name", name);
	//canTransition = j.value("canTransition", canTransition);

	// 次のインスタンスIDを復元
	//if (j.contains("nextInstanceId")) {
	//	int nextId = j["nextInstanceId"].get<int>();
	//	LOG_INFO("Deserializing scene: resetting instance ID counter to " + std::to_string(nextId));
	//	ResetInstanceID(nextId);
	//}

	// エディターカメラのデシリアライズ
	if (j.contains("editorCamera"))
	{
		GetEditorCamera(EDITOR_CAMERA_SCENE_VIEW)->Deserialize(j["editorCamera"]);
	}

	// シーン内の全オブジェクトをデシリアライズ
	if (j.contains("objects")) {
		objectManager->Deserialize(j["objects"]);
	}
}