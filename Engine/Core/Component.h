#pragma once
#include <string>
#include <wrl.h>
#include <d3d11.h>
#ifdef USE_IMGUI
#include <imgui.h>
#include "Engine/EditorSupport/PropertyDrawContext.h"
#endif // USE_IMGUI

#include "Engine/EditorSupport/AutoRegisterComponent.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"
#include "Engine/Core/Math/Vector3.h"
#include "Engine/Core/Math/Quaternion.h"
#include "Engine/Core/Math/BoundingBox.h"
#include "Engine/Core/Object.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"

/**
 * @file
 * @brief 全てのコンポーネントの共通基底クラス。
 * @details ライフサイクル（Awake/Initialize/Update/Render/Draw/Finalize など）と
 *          有効/無効切替、所属 `GameObject` の参照、インスペクタ表示制御を提供します。
 */
class GameObject;

class Transform;

class Scene;

/**
 * @brief ゲームオブジェクトに付随する振る舞いの基底クラス。
 * @details 各種ライフサイクル仮想関数を定義し、派生クラスで必要な処理を実装します。
 */
class Component : public Object
{
	C_REFLECT(Component)
public:
	/** @brief 既定コンストラクタ。*/
	Component() = default;
	/**
	 * @brief デストラクタ。
	 * @details 終了処理として `OnDestroy()` を呼び出します。
	 */
	virtual ~Component() { OnDestroy(); SetEnabled(false); }
public:
	/** @brief このコンポーネントが所属する `GameObject`。*/
	GameObject* gameObject = nullptr;

	/** @brief 所有者 `GameObject` を取得。*/
	GameObject* GetOwner() const
	{
		// `gameObject` が有効な状態であれば返す。無効な状態（例: 破棄された）なら `nullptr` を返す。
		return gameObject;
	}

	/** @brief 所属 `GameObject` の `Transform` を取得。*/
	Transform* GetTransform() const;

	/** @brief 所属 `GameObject` の所属する `Scene` を取得。*/
	Scene* GetScene() const;

	/**
	 * @brief 有効/無効を設定します。
	 * @param set `true` で有効、`false` で無効。
	 */
	void SetEnabled(bool set);
	/**
	 * @brief 現在有効かを返します。
	 */
	bool IsEnabled() const;

	/**
	 * @brief 自身の有効状態を返します。
	 */
	bool IsEnabledSelf() const;

	/** @brief 属性フラグを取得。ComponentAttributes 名前空間のenumと対応。*/
	uint8_t GetAttributeFlags() const { return attributeFlags; }

	/**
	 * @brief このコンポーネントを破棄します。
	 */
	void Destroy();

	/**
	 * @brief 指定したコンポーネントを破棄します。
	 * @param obj 破棄するコンポーネント。
	 */
	static void Destroy(GameObject* obj);

	// Prefab のインスタンス化(ファイルパス版)
	static GameObject* Instantiate(const std::string& prefabPath, Transform* parent = nullptr, const Vector3& position = Vector3::Zero, const Quaternion& rotation = Quaternion::Identity);
	static GameObject* Instantiate(const std::string& prefabPath, const Vector3& position = Vector3::Zero, const Quaternion& rotation = Quaternion::Identity);
	// Prefab のインスタンス化(オブジェクト版)
	static GameObject* Instantiate(GameObject* prefab, Transform* parent = nullptr, const Vector3& position = Vector3::Zero, const Quaternion& rotation = Quaternion::Identity);
	static GameObject* Instantiate(GameObject* prefab, const Vector3& position = Vector3::Zero, const Quaternion& rotation = Quaternion::Identity);

protected:
	friend class GameObject;

	/** @brief フレーム開始処理（状態リセット）。*/
	virtual void BeginFrame() {};
	/** @brief フレーム終了処理。*/
	virtual void EndFrame() {};

	/** @brief 初期化処理（生成直後に一度だけ）。*/
	virtual void Initialize() {};
	/** @brief 開始前処理。全てのコンポーネントの `Initialize()` 後に一度だけ呼び出されます。*/
	virtual void Awake() {};
	/** @brief 開始処理。シーン開始時に一度だけ呼び出されます。*/
	virtual void Start() {};
	/**
	 * @brief 更新処理。
	 * @param deltaTime 前フレームからの経過時間（秒）。
	 */
	virtual void Update(float deltaTime) {};
	/**
	 * @brief 更新処理の後処理。`Update()` の後に呼び出されます。
	 * @param deltaTime 前フレームからの経過時間（秒）。
	 */
	virtual void LateUpdate(float deltaTime) {};
	/**
	 * @brief 固定更新処理。
	 * @param fixedDeltaTime 固定更新の経過時間（秒）。
	 */
	virtual void FixedUpdate(float fixedDeltaTime) {};
	/** @brief 3D 描画の前処理。*/
	virtual void BeginRendering(RenderContext* rtx) {};
	/** @brief 3D 描画処理。*/
	virtual void Render(RenderContext* rtx) {};
	/** @brief 3D 描画の後処理。*/
	virtual void EndRendering(RenderContext* rtx) {};
	/** @brief 2D 描画の前処理。*/
	virtual void Begin(RenderContext* rtx) {};
	/** @brief 2D 描画処理。*/
	virtual void Draw(RenderContext* rtx) {};
	/** @brief 2D 描画の後処理。*/
	virtual void End(RenderContext* rtx) {};

	/** @brief 有効化時のコールバック。*/
	virtual void OnEnable() {};
	/** @brief 無効化時のコールバック。*/
	virtual void OnDisable() {};
	/** @brief 破棄直前のコールバック。*/
	virtual void OnDestroy() {};
	/** @brief 終了処理。GameObject破棄前に呼び出されます。*/
	virtual void Finalize() {};

	friend class Transform;
	friend class GameObjectFactory;
	/** @brief 所属 `GameObject` の `Transform` が変更されたときのコールバック。*/
	virtual void OnTransformChanged() {};
	
	// TODO: 所有者 `GameObject` を直接参照するのではなく、IDベースに変更する。これにより、所有者が破棄された場合の安全性が向上します。
	/** @brief 所有者 `GameObject` を設定（エンジン内部用）。*/
	void SetOwner(GameObject* gameObject) { this->gameObject = gameObject; }

public:
	/** @brief このコンポーネントのバウンディングボックスを取得します。描画や衝突判定などで使用されます。*/
	virtual Math::BoundingBox GetBoundingBox() const { return Math::BoundingBox({}, {}); }

#ifdef USE_IMGUI
	/** @brief デバッグ GUI の描画（インスペクタなど）。*/
	virtual void DrawProperty(const struct PropertyDrawContext& ctx);
#endif // USE_IMGUI

	/** @brief 所有者 `GameObject` の一意識別子を設定（エンジン内部用）。*/
	C_FUNCTION()
	void SetOwnerId(ObjectId ownerId) { this->ownerId = ownerId; }

	C_FUNCTION()
	ObjectId GetOwnerId() const { return ownerId; }

	C_FUNCTION()
	void ChangeEnable() { SetEnabled(true); }

	C_FUNCTION()
	void ChangeDisable() { SetEnabled(false); }
	
private:
	friend class ComponentFactory;
	/** @brief 属性フラグを設定。ComponentAttributes 名前空間のenumと対応。*/
	void SetAttributeFlags(uint8_t flags) { attributeFlags = flags; }

protected:
	/** @brief 自身の有効状態。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::HideInInspector)
	bool enabledSelf = true;

	/** @brief 現在の有効状態。*/
	bool enabledInGame = false;
public:
	/** @brief インスペクタからこのコンポーネント自体を隠すか。*/
	bool hideInspector = false;
	/** @brief インスペクタでプロパティ項目を隠すか。*/
	bool hideInspectorProperty = false;
private:
	/** @brief 所属 `GameObject` の一意識別子。*/
	ObjectId ownerId = ObjectId::Invalid();

	/** @brief 属性フラグ。ComponentAttributes 名前空間のenumと対応。*/
	uint8_t attributeFlags = 0U;

	friend class GameObject;
	bool m_initialized = false; // Initialize() が呼び出されたかどうか
	bool m_awaked = false; // Awake() が呼び出されたかどうか
	bool m_started = false; // Start() が呼び出されたかどうか
};