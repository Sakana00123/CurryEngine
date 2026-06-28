#pragma once
#include "Component.h"
#include "Engine/Physics/CollisionEvent.h"

class ScriptComponent : public Component, public ICollisionEventCallback, public ITriggerEventCallback
{
	C_REFLECT(ScriptComponent)
public:
	ScriptComponent() = default;
	virtual ~ScriptComponent() override { OnScriptUnload(); }

	/** @brief 有効化処理の呼び出し。*/
	void OnEnable() override;

	/** @brief 無効化処理の呼び出し。*/
	void OnDisable() override;

	/** @brief 衝突コールバック。*/
	void OnCollisionEnter(const CollisionInfo& info) override;
	/** @brief 衝突コールバック。*/
	void OnCollisionStay(const CollisionInfo& info) override;
	/** @brief 衝突コールバック。*/
	void OnCollisionExit(const CollisionInfo& info) override;
	/** @brief トリガーコールバック。*/
	void OnTriggerEnter(const TriggerInfo& info) override;
	/** @brief トリガーコールバック。*/
	void OnTriggerStay(const TriggerInfo& info) override;
	/** @brief トリガーコールバック。*/
	void OnTriggerExit(const TriggerInfo& info) override;


	/**
	 * @brief 初期化処理の呼び出し。
	 */
	void Initialize() override;

	/**
	 * @brief 開始処理の呼び出し。
	 */
	void Start() override;

	/**
	 * @brief フレーム更新処理。
	 * @param deltaTime 経過時間（秒）。
	 * @details `Update()` の呼び出し。スクリプトがアタッチされたオブジェクトがシーンに存在する限り、毎フレーム呼び出される。
	 */
	void Update(float deltaTime) override;

#ifdef USE_IMGUI
	/** @brief プロパティ描画。*/
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

	/** @brief シリアライズ処理。*/
	json Serialize() const override;
	
	/** @brief デシリアライズ処理。*/
	void Deserialize(const json& j) override;

	/** @brief 型名の取得。*/
	std::string GetTypeName() const override { return scriptName; }

public:

	void OnScriptUnload();

	void OnPreScriptReload();

	void OnPostScriptReload();

	void OnScriptReload();

	const std::string& GetScriptName() const { return scriptName; }

	void* GetGCHandle() const { return m_gcHandle; }

private:
	friend class GameObject;
	// スクリプト名
	C_PROPERTY()
	std::string scriptName = "";

	// C# GCHandle - ScriptBridge が管理するポインタ
	void* m_gcHandle = nullptr;

	// C# スクリプトのフィールド値を一時保持する Json オブジェクト
	json m_pendingFields;

	//bool m_isStartCalled = false; // Start が呼び出されたかどうかのフラグ
	std::unordered_map<std::string, json> m_fieldValues; // フィールド名と値のマップ
};
