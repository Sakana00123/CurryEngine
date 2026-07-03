#pragma once
#include "Object.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_set>
#include "Component.h"
#include "Transform.h"
#include "ObjectManager.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

class Scene;

// ゲームオブジェクトのシリアライズバージョン管理用列挙型
namespace CurryEngine
{
    enum class GameObjectSerializeVersion
    {
		Legacy = 0,
		Priority = 1,

		Latest = Priority
	};
}

/**
 * @file
 * @brief シーン上に存在するゲームオブジェクトの管理クラス。
 * @details コンポーネントの追加/取得/削除、親子関係の構築、ライフサイクルの伝搬
 *         （Update/Draw/Render など）を担当します。
 */

/**
 * @brief ゲームオブジェクト本体。
 * @details 複数の `Component` を保持し、各種イベントや描画を一括管理します。
 */
class GameObject : public Object// : public std::enable_shared_from_this<GameObject>
{
	C_REFLECT(GameObject)
public:
    /** @brief 既定コンストラクタ。*/
    GameObject() = default;
    /** @brief デストラクタ。コンポーネント等の後始末を行います。*/
    virtual ~GameObject();
    /** @brief コピーコンストラクタ。*/
    GameObject(const GameObject&) = default;

    /**
     * @brief ゲームオブジェクトを生成し、名称を設定します。
     * @param name 希望する名前。
     */
    void Create(const std::string& name);

    /** @brief 名称を設定します。*/
	void SetName(const std::string& newName) override;

    /**
     * @brief 一意な名前を生成します。
     * @param name ベース名。
     * @return 一意化された名前。
     */
    std::string MakeUniqueName(const std::string& name);

    /**
    * @brief 指定した型のコンポーネントを追加します。
    * @tparam T コンポーネントの型。
    * @return 追加された T のポインタ。
    * @details Awake/Initialize まで呼び出されます。`SetEnable(true)` も実行します。
    */
    template<class T>
    T* AddComponent() {
        std::string className = typeid(T).name();
        className = className.substr(className.find_last_of(" ") + 1, className.length());
		if (!ComponentFactory::Exists(className)) { // 追加するコンポーネントが登録されているか確認
            _ASSERT_EXPR_A(false, ("Component type not registered: " + className).c_str());
		}
		std::shared_ptr<Component> component = ComponentFactory::Create(className);
        std::shared_ptr<T> instance = std::dynamic_pointer_cast<T>(component);
		AttachComponent(className, component);
        InitializeComponent(component);
		RefreshActiveInHierarchy();
        return instance.get();
    }

    /**
     * @brief 指定型の最初のコンポーネントを取得します。
     * @tparam T 取得するコンポーネント型。
     * @return 見つかった `T*`。存在しない場合は `nullptr`。
     */
    template<class T>
    T* GetComponent() {
        for (auto& component : _components) {
			if (std::weak_ptr<Component> weakComp = component; !weakComp.expired())
            {
                if (T* p = dynamic_cast<T*>(component.get())) {
                    return p;
                }
            }
        }
        return nullptr;
    }

    /**
     * @brief 指定型のコンポーネントが存在するかを確認します。
     * @param typeName 確認するコンポーネントの型名。
     * @return 存在する場合は `true`、そうでない場合は `false`。
	 */
    bool HasComponent(const std::string& typeName) {
        for (auto& component : _components) {
            if (component->GetTypeName() == typeName) {
                return true;
            }
        }
        return false;
	}

    /**
     * @brief 指定型の最初のコンポーネントを取得します。
     * @param typeName 取得するコンポーネントの型名。
	 * @return 見つかった `std::shared_ptr<Component>`。存在しない場合は `nullptr`。
     */
    std::shared_ptr<Component> GetComponentByTypeName(const std::string& typeName) {
        for (auto& component : _components) {
            if (component->GetTypeName() == typeName) {
                return component;
            }
        }
        return nullptr;
	}

    /**
     * @brief 指定型のコンポーネントをすべて取得します。
     * @param typeName 取得するコンポーネントの型名。
     * @return 見つかった `std::shared_ptr<Component>` の配列。存在しない場合は空の配列。
	 */
	std::vector<std::shared_ptr<Component>> GetComponentsByTypeName(const std::string& typeName) {
        std::vector<std::shared_ptr<Component>> components;
        for (auto& component : _components) {
            if (component->GetTypeName() == typeName) {
                components.push_back(component);
            }
        }
        return components;
	}

    /**
     * @brief 指定型のスマートポインタを取得します。
     * @tparam T 取得するコンポーネント型。
     * @return 見つかった `std::shared_ptr<T>`。存在しない場合は `nullptr`。
     */
    template<class T>
    std::shared_ptr<T> GetComponentShared() {
        for (auto& component : _components) {
            if (std::shared_ptr<T> p = std::dynamic_pointer_cast<T>(component)) {
                return p;
            }
        }
        return nullptr;
    }

    /**
     * @brief 指定型の全コンポーネントを取得します。
     * @tparam T 取得するコンポーネント型。
     * @return 見つかった `T*` の配列。
     */
    template<class T>
    std::vector<T*> GetComponents() {
        std::vector<T*> components;
        for (auto& component : _components) {
            if (T* p = dynamic_cast<T*>(component.get())) {
                components.push_back(p);
            }
        }
        return components;
    }

    /**
     * @brief ルート親のオブジェクトから指定型のコンポーネントを取得します。
     * @tparam T 取得するコンポーネント型。
     * @return 見つかった `T*`。存在しない場合は `nullptr`。
     */
    template<class T>
    T* GetComponentInParent() {
        if (GameObject* root = this->parent) {
            while (root->parent) {
                root = root->parent;
            }
            for (auto& component : root->_components) {
                if (T* p = dynamic_cast<T*>(component.get())) {
                    return p;
                }
            }
        }
        return nullptr;
    }

    /**
     * @brief 自身および直下の子から指定型のコンポーネントを取得します。
     * @tparam T 取得するコンポーネント型。
     * @return 見つかった `T*`。存在しない場合は `nullptr`。
     */
    template<class T>
    T* GetComponentInChildren() {
        if (T* component = this->GetComponent<T>()) {
            return component;
        }
        for (GameObject* child : this->children) {
            if (T* component = child->GetComponent<T>()) {
                return component;
            }
        }
        return nullptr;
    }

    /**
     * @brief 自身およびすべての子孫から指定型のコンポーネントをすべて取得します。
     * @tparam T 取得するコンポーネント型。
     * @return 見つかった `T*` の配列。
	 */
	template<class T>
    std::vector<T*> GetComponentsInChildren() {
        std::vector<T*> components;
        if (T* component = this->GetComponent<T>()) {
            components.push_back(component);
        }
		// 再帰的に子をたどる
		std::function<void(GameObject*)> fetchChildren;
        fetchChildren = [&](GameObject* object) {
            for (GameObject* child : object->children) {
                auto childComponents = child->GetComponents<T>();
                components.insert(components.end(), childComponents.begin(), childComponents.end());
                fetchChildren(child);
            }
			};
        fetchChildren(this);
		return components;
	}

    /**
     * @brief 全コンポーネントを取得します。
     * @return 内部で保持しているコンポーネントの配列（共有ポインタ）。
     */
    std::vector<std::shared_ptr<Component>> GetAllComponents() {
        return _components;
    }

    /**
     * @brief 指定型のコンポーネントを削除キューに登録します。
     * @tparam T 削除するコンポーネント型。
     */
    template<class T>
    void RemoveComponent() {
        std::string className = typeid(T).name();
        className = className.substr(className.find_last_of(" ") + 1, className.length());
        for (auto& component : _components) {
			if (component->GetTypeName() == className)
            {
                if (std::shared_ptr<T> p = std::dynamic_pointer_cast<T>(component)) {
                    removes.push_back(p);
                }
            }
        }
    }

    /** @brief 親オブジェクト設定。*/
    void SetParent(GameObject* newParent);

	/** @brief 型名を取得します。*/
	std::string GetTypeName() const override { return "GameObject"; }

private:
    friend class ObjectManager;
    friend class Canvas;

    /** @brief フレーム開始処理（状態リセット）。*/
    void BeginFrame();
    /** @brief フレーム終了処理。*/
    void EndFrame();

    /** @brief すべてのコンポーネントの Update 関数を呼び出す。*/
    void Update(float deltaTime);
	/** @brief すべてのコンポーネントの Update 後の処理を呼び出す。*/
	void LateUpdate(float deltaTime);
	/** @brief すべてのコンポーネントの FixedUpdate 関数を呼び出す。*/
	void FixedUpdate(float fixedDeltaTime);

    /** @brief すべてのコンポーネントの 3D 描画処理（前処理）。*/
    void BeginRendering(RenderContext* rtx);
    /** @brief すべてのコンポーネントの 3D 描画処理（本処理）。*/
    void Render(RenderContext* rtx);
    /** @brief すべてのコンポーネントの 3D 描画処理（後処理）。*/
    void EndRendering(RenderContext* rtx);

    /** @brief すべてのコンポーネントの 2D 描画前処理。*/
    void Begin(RenderContext* rtx);
    /** @brief すべてのコンポーネントの 2D 描画処理。*/
    void Draw(RenderContext* rtx);
    /** @brief すべてのコンポーネントの 2D 描画後処理。*/
    void End(RenderContext* rtx);

	/** @brief すべてのコンポーネントの破棄コールバックを呼び出す。*/
	void OnDestroy();

public:

	/** @brief コンポーネントをGameObjectにアタッチします。親子関係を構築します。*/
    void AttachComponent(const std::string& name, std::shared_ptr<Component>& component, bool generateId = true);

	/** @brief コンポーネントのセットアップを行います。Initialize を呼び出します。*/
	void InitializeComponent(std::shared_ptr<Component>& component);

	/** @brief コンポーネントの開始処理を行います。Awake を呼び出します。*/
	void AwakeComponents();

public:

    /** @brief すべてのコンポーネントのプロパティ描画。*/
    void DrawProperty();

	/** @brief 自身とすべてのコンポーネントのシリアライズを行う。*/
	json Serialize() const override;

	/** @brief 自身とすべてのコンポーネントのデシリアライズを行う。*/
	void Deserialize(const json& j) override;

	/** @brief コンポーネントのデシリアライズを行います。*/
	void DeserializeComponents(const json& j, const std::unordered_map<ObjectId, ObjectId>& idMap);

public:

    /** @brief 有効か（アクティブ）を返します。*/
	C_FUNCTION()
    bool IsActive() const;

    /**
     * @brief 有効/無効を設定します。
     * @param set `true` で有効、`false` で無効。
     */
	C_FUNCTION()
    void SetActive(bool set);

	/** @brief 自身が有効かを返します。*/
	bool IsActiveSelf() const;

	/** @brief 階層上でアクティブかを更新します。*/
	void RefreshActiveInHierarchy();

	/** @brief コンポーネントのアクティブ状態を更新します。*/
    void RefreshComponentActive(Component* component);

    /**
     * @brief 自身を破棄します。遅延オプション付き。
     * @param delay 破棄予定のコンポーネントを実際に削除するまでの遅延時間（秒）。デフォルトは 0 秒。
	 */
	void Destroy(float delay = 0.0f);

    /**
     * @brief 指定のコンポーネントを破棄します。
     * @param component 破棄対象。
     */
    void Destroy(Component* component);

public:
	/** @brief 自身の変換を取得します。*/
	Transform* GetTransform() const { return transform; }

	/** @brief 親ゲームオブジェクトを取得します。存在しない場合は `nullptr` を返します。*/
	GameObject* GetParent() const { return parent; }

	/** @brief 子ゲームオブジェクト一覧を取得します。*/
	std::vector<GameObject*> GetChildren() const { return children; }

	/** @brief 所属シーンを取得します。*/
	Scene* GetScene() const { return scene; }

	/** @brief 親ゲームオブジェクトの ID を取得します。存在しない場合は `ObjectId::Invalid()` を返します。*/
	ObjectId GetParentId() const { return parentId; }

	/** @brief 所属レイヤーを取得します。*/
	C_FUNCTION()
	int GetLayer() const { return layer; }

	/** @brief 所属レイヤーを設定します。*/
	C_FUNCTION()
    void SetLayer(int layer);

    /** @brief 自身の変換。*/
    Transform* transform = nullptr;

    /** @brief 親ゲームオブジェクト。今後廃止予定。*/
    GameObject* parent = nullptr;

	/** @brief 親ゲームオブジェクトの ID。GameObject*のparentは今後廃止予定。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly, CurryEngine::PropertyAttributes::NonSerialized)
	ObjectId parentId = ObjectId::Invalid();

    

    /** @brief 子ゲームオブジェクト一覧。*/
    std::vector<GameObject*> children;



private:
    friend class SceneMigrator;
	friend class GameObjectFactory;
    /** @brief 所属シーン。*/
	Scene* scene = nullptr;
	
    /** @brief 所属レイヤー。描画順や衝突判定などで使用されることを想定。*/
    int layer = 0;

    /** @brief 所有するコンポーネント一覧。*/
    std::vector<std::shared_ptr<Component>> _components;
    /** @brief 削除予定のコンポーネント一覧。*/
    std::vector<std::shared_ptr<Component>> removes;
    /** @brief Create 済みか。*/
    bool isCreated = false;
	/** @brief 親がまだ設定されていない場合の保留親 ID。*/
	ObjectId pendingParentID = ObjectId::Invalid();
	/** @brief 自身のアクティブフラグ。*/
    C_PROPERTY()
	bool activeSelf = true;
	/** @brief 階層上でアクティブか。*/
	bool activeInHierarchy = true;

    C_PROPERTY()
	bool isDefaultOpenOnHierarchy = true; // デフォルトでヒエラルキー上で開いているか

private:
	int version = 0; // シリアライズバージョン
	float destroyDelay = 0.0f; // 破棄予定のコンポーネントを実際に削除するまでの遅延時間（秒）
};