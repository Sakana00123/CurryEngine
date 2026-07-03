#pragma once
#include <memory>
#include <vector>
#include <set>
#include <string>
#include <d3d11.h>
#include "Engine/Rendering/Pipeline/RenderContext.h"

#include "Engine/Core/Object.h"
#include "Engine/Core/Component.h"

class GameObject;
class Scene;
class EditorSelection;
#ifdef USE_IMGUI
class SceneManager;
#endif // USE_IMGUI

/** @brief オブジェクトマネージャ。シーン内のゲームオブジェクトを管理します。*/
class ObjectManager
{
public:
	ObjectManager(Scene* scene);
	~ObjectManager();

	/** @brief フレームの開始処理を呼び出します。*/
	void BeginFrame();
	/** @brief フレームの終了処理を呼び出します。*/
	void EndFrame();
	/** @brief ゲームオブジェクトの開始処理を呼び出します。*/
	void Start();
	/** @brief フレーム更新の前処理を行います。(これは必ず呼び出すこと。)*/
	void PreUpdate(float deltaTime);
	/** @brief フレーム更新を行います。*/
	void Update(float deltaTime);
	/** @brief フレーム更新の後処理を行います。*/
	void LateUpdate(float deltaTime);
	/** @brief 固定更新を行います。*/
	void FixedUpdate(float fixedDeltaTime);
	/** @brief 3D 描画処理を呼び出します。*/
	void Render(RenderContext* rtx);
	/** @brief 2D 描画処理を呼び出します。*/
	void Draw(RenderContext* rtx);

	void DrawGuizmo(RenderContext* rtx);
	void DrawHierarchy();
	void DrawProperty();

	static GameObject* Find(const std::string& name);
	static GameObject* Find(const ObjectId& id);
	static std::shared_ptr<GameObject> Find_Ptr(const std::string& name);
	static std::shared_ptr<GameObject> Find_Ptr(const ObjectId& id);

	static std::shared_ptr<Component> FindComponent(const ObjectId& id);

	GameObject* FindInObjects(const std::string& name);
	GameObject* FindInObjects(const ObjectId& id);
	
	/** @brief 指定した名前のオブジェクトを破棄予約します。*/
	void Destroy(const std::string& name);
	/** @brief すべてのオブジェクトを取得します。*/
	const std::vector<std::shared_ptr<GameObject>>& GetAll() const { return objects; }
	/** @brief すべてのオブジェクトを取得します。(変更可能)*/
	std::vector<std::shared_ptr<GameObject>>& GetAllMutable() { return objects; }
	/** @brief インスペクタ表示中のオブジェクトを取得します。*/
	GameObject* GetInspectorNode() const { return inspectorNode; }
	/** @brief インスペクタ表示中のオブジェクトを設定します。*/
	void SelectInspectorNode(GameObject* node);
	/** @brief インスペクタ表示をロックします。*/
	void LockInspector(bool lock);
	/** @brief インスペクタ表示がロックされているかを返します。*/
	bool IsInspectorLocked() const { return lockInspector; }
public:
	/** @brief シリアライズします。*/
	json Serialize() const;
	/** @brief デシリアライズします。*/
	void Deserialize(const json& j);

	/**
	 * @brief 指定したオブジェクトを複製します。
	 * @param original 複製元のオブジェクト。
	 * @return 複製されたオブジェクトのポインタ。
	 */
	GameObject* Duplicate(GameObject* original);

	/**
	 * @brief JSON からオブジェクトを生成します。
	 * @param j オブジェクト情報の JSON データ。
	 * @return 生成されたオブジェクトのポインタ。
	 */
	GameObject* Instantiate(const json& j);

	/**
	 * @brief オブジェクトをファイルに保存します。
	 * @param object 保存するオブジェクト。
	 * @param filePath 保存先のファイルパス。
	 */
	void SaveGameObject(GameObject* object, const std::string& filePath);

	/** @brief コンポーネントキャッシュマップを取得します。*/
	const std::unordered_map<ObjectId, std::weak_ptr<Component>>& GetComponentCacheMap() const { return componentCacheMap; }


	// TODO: 応急処置のため、後でリファクタリングすること。
	/** @brief 存在するコンポーネントのIDリストを取得します。*/
	std::unordered_set<ObjectId>& GetExistingComponentIds() { return existingComponentIds; }

	/** @brief エディタの選択状態を管理するオブジェクトを取得します。*/
	EditorSelection* GetEditorSelection() { return selection; }

	/** @brief オブジェクトの選択とインスペクタ表示をリセットします。*/
	void Reset();
private:
	
	/** @brief 指定したオブジェクトとその子オブジェクトを破棄します。*/
	void DestroyChildren(GameObject* object);
	friend class GameObjectFactory;
	friend class GameObject;
	/** @brief オブジェクトを登録します。*/
	void Register(std::shared_ptr<GameObject> object);
	
	friend class EditorGUI;
	GameObject* selectNode = nullptr;
	GameObject* inspectorNode = nullptr;
	bool lockInspector = false;
	EditorSelection* selection; // エディタの選択状態を管理するオブジェクト
private:
	friend class Scene;
	friend class Framework;
	friend class SceneMigrator;
	std::vector<std::shared_ptr<GameObject>> objects;
	std::vector<std::shared_ptr<GameObject>> erases;
	std::unordered_map<ObjectId, std::weak_ptr<Component>> componentCacheMap; // コンポーネントのキャッシュマップ

	std::unordered_set<ObjectId> existingComponentIds; // 存在するコンポーネントのIDリスト
	Scene* scene = nullptr; // 所属シーンへのポインタ
};