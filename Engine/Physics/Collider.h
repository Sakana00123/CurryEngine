#pragma once
#include "Engine/Core/Component.h"

#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Rendering/Pipeline/RenderState.h"
#include "Engine/Core/Color.h"
#include <functional>
#include "Engine/Core/GameObject.h"
#include <unordered_map>

#define USE_PHYSX // PhysX を使用する場合は定義を有効にしてください。未定義の場合は独自の簡易物理エンジンが使用されます。
#include "Engine/Physics/Physics.h"

/**
 * @brief コライダ基底クラス。
 * @details イベント管理、衝突レポート集約、デバッグ描画、派生型との相互判定を定義します。
 */
class Collider : public Component
{
	C_REFLECT(Collider)
private:
	/** @brief Collision/Trigger 各イベントの購読者。*/
	std::vector<std::function<void(const CollisionInfo&)>> onCollisionEnterEvents;
	std::vector<std::function<void(const CollisionInfo&)>> onCollisionStayEvents;
	std::vector<std::function<void(const CollisionInfo&)>> onCollisionExitEvents;
	std::vector<std::function<void(const TriggerInfo&)>> onTriggerEnterEvents;
	std::vector<std::function<void(const TriggerInfo&)>> onTriggerStayEvents;
	std::vector<std::function<void(const TriggerInfo&)>> onTriggerExitEvents;
	/** @brief 当フレームの衝突集合。*/
	std::unordered_map<Collider*, CollisionInfo> currentCollisions;
	/** @brief 前フレームの衝突集合。*/
	std::unordered_map<Collider*, CollisionInfo> previousCollisions;
	/** @brief 何かにヒットしているか（内部利用）。*/
	bool isHit{ false };

protected:
	ShapeHandle m_shapeHandle{ INVALID_SHAPE_HANDLE };
	MaterialHandle m_materialHandle{ DEFAULT_MATERIAL_HANDLE };
	bool onEnableFlag = false; // 内部状態管理用フラグ
private:
	bool m_needSync = false; // 物理エンジンとの状態同期が必要か
public:
	/** @brief 既定コンストラクタ。*/
	Collider() = default;

	/** @brief 有効化時のコールバック。*/
	void OnEnable() override;

	/** @brief 無効化時のコールバック。*/
	void OnDisable() override;

	/** @brief 破棄直前のコールバック。*/
	void OnDestroy() override;

	/** @brief 終了処理。*/
	void Finalize() override;

	/** @brief トランスフォーム変更時のコールバック。*/
	void OnTransformChanged() override;

	void Awake() override;

	/** @brief 開始処理。シーン開始時に一度だけ呼び出されます。*/
	void Start() override;

	/** @brief 初期化（デバッグプリミティブ準備など）。*/
	void Initialize() override;

	/** @brief 固定フレーム更新。*/
	void FixedUpdate(float fixedDeltaTime) override;

	/** @brief フレーム更新。*/
	void LateUpdate(float deltaTime) override;
	
	/** @brief フレーム開始処理（状態リセット）。*/
	void BeginFrame() override;

	/**
	 * @brief 他コライダとの衝突を報告します。
	 * @param other 相手コライダ。
	 * @param info 衝突情報。
	 */
	void ReportCollision(Collider* other, const CollisionInfo& info);

	/** @brief フレーム終了処理（Enter/Stay/Exit/Trigger を通知）。*/
	void EndFrame() override;

	/**
	 * @brief コライダーのワールド行列を返します。
	 * @return ワールド変換行列。
	 */
	XMFLOAT4X4 CalculateColliderWorldTransform(const Vector3& localPos, const Vector3& localScale) const;


	/** @brief トリガーかどうか。*/
	C_FUNCTION()
	bool IsTrigger();

	/** @brief トリガー設定。*/
	C_FUNCTION()
	void SetTrigger(bool trigger);

	/* @brief 自動フィットするか。*/
	C_FUNCTION()
	bool GetAutoFit() const;

	/* @brief 自動フィット設定。*/
	C_FUNCTION()
	void SetAutoFit(bool autoFit);

	/** @brief 接触オフセットを取得します。*/
	C_FUNCTION()
	float GetContactOffset() const;

	/** @brief 接触オフセットを設定します。*/
	C_FUNCTION()
	void SetContactOffset(float offset);

	/**
	 * @brief 物理マテリアルを設定します。
	 * @param materialHandle 設定する物理マテリアルのMaterialHandle。
	 */
	void SetMaterial(MaterialHandle materialHandle);

	/** @brief 物理マテリアルを取得します。*/
	MaterialHandle GetMaterialHandle() const { return m_materialHandle; }

	/** @brief 物理マテリアルの特性を設定します。*/
	void SetMaterialData(const PhysicsMaterialData& data);

	/** @brief 物理マテリアルの特性を取得します。*/
	PhysicsMaterialData GetMaterialData() const;

	/** @brief 形状ハンドルを取得します。*/
	ShapeHandle GetShapeHandle() const { return m_shapeHandle; }

	/* @brief 物理エンジンとの状態同期が必要かを設定します。*/
	void SetNeedSync();

	/** @brief コライダーの形状を中心とサイズでフィットさせる。*/
	virtual void FitToBoundingBox(const Vector3& center, const Vector3& size) {}

	/** @brief 物理エンジンとの状態同期。*/
	virtual void SyncWithPhysics() = 0;

#ifdef USE_IMGUI
	/** @brief インスペクタ描画。*/
	virtual void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


	/** @brief シリアライズ。*/
	virtual json Serialize() const override;

	/** @brief デシリアライズ。*/
	virtual void Deserialize(const json& j) override;

	/** @brief ブロードキャスト登録（空間構造等への登録）。*/
	virtual void Register() = 0;

public:
	/** @brief 衝突開始イベントを追加。*/
	void AddOnCollisionEnterEvent(const std::function<void(const CollisionInfo&)>& func);

	/** @brief 衝突継続イベントを追加。*/
	void AddOnCollisionStayEvent(const std::function<void(const CollisionInfo&)>& func);

	/** @brief 衝突終了イベントを追加。*/
	void AddOnCollisionExitEvent(const std::function<void(const CollisionInfo&)>& func);

	/** @brief トリガー開始イベントを追加。*/
	void AddOnTriggerEnterEvent(const std::function<void(const TriggerInfo&)>& func);

	/** @brief トリガー継続イベントを追加。*/
	void AddOnTriggerStayEvent(const std::function<void(const TriggerInfo&)>& func);

	/** @brief トリガー終了イベントを追加。*/
	void AddOnTriggerExitEvent(const std::function<void(const TriggerInfo&)>& func);

private:
	//friend class Physics; // 内部から直接イベントを呼び出すためのフレンド宣言
	friend class SimulationEventCallback; // 内部から直接イベントを呼び出すためのフレンド宣言
	friend class Physics; // 内部から直接イベントを呼び出すためのフレンド宣言
	/** @brief 直近フレームの衝突開始を通知。*/
	void OnCollisionEnter(const CollisionInfo& info);
	/** @brief 直近フレームの衝突継続を通知。*/
	void OnCollisionStay(const CollisionInfo& info);
	/** @brief 直近フレームの衝突終了を通知。*/
	void OnCollisionExit(const CollisionInfo& info);
	/** @brief 直近フレームのトリガー開始を通知。*/
	void OnTriggerEnter(const TriggerInfo& info);
	/** @brief 直近フレームのトリガー継続を通知。*/
	void OnTriggerStay(const TriggerInfo& info);
	/** @brief 直近フレームのトリガー終了を通知。*/
	void OnTriggerExit(const TriggerInfo& info);
public:
	/** @brief 物理的に反応せず通知のみを行う場合に true。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("IsTrigger"), CurryEngine::PropertyAttributes::Setter("SetTrigger"))
	bool isTrigger{ false };

	/** @brief 自動フィットするか。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetAutoFit"), CurryEngine::PropertyAttributes::Setter("SetAutoFit"))
	bool autoFit{ false };

	/** @brief 衝突判定の際の接触オフセット。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetContactOffset"), CurryEngine::PropertyAttributes::Setter("SetContactOffset"), CurryEngine::PropertyAttributes::Range(0.000001f, 1000000.0f), CurryEngine::PropertyAttributes::Format("%.6f"))
	float contactOffset{ 0.02f };

	/** @brief AABB の最小点。*/
	virtual XMFLOAT3 Min() const { return { 0,0,0 }; }
	/** @brief AABB の最大点。*/
	virtual XMFLOAT3 Max() const { return { 0,0,0 }; }
};
