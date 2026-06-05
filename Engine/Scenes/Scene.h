#pragma once
#include <string>

#include "Engine/Core/GameObject.h"
#include "Engine/EditorSupport/UndoRedoStack.h"

#include "Engine/Core/Component.h"
#include "Engine/Rendering/Camera/CameraSystem.h"
class DirectionalLightComponent;
class PointLightComponent;
class SpotLightComponent;

/**
 * @file
 * @brief シーンクラス。
 * @details ライフサイクル（Initialize/Update/Render/DrawGUI/Finalize）を提供します。
 */
class Scene
{
public:
	/** @brief オブジェクトマネージャ。*/
	std::unique_ptr<ObjectManager> objectManager;

	/** @brief カメラシステム。*/
	CameraSystem cameraSystem;

	/** @brief Undo/Redoスタック。シーン内での変更を管理します。*/
	CurryEngine::UndoRedoStack undoRedoStack;

	/* シーン内のオブジェクトやコンポーネントのキャッシュは、頻繁にアクセスされるものを対象に行います。これにより、シーン全体を検索する必要がある処理を減らし、パフォーマンスを向上させます。*/

	/** @brief シーン内のディレクショナルライトコンポーネントのキャッシュ。*/
	DirectionalLightComponent* directionalLight = nullptr;

	/** @brief シーン内のポイントライトコンポーネントのキャッシュ。*/
	std::vector<PointLightComponent*> pointLights;

	/** @brief シーン内のスポットライトコンポーネントのキャッシュ。*/
	std::vector<SpotLightComponent*> spotLights;

	/* ライトコンポーネントのキャッシュ管理用関数。これらはライトコンポーネントの `OnEnable` と `OnDisable` から呼び出され、シーンのキャッシュを更新します。*/

	/** @brief ディレクショナルライトをシーンのキャッシュに登録します。*/
	void RegisterDirectionalLight(DirectionalLightComponent* light) {
		directionalLight = light;
	}
	/** @brief ディレクショナルライトをシーンのキャッシュから削除します。*/
	void UnregisterDirectionalLight(DirectionalLightComponent* light) {
		if (directionalLight == light) {
			directionalLight = nullptr;
		}
	}

	/** @brief ポイントライトをシーンのキャッシュに登録します。*/
	void RegisterPointLight(PointLightComponent* light) {
		pointLights.push_back(light);
	}
	/** @brief ポイントライトをシーンのキャッシュから削除します。*/
	void UnregisterPointLight(PointLightComponent* light) {
		pointLights.erase(std::remove(pointLights.begin(), pointLights.end(), light), pointLights.end());
	}

	/** @brief スポットライトをシーンのキャッシュに登録します。*/
	void RegisterSpotLight(SpotLightComponent* light) {
		spotLights.push_back(light);
	}
	/** @brief スポットライトをシーンのキャッシュから削除します。*/
	void UnregisterSpotLight(SpotLightComponent* light) {
		spotLights.erase(std::remove(spotLights.begin(), spotLights.end(), light), spotLights.end());
	}

public:
	/** @brief 既定コンストラクタ。*/
	Scene();
	/** @brief デストラクタ。*/
	virtual ~Scene() = default;

	/** @brief 初期化処理（リソースのロードなど）。*/
	void Initialize();

	/** @brief フレーム開始処理（状態リセット）。*/
	void BeginFrame();

	/** @brief フレーム終了処理。*/
	void EndFrame();

	/** @brief 開始処理（シーン開始時に一度だけ呼び出される）。*/
	void Start();

	/**
	 * @brief 更新処理。
	 * @param deltaTime 経過時間（秒）。
	 */
	void Update(float deltaTime);

	/**
	 * @brief 更新処理の後処理。`Update()` の後に呼び出されます。
	 * @param deltaTime 経過時間（秒）。
	 */
	void LateUpdate(float deltaTime);

	/**
	 * @brief 固定更新処理。
	 * @param fixedDeltaTime 固定更新の経過時間（秒）。
	 */
	void FixedUpdate(float fixedDeltaTime);

	/**
	 * @brief 3D 描画の前処理。
	 * @param rtx レンダリングコンテキスト。
	 */
	void BeginRendering(RenderContext* rtx) {};

	/**
	 * @brief 3D 描画処理。
	 * @param rtx レンダリングコンテキスト。
	 */
	void Render(RenderContext* rtx);

	/**
	 * @brief 3D 描画の後処理。
	 * @param rtx レンダリングコンテキスト。
	 */
	void EndRendering(RenderContext* rtx) {};

	/**
	 * @brief 2D 描画処理。
	 * @param rtx レンダリングコンテキスト。
	 */
	void Draw(RenderContext* rtx);

	/** @brief 追加のImGUI描画処理。*/
	void DrawGUI() {};

	/** @brief 終了化処理（リソース解放など）。*/
	void Finalize();
public:
	/** @brief シーン名。*/
	std::string name;
	/** @brief 遷移が可能かどうか。*/
	bool canTransition;

	/** @brief シーンが開始されたかどうかを返します。*/
	bool IsStarted() const { return isStarted; }

public:
	/** @brief 名称からシーン内オブジェクトを取得します。*/
	GameObject* GetSceneObject(const std::string& name);
	/** @brief 名称でオブジェクトを破棄します。*/
	void Destroy(const std::string& name);
	/** @brief シーン内のすべてのオブジェクトを取得します。*/
	std::vector<std::shared_ptr<GameObject>> GetAllSceneObjects() const;
	/** @brief シーン内オブジェクト数を取得します。*/
	size_t GetSceneObjectsSize() const;


	/** @brief ObjectManagerへのアクセス。*/
	ObjectManager* GetObjectManager() const { return objectManager.get(); }

	/** @brief  CameraSystemへのアクセス。*/
	CameraSystem* GetCameraSystem() { return &cameraSystem; }

	/**
	 * @brief 指定型の全コンポーネントをシーン内から検索します。
	 * @tparam T 取得するコンポーネント型。
	 * @return 見つかった `T*` の配列。
	 */
	template<typename T>
	std::vector<T*> FindComponents() const {
		std::vector<T*> components;
		for (const std::weak_ptr<GameObject>& obj : objectManager->GetAll()) {
			if (!obj.expired()) {
				T* comp = obj.lock()->GetComponent<T>();
				if (comp) {
					components.emplace_back(comp);
				}
			}
		}
		return components;
	}

	/**
	 * @brief 指定型の最初のコンポーネントをシーン内から検索します。
	 * @tparam T 取得するコンポーネント型。
	 * @return 見つかった `T*`。存在しない場合は `nullptr`。
	 */
	template<typename T>
	T* FindComponentById(const ObjectId& id) const {
		const auto& cacheMap = objectManager->GetComponentCacheMap();
		auto it = cacheMap.find(id);
		if (it != cacheMap.end()) {
			if (auto compPtr = it->second.lock()) {
				return dynamic_cast<T*>(compPtr.get());
			}
		}
		return nullptr; // 見つからない場合は nullptr を返す
	}

	/**
	 * @brief 指定型の最初のコンポーネントをシーン内から検索し、共有ポインタで返します。
	 * @tparam T 取得するコンポーネント型。
	 * @return 見つかった `std::shared_ptr<T>`。存在しない場合は `nullptr`。
	 */
	template<typename T>
	std::shared_ptr<T> FindComponentPtrById(const ObjectId& id) const {
		const auto& cacheMap = objectManager->GetComponentCacheMap();
		auto it = cacheMap.find(id);
		if (it != cacheMap.end()) {
			if (auto compPtr = it->second.lock()) {
				return std::dynamic_pointer_cast<T>(compPtr);
			}
		}
		return nullptr; // 見つからない場合は nullptr を返す
	}

	/**
	 * @brief 指定IDのゲームオブジェクトをシーン内から検索します。
	 * @param id 取得するゲームオブジェクトのID。
	 * @return 見つかった `GameObject*`。存在しない場合は `nullptr`。
	 */
	GameObject* FindGameObjectById(const ObjectId& id) const;

	/** @brief シーン内のメインカメラを取得します。*/
	CameraComponent* GetMainCamera() {
		return cameraSystem.GetMainCamera();
	}


protected:
	friend class ObjectManager;
	friend class SceneManager;
	friend class Framework;
	friend class EditorGUI;
	
	/** @brief シリアライズ処理の呼び出し。*/
	void Serialize(json& j) const;

	/** @brief デシリアライズ処理の呼び出し。*/
	void Deserialize(const json& j);

private:
	/** @brief 前回のシリアライズデータ（差分保存用）。*/
	json previousData;

	/** @brief 固定更新のタイマー。*/
	float m_fixedUpdateTimer = 0.0f;

	float m_fixedUpdateInterval = (1.0f / 60.0f); // 固定更新の間隔（秒）

	/** @brief シーンが開始されたかどうか。*/
	bool isStarted = false;
};