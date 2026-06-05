#pragma once

#include "Engine/Core/Transform.h"
#include <map>
#include <unordered_map>
#include <vector>
#include <PxPhysicsAPI.h>

#include "Engine/Core/Layer.h"
#include "Engine/Physics/CollisionEvent.h"

//#define BINARY_PHYSICS_DATA // 物理データをバイナリ形式で保存するかどうかのフラグ。デバッグ用に有効にすることができます。

enum class PhysicMaterialCombineMode
{
	Average,    // 平均値を使用して衝突反応を計算
	Minimum,    // 最小値を使用して衝突反応を計算
	Multiply,   // 乗算して衝突反応を計算
	Maximum     // 最大値を使用して衝突反応を計算
};

using ActorHandle = int;
using ShapeHandle = int;
using MaterialHandle = int;

#define INVALID_ACTOR_HANDLE -1
#define INVALID_SHAPE_HANDLE -1
#define INVALID_MATERIAL_HANDLE -1

#define DEFAULT_STATIC_FRICTION 1.5f
#define DEFAULT_DYNAMIC_FRICTION 1.0f
#define DEFAULT_BOUNCINESS 0.25f
#define DEFAULT_FRICTION_COMBINE_MODE PhysicMaterialCombineMode::Average
#define DEFAULT_BOUNCE_COMBINE_MODE PhysicMaterialCombineMode::Average

#define DEFAULT_MATERIAL_HANDLE 0

// 1フレームあたりの最大接触点数
#define MAX_CONTACTS_PER_PAIR 8

class Rigidbody;
class Collider;

// アクターのデータを管理する構造体
struct ActorData
{
	physx::PxRigidActor* actor; // アクターへのポインタ
	Transform* transform; // アクターのトランスフォームへのポインタ
};

struct RaycastHit
{
	Vector3 point; // 衝突点の位置
	Vector3 normal; // 衝突面の法線
	float distance; // レイの発射点から衝突点までの距離
	Collider* collider; // 衝突したコライダへのポインタ
};

// 物理マテリアルの特性を定義する構造体
struct PhysicsMaterialData
{
	std::string name; // マテリアルの名前
	float staticFriction = DEFAULT_STATIC_FRICTION; // 静止摩擦係数
	float dynamicFriction = DEFAULT_DYNAMIC_FRICTION; // 動摩擦係数
	float bounciness = DEFAULT_BOUNCINESS; // 反発係数
	PhysicMaterialCombineMode frictionCombineMode = DEFAULT_FRICTION_COMBINE_MODE; // 摩擦の組み合わせモード
	PhysicMaterialCombineMode bounceCombineMode = DEFAULT_BOUNCE_COMBINE_MODE; // 反発の組み合わせモード
};

// 物理マテリアルを管理する構造体
struct PhysicsMaterial
{
	MaterialHandle handle; // マテリアルのハンドル
	PhysicsMaterialData data; // マテリアルの特性データ
	physx::PxMaterial* pxMaterial; // 物理エンジンのマテリアルへのポインタ
};

struct ColliderData
{
	//ActorHandle actorHandle; // コライダが属するアクターのハンドル
	//ShapeHandle shapeHandle; // コライダの形状のハンドル
	MaterialHandle materialHandle; // コライダに適用する物理マテリアルのハンドル
	bool isTrigger; // トリガーかどうか
	float contactOffset; // 接触オフセット
	Collider* collider; // コライダへのポインタ（オプション、必要に応じて使用）
};

// コライダーの形状を定義する構造体
struct BoxColliderData : public ColliderData
{
	Vector3 halfExtents; // ボックスの半分のサイズ（幅/2, 高さ/2, 奥行き/2）
	Vector3 center; // ボックスの中心位置（ローカル座標）
};

struct SphereColliderData : public ColliderData
{
	float radius; // 球の半径
	Vector3 center; // 球の中心位置（ローカル座標）
};

struct CapsuleColliderData : public ColliderData
{
	float radius; // カプセルの半径
	float height; // カプセルの高さ（中心から両端までの距離）
	Vector3 center; // カプセルの中心位置（ローカル座標）
};

struct MeshColliderData : public ColliderData
{
	std::vector<Vector3> vertices; // メッシュの頂点データ
	std::vector<int> indices; // メッシュのインデックスデータ
};

struct HeightFieldColliderData : public ColliderData
{
	std::vector<float> heightData; // 高さフィールドの高さデータ
	int numRows; // 高さフィールドの行数
	int numCols; // 高さフィールドの列数
	float rowScale; // 行方向のスケール
	float colScale; // 列方向のスケール
};

struct CharacterControllerData : public ColliderData
{
	float radius; // キャラクターコントローラーの半径
	float height; // キャラクターコントローラーの高さ
	Vector3 center; // キャラクターコントローラーの中心位置（ローカル座標）
};


// 物理エンジンのイベントコールバックを処理するクラス
class SimulationEventCallback : public physx::PxSimulationEventCallback
{
public:
	// トリガーの継続イベントのペアをクリアする関数
	void ClearTriggerStayPairs();

	// 特定の形状に関連するトリガーの継続イベントのペアをクリアする関数
	void ClearTriggerStayPairsForShape(physx::PxShape* shape);

	// 物理エンジンのイベントを処理する関数
	void Update();

	// 衝突イベントの呼び出し関数
	void CallCollisionEvents();
	// トリガーイベントの呼び出し関数
	void CallTriggerEvents();

protected:
	// PxSimulationEventCallback の純粋仮想関数をオーバーライド
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override{}
	void onWake(physx::PxActor** actors, physx::PxU32 count) override{}
	void onSleep(physx::PxActor** actors, physx::PxU32 count) override{}
	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
	void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override{}

private:
	using ShapePair = std::pair<physx::PxShape*, physx::PxShape*>;
	std::map<ShapePair, physx::PxTriggerPair> pxTriggerStayPairs; // トリガーの継続イベントを管理するためのマップ

	std::vector<std::pair<Collider*, CollisionInfo>> collisionEnterEvents{}; // 衝突開始イベントのペアを管理するためのベクター
	std::vector<std::pair<Collider*, CollisionInfo>> collisionStayEvents{}; // 衝突継続イベントのペアを管理するためのベクター
	std::vector<std::pair<Collider*, CollisionInfo>> collisionExitEvents{}; // 衝突終了イベントのペアを管理するためのベクター
	std::vector<std::pair<Collider*, TriggerInfo>> triggerEnterEvents{}; // トリガー開始イベントのペアを管理するためのベクター
	//std::vector<std::pair<Collider*, TriggerInfo>> triggerStayEvents{}; // トリガー継続イベントのペアを管理するためのベクター
	std::vector<std::pair<Collider*, TriggerInfo>> triggerExitEvents{}; // トリガー終了イベントのペアを管理するためのベクター
};

// クエリフィルタコールバックを処理するクラス
class FilterShader : public physx::PxQueryFilterCallback
{
public:
	// コリジョンペアのフィルタリングを行う関数
	static physx::PxFilterFlags SimulationFilter(
		physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

	// PxQueryFilterCallback の純粋仮想関数をオーバーライド
	physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;
	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit, const physx::PxShape* shape, const physx::PxRigidActor* actor) override;
};

// 物理エンジンを管理するクラス
class Physics
{
public:
	Physics();
	~Physics();

	/**
	 * @brief 物理エンジンの初期化
	 */
	static void Initialize();
	/**
	 * @brief 物理エンジンの終了処理
	 */
	static void Terminate();

	/**
	 * @brief 物理エンジンの設定を保存
	 * @details 物理エンジンの現在の設定を保存します。これには、物理マテリアルの特性やレイヤー設定などが含まれます。ゲームの終了時やビルド前などに呼び出されることを想定しています。
	 */
	static void SaveSettings();

	/**
	 * @brief 物理エンジンの状態をリセット
	 * @details すべての物理オブジェクトを削除し、物理エンジンの状態をリセットします。シーンの切り替えやゲームのリセット時などに呼び出されることを想定しています。
	 */
	static void Clean();

	// --- シリアライズ / デシリアライズ ---

	/**
	 * @brief 物理エンジンの状態をJSON形式でシリアライズ
	 * @return 物理エンジンの状態を表すJSONオブジェクト
	 */
	static json Serialize();

	/**
	 * @brief JSON形式のデータから物理エンジンの状態をデシリアライズ
	 * @param data 物理エンジンの状態を表すJSONオブジェクト
	 */
	static void Deserialize(const json& data);

	/**
	 * @brief 物理エンジンの更新
	 * @param fixedDeltaTime 固定更新のデルタタイム
	 */
	static void FixedUpdate(float fixedDeltaTime);

	/**
	 * @brief 物理エンジンのデバッグ描画
	 * @param rtx 描画に必要なコンテキスト情報を持つRenderContextへのポインタ
	 */
	static void RenderDebug(RenderContext* rtx);

	/**
	 * @brief 物理エンジンのデバッグ描画
	 */
	static void DrawGUI();

	// --- コンポーネントからの登録用 ---
	
	/**
	 * @brief Rigidbodyを必要に応じて生成し、物理エンジンに登録する
	 * @param rigidbody 登録するRigidbodyコンポーネント
	 */
	static void RegisterPendingRigidbody(Rigidbody* rigidbody);

	/**
	 * @brief 登録を保留しているRigidbodyを物理エンジンから登録解除する
	 * @param rigidbody 登録解除するRigidbodyコンポーネント
	 */
	static void UnregisterPendingRigidbody(Rigidbody* rigidbody);

	/**
	 * @brief Colliderを必要に応じて生成し、物理エンジンに登録する
	 * @param collider 登録するColliderコンポーネント
	 */
	static void RegisterPendingCollider(Collider* collider);

	/**
	 * @brief 登録を保留しているColliderを物理エンジンから登録解除する
	 * @param collider 登録解除するColliderコンポーネント
	 */
	static void UnregisterPendingCollider(Collider* collider);

	/**
	 * @brief 登録を保留しているものをすべて物理エンジンに登録する
	 */
	static void FlushPendingRegistrations();

	/**
	 * @brief Actorを必要に応じて生成し、コライダーを持つゲームオブジェクトを物理エンジンに登録する
	 * @param transform 登録するコライダーを持つゲームオブジェクトのTransformコンポーネント
	 * @param isDynamic 登録するコライダーが動的かどうか
	 * @return 登録されたActorのActorHandle。登録に失敗した場合はINVALID_ACTOR_HANDLEを返す
	 */
	static ActorHandle RegisterBody(Transform* transform, bool isDynamic);

	/**
	 * @brief コライダーを持つゲームオブジェクトを物理エンジンから登録解除する
	 * @param transform 登録解除するコライダーを持つゲームオブジェクトのTransformコンポーネント
	 */
	static void UnregisterBody(Transform* transform);

	/**
	 * @brief Transformが破棄されたときのコールバック関数。関連するActorやShapeを物理エンジンから削除する。
	 * @param transform 破棄されるTransformコンポーネント
	 * @details Transformが破棄されると、そのTransformに関連付けられたActorやShapeも物理エンジンから削除される必要があります。この関数は、TransformのデストラクタやOnDestroy()などの適切な場所で呼び出されることを想定しています。
	 */
	static void OnTrnasformDestroyed(Transform* transform);

	// --- 形状 (Collider) の追加 ---

	/**
	 * @brief BoxShapeを生成し、対応するActorにアタッチ、物理エンジンへの登録を行う。
	 * @param transform BoxColliderを追加するTransformコンポーネント
	 * @param data 追加するBoxColliderの形状データ
	 * @param outHandle 追加されたBoxColliderのShapeHandleを格納する参照。追加に成功した場合は有効なShapeHandleが格納され、失敗した場合はINVALID_SHAPE_HANDLEが格納されます。
	 * @return 追加に成功した場合はtrue、失敗した場合はfalse
	 */
	static bool AddBoxShape(Transform* transform, const BoxColliderData& data, ShapeHandle& outHandle);

	/**
	 * @brief SphereShapeを生成し、対応するActorにアタッチ、物理エンジンへの登録を行う。
	 * @param transform SphereColliderを追加するTransformコンポーネント
	 * @param data 追加するSphereColliderの形状データ
	 * @param outHandle 追加されたSphereColliderのShapeHandleを格納する参照。追加に成功した場合は有効なShapeHandleが格納され、失敗した場合はINVALID_SHAPE_HANDLEが格納されます。
	 * @return 追加に成功した場合はtrue、失敗した場合はfalse
	 */
	static bool AddSphereShape(Transform* transform, const SphereColliderData& data, ShapeHandle& outHandle);

	/**
	 * @brief CapsuleShapeを生成し、対応するActorにアタッチ、物理エンジンへの登録を行う。
	 * @param transform CapsuleColliderを追加するTransformコンポーネント
	 * @param data 追加するCapsuleColliderの形状データ
	 * @param outHandle 追加されたCapsuleColliderのShapeHandleを格納する参照。追加に成功した場合は有効なShapeHandleが格納され、失敗した場合はINVALID_SHAPE_HANDLEが格納されます。
	 * @return 追加に成功した場合はtrue、失敗した場合はfalse
	 */
	static bool AddCapsuleShape(Transform* transform, const CapsuleColliderData& data, ShapeHandle& outHandle);

	/**
	 * @brief TriangleMeshShapeを生成し、対応するActorにアタッチ、物理エンジンへの登録を行う。
	 * @param transform MeshColliderを追加するTransformコンポーネント
	 * @param data 追加するMeshColliderの形状データ
	 * @param outHandle 追加されたMeshColliderのShapeHandleを格納する参照。追加に成功した場合は有効なShapeHandleが格納され、失敗した場合はINVALID_SHAPE_HANDLEが格納されます。
	 * @return 追加に成功した場合はtrue、失敗した場合はfalse
	 */
	static bool AddTriangleMeshShape(Transform* transform, const MeshColliderData& data, ShapeHandle& outHandle);

	/**
	 * @brief ConvexMeshShapeを生成し、対応するActorにアタッチ、物理エンジンへの登録を行う。
	 * @param transform ConvexMeshColliderを追加するTransformコンポーネント
	 * @param data 追加するConvexMeshColliderの形状データ
	 * @param outHandle 追加されたConvexMeshColliderのShapeHandleを格納する参照。追加に成功した場合は有効なShapeHandleが格納され、失敗した場合はINVALID_SHAPE_HANDLEが格納されます。
	 * @return 追加に成功した場合はtrue、失敗した場合はfalse
	 */
	static bool AddConvexMeshShape(Transform* transform, const MeshColliderData& data, ShapeHandle& outHandle);

	// --- 形状 (Collider) の情報取得 ---


	// --- 形状 (Collider) の情報設定 ---


	// --- シーンの設定 ---

	/**
	 * @brief シーンの重力を設定する
	 * @param gravity 設定する重力のベクトル。通常はY軸方向に負の値を設定します（例: (0, -9.81, 0)）。
	 */
	static void SetGravity(const Vector3& gravity);

	/**
	 * @brief シーンの重力を取得する
	 * @return 現在のシーンの重力のベクトル
	 */
	static Vector3 GetGravity();


	// --- レイキャスト ---

	/**
	 * @brief レイキャストを実行する
	 * @param origin レイの発射点の位置
	 * @param direction レイの発射方向（正規化されている必要があります）
	 * @param maxDistance レイの最大距離
	 * @param hitInfo レイが衝突した場合に衝突情報を格納するRaycastHit構造体への参照。レイが衝突しなかった場合は内容が変更されません。
	 * @param layerMask レイキャストで検出するレイヤーのビットフラグ。省略した場合はすべてのレイヤーが検出されます。
	 * @return レイが何かに衝突した場合はtrue、衝突しなかった場合はfalse
	 */
	static bool Raycast(const Vector3& origin, const Vector3& direction, float maxDistance, RaycastHit& hitInfo, LayerMask layerMask = 0xFFFFFFFF);



	// --- レイヤー設定 ---

	/**
	 * @brief Shapeのレイヤーを設定する
	 * @param shapeHandle レイヤーを設定するShapeのShapeHandle
	 * @param layer 設定するレイヤーのID（0~31）
	 */
	static void SetLayer(const ShapeHandle& shapeHandle, Layer layer);

	/**
	 * @brief Shapeのレイヤーマスクを設定する
	 * @param shapeHandle レイヤーマスクを設定するShapeのShapeHandle
	 * @param layerMask 設定するレイヤーマスクのビットフラグ
	 */
	static void SetLayerMask(const ShapeHandle& shapeHandle, LayerMask layerMask);

	/**
	 * @brief Shapeのレイヤーとレイヤーマスクを設定する
	 * @param shapeHandle レイヤーとレイヤーマスクを設定するShapeのShapeHandle
	 * @param layer 設定するレイヤーのID（0~31）
	 * @param layerMask 設定するレイヤーマスクのビットフラグ
	 */
	static void UpdateFilterData(const ShapeHandle& shapeHandle, Layer layer, LayerMask layerMask);

	/**
	 * @brief Shapeのレイヤーを取得する
	 * @param shapeHandle レイヤーを取得するShapeのShapeHandle
	 * @return 取得したレイヤーのID（0~31）。取得に失敗した場合は0を返します。
	 */
	//static Layer GetLayer(const ShapeHandle& shapeHandle);

	/**
	 * @brief Shapeのレイヤーマスクを取得する
	 * @param shapeHandle レイヤーマスクを取得するShapeのShapeHandle
	 * @return 取得したレイヤーマスクのビットフラグ。取得に失敗した場合は0を返します。
	 */
	//static LayerMask GetLayerMask(const ShapeHandle& shapeHandle);

	/**
	 * @brief 指定したレイヤーの衝突マスクを取得する
	 * @param layer 衝突マスクを取得するレイヤーのID（0~31）
	 * @return 取得した衝突マスクのビットフラグ。取得に失敗した場合は0を返します。
	 */
	static LayerMask GetCollisionMask(Layer layer);

	/**
	 * @brief 2つのレイヤー間の衝突を無視するかどうかを取得する
	 * @param layer1 レイヤー1のID（0~31）
	 * @param layer2 レイヤー2のID（0~31）
	 * @return 無視する場合はtrue、無視しない場合はfalse
	 */
	static bool GetIgnoreLayerCollision(Layer layer1, Layer layer2);

	/**
	 * @brief 2つのレイヤー間の衝突を無視するかどうかを設定する
	 * @param layer1 レイヤー1のID（0~31）
	 * @param layer2 レイヤー2のID（0~31）
	 * @param ignore 無視する場合はtrue、無視しない場合はfalse。省略した場合はtrueになります。
	 */
	static void SetIgnoreLayerCollision(Layer layer1, Layer layer2, bool ignore = true);


	// --- 物理マテリアルの追加 ---

	/**
	 * @brief 物理マテリアルを作成して物理エンジンに登録する
	 * @param data 作成する物理マテリアルの特性を表すPhysicsMaterialData構造体
	 * @param overrideHandle 作成する物理マテリアルに割り当てるMaterialHandle。省略した場合は自動的に割り当てられます。既存の有効なMaterialHandleを指定した場合は、そのMaterialHandleを持つ物理マテリアルが上書きされます。
	 * @return 作成に成功した場合は作成された物理マテリアルを表すPhysicsMaterial構造体、失敗した場合は無効なMaterialHandleを持つPhysicsMaterial構造体
	 */
	static PhysicsMaterial CreateAndRegisterMaterial(const PhysicsMaterialData& data, MaterialHandle overrideHandle = INVALID_MATERIAL_HANDLE);

	// --- 物理マテリアルの削除 ---

	/**
	 * @brief 物理マテリアルを削除し、物理エンジンから登録解除する
	 * @param handle 削除する物理マテリアルのMaterialHandle
	 */
	static void RemoveMaterial(MaterialHandle handle);


	// --- 物理マテリアルの情報取得 ---

	/**
	 * @brief 物理マテリアルの特性を取得する
	 * @param handle 特性を取得する物理マテリアルのMaterialHandle
	 * @param outMaterial 取得した物理マテリアルの特性を格納するPhysicsMaterialData構造体への参照
	 * @return 取得に成功した場合はtrue、失敗した場合はfalse
	 */
	static bool GetMaterialData(MaterialHandle handle, PhysicsMaterialData& outMaterial);

	// --- 物理マテリアルの情報設定 ---

	/**
	 * @brief 物理マテリアルの特性を設定する
	 * @param handle 特性を設定する物理マテリアルのMaterialHandle
	 * @param material 設定する物理マテリアルの特性を表すPhysicsMaterialData構造体
	 * @return 設定に成功した場合はtrue、失敗した場合はfalse
	 */
	static bool SetMaterialData(MaterialHandle handle, const PhysicsMaterialData& material);

	// --- 物理マテリアルマップの管理 ---

	/**
	 * @brief 物理マテリアルの名前とMaterialHandleのマップを更新する
	 * @details 物理マテリアルの追加、削除、特性の変更などが行われた際に、この関数を呼び出してマップを最新の状態に保つ必要があります。
	 */
	static void UpdateMaterialNameMap();

	// --- 物理マテリアルの名前からMaterialHandleを取得する ---

	/**
	 * @brief 物理マテリアルの名前からMaterialHandleを取得する
	 * @param name 取得する物理マテリアルの名前
	 * @param outHandle 取得したMaterialHandleを格納する参照。取得に成功した場合は有効なMaterialHandleが格納され、失敗した場合はINVALID_MATERIAL_HANDLEが格納されます。
	 * @return 取得に成功した場合はtrue、失敗した場合はfalse
	 */
	static bool GetMaterialHandleByName(const std::string& name, MaterialHandle& outHandle);

	/**
	 * @brief すべての物理マテリアルの名前とMaterialHandleを取得する
	 * @param outNames すべての物理マテリアルの名前を格納するstd::vector<const char*>への参照
	 * @param outHandles すべての物理マテリアルのMaterialHandleを格納するstd::vector<MaterialHandle>への参照
	 */
	static void GetAllMaterialNamesAndHandles(std::vector<const char*>& outNames, std::vector<MaterialHandle>& outHandles);


	// --- 物理操作 (Rigidbody) の窓口 ---

	/**
	 * @brief Actorの質量を設定する
	 * @param handle 質量を設定するActorHandle
	 * @param mass 設定する質量の値
	 */
	static void SetMass(const ActorHandle& handle, float mass);

	/**
	 * @brief Actorの慣性テンソルを設定する
	 * @param handle 慣性テンソルを設定するActorHandle
	 * @param inertiaTensor 設定する慣性テンソルのベクトル（x, y, z成分がそれぞれの軸の慣性モーメントを表す）
	 */
	static void SetInertiaTensor(const ActorHandle& handle, const Vector3& inertiaTensor);

	/**
	 * @brief Actorを起こす
	 * @param handle 起こすActorHandle
	 * @details 物理エンジンでは、一定時間動きがないアクターは自動的にスリープ状態になります。スリープ状態のアクターは物理シミュレーションの計算から除外されるため、パフォーマンスが向上します。ただし、スリープ状態のアクターは外部からの力や衝撃を受けても反応しません。WakeUp関数を呼び出すことで、スリープ状態のアクターを起こして物理シミュレーションに再び参加させることができます。
	 */
	static void WakeUp(const ActorHandle& handle);

	/**
	 * @brief Actorをスリープ状態にする
	 * @param handle スリープ状態にするActorHandle
	 * @details 物理エンジンでは、一定時間動きがないアクターは自動的にスリープ状態になります。スリープ状態のアクターは物理シミュレーションの計算から除外されるため、パフォーマンスが向上します。ただし、スリープ状態のアクターは外部からの力や衝撃を受けても反応しません。PutToSleep関数を呼び出すことで、アクターを強制的にスリープ状態にすることができます。
	 */
	static void PutToSleep(const ActorHandle& handle);

	/**
	 * @brief Actorの慣性テンソルの回転を設定する
	 * @param handle 慣性テンソルの回転を設定するActorHandle
	 * @param inertiaTensorRotation 設定する慣性テンソルの回転を表すクォータニオン
	 */
	static bool IsSleeping(const ActorHandle& handle);

	/**
	 * @brief Shapeの物理マテリアルを設定する
	 * @param shapeHandle 物理マテリアルを設定するShapeHandle
	 * @param materialHandle 設定する物理マテリアルのMaterialHandle
	 */
	static void SetMaterial(const ShapeHandle& shapeHandle, const MaterialHandle& materialHandle);

	/**
	 * @brief Shapeのトリガー状態を設定する
	 * @param shapeHandle トリガー状態を設定するShapeHandle
	 * @param isTrigger トリガー状態を表すフラグ。trueの場合はトリガーとして機能し、falseの場合は通常のコライダーとして機能します。
	 */
	static void SetTrigger(const ShapeHandle& shapeHandle, bool isTrigger);

	/**
	 * @brief Shapeの接触オフセットを設定する
	 * @param shapeHandle 接触オフセットを設定するShapeHandle
	 * @param contactOffset 設定する接触オフセットの値。接触オフセットは、物理エンジンが衝突を検出する際の距離の余裕を表す値で、通常は正の値で設定されます。
	 */
	static void SetContactOffset(const ShapeHandle& shapeHandle, float contactOffset);

	/**
	 * @brief Actorの重力の影響を設定する
	 * @param handle 重力の影響を設定するActorHandle
	 * @param useGravity 重力の影響を受けるかどうかのフラグ。trueの場合は重力の影響を受け、falseの場合は重力の影響を受けません。
	 */
	static void SetUseGravity(const ActorHandle& handle, bool useGravity);

	/**
	 * @brief Actorの連続衝突検出 (CCD) の使用を設定する
	 * @param handle CCDの使用を設定するActorHandle
	 * @param useCCD CCDを使用するかどうかのフラグ。trueの場合はCCDを使用し、falseの場合は使用しません。CCDは、高速で移動する物体が他の物体をすり抜けるのを防ぐための機能です。
	 */
	static void SetUseCCD(const ActorHandle& handle, bool useCCD);

	/**
	 * @brief Actorの移動や回転の制約を設定する
	 * @param handle 移動や回転の制約を設定するActorHandle
	 * @param constraints 設定するRigidbodyConstraintsの値。複数の制約を組み合わせることができます。
	 */
	static void SetConstraints(const ActorHandle& handle, physx::PxRigidDynamicLockFlags constraints);

	/**
	 * @brief Actorの移動や回転の制約を取得する
	 * @param handle 移動や回転の制約を取得するActorHandle
	 * @return 取得したRigidbodyConstraintsの値。複数の制約が設定されている場合は、それらが組み合わされた値が返されます。
	 */
	static physx::PxRigidDynamicLockFlags GetConstraints(const ActorHandle& handle);

	/**
	 * @brief Actorの線形減衰を設定する
	 * @param handle 線形減衰を設定するActorHandle
	 * @param linearDamping 設定する線形減衰の値。線形減衰は、物体の速度に比例して減速する効果を表す値で、通常は正の値で設定されます。
	 */
	static void SetLinearDamping(const ActorHandle& handle, float linearDamping);

	/**
	 * @brief Actorの線形減衰を取得する
	 * @param handle 線形減衰を取得するActorHandle
	 * @return 取得した線形減衰の値。線形減衰は、物体の速度に比例して減速する効果を表す値で、通常は正の値で設定されます。
	 */
	static float GetLinearDamping(const ActorHandle& handle);

	/**
	 * @brief Actorの線形抵抗を設定する
	 * @param handle 線形抵抗を設定するActorHandle
	 * @param linearDrag 設定する線形抵抗の値。線形抵抗は、物体の速度に比例して減速する効果を表す値で、通常は正の値で設定されます。
	 */
	static void SetLinearDrag(const ActorHandle& handle, float linearDrag);

	/**
	 * @brief Actorの線形抵抗を取得する
	 * @param handle 線形抵抗を取得するActorHandle
	 * @return 取得した線形抵抗の値。線形抵抗は、物体の速度に比例して減速する効果を表す値で、通常は正の値で設定されます。
	 */
	static float GetLinearDrag(const ActorHandle& handle);

	/**
	 * @brief Actorの最大線形速度を設定する
	 * @param handle 最大線形速度を設定するActorHandle
	 * @param maxLinearVelocity 設定する最大線形速度の値。最大線形速度は、物体が移動できる最大の速度を表す値で、通常は正の値で設定されます。
	 */
	static void SetMaxLinearVelocity(const ActorHandle& handle, float maxLinearVelocity);

	/**
	 * @brief Actorの最大線形速度を取得する
	 * @param handle 最大線形速度を取得するActorHandle
	 * @return 取得した最大線形速度の値。最大線形速度は、物体が移動できる最大の速度を表す値で、通常は正の値で設定されます。
	 */
	static float GetMaxLinearVelocity(const ActorHandle& handle);

	/**
	 * @brief Actorの角減衰を設定する
	 * @param handle 角減衰を設定するActorHandle
	 * @param angularDamping 設定する角減衰の値。角減衰は、物体の角速度に比例して減速する効果を表す値で、通常は正の値で設定されます。
	 */
	static void SetAngularDamping(const ActorHandle& handle, float angularDamping);

	/**
	 * @brief Actorの角減衰を取得する
	 * @param handle 角減衰を取得するActorHandle
	 * @return 取得した角減衰の値。角減衰は、物体の角速度に比例して減速する効果を表す値で、通常は正の値で設定されます。
	 */
	static float GetAngularDamping(const ActorHandle& handle);

	/**
	 * @brief Actorの角抵抗を設定する
	 * @param handle 角抵抗を設定するActorHandle
	 * @param angularDrag 設定する角抵抗の値。角抵抗は、物体の角速度に比例して減速する効果を表す値で、通常は正の値で設定されます。
	 */
	static void SetAngularDrag(const ActorHandle& handle, float angularDrag);

	/**
	 * @brief Actorの角抵抗を取得する
	 * @param handle 角抵抗を取得するActorHandle
	 * @return 取得した角抵抗の値。角抵抗は、物体の角速度に比例して減速する効果を表す値で、通常は正の値で設定されます。
	 */
	static float GetAngularDrag(const ActorHandle& handle);

	/**
	 * @brief Actorの最大角速度を設定する
	 * @param handle 最大角速度を設定するActorHandle
	 * @param maxAngularVelocity 設定する最大角速度の値。最大角速度は、物体が回転できる最大の速度を表す値で、通常は正の値で設定されます。
	 */
	static void SetMaxAngularVelocity(const ActorHandle& handle, float maxAngularVelocity);

	/**
	 * @brief Actorの最大角速度を取得する
	 * @param handle 最大角速度を取得するActorHandle
	 * @return 取得した最大角速度の値。最大角速度は、物体が回転できる最大の速度を表す値で、通常は正の値で設定されます。
	 */
	static float GetMaxAngularVelocity(const ActorHandle& handle);

	/**
	 * @brief Actorのスリープ状態を設定する
	 * @param handle スリープ状態を設定するActorHandle
	 * @param isSleeping スリープ状態を表すフラグ。trueの場合はActorがスリープ状態になり、物理シミュレーションの影響を受けなくなります。falseの場合はActorがアクティブな状態になり、物理シミュレーションの影響を受けるようになります。
	 */
	static void SetSleepThreshold(const ActorHandle& handle, float sleepThreshold);

	/**
	 * @brief Actorのスリープ状態を取得する
	 * @param handle スリープ状態を取得するActorHandle
	 * @return 取得したスリープ状態の値。スリープ状態は、物理シミュレーションの影響を受けなくなる速度の閾値を表す値で、通常は正の値で設定されます。
	 */
	static float GetSleepThreshold(const ActorHandle& handle);

	/**
	 * @brief Actorに力を加える
	 * @param handle 力を加えるActorHandle
	 * @param force 加える力のベクトル （グローバル座標）
	 * @param mode 力の加え方を指定する物理エンジンの力モード（例: eFORCE, eIMPULSEなど）
	 */
	static void AddForce(const ActorHandle& handle, const Vector3& force, physx::PxForceMode::Enum mode);

	/**
	 * @brief Actorの特定の位置に力を加える（ローカル座標）
	 * @param handle 力を加えるActorHandle
	 * @param localForce 加える力のベクトル （ローカル座標）
	 * @param localPosition 力を加える位置のベクトル（ローカル座標）
	 * @param mode 力の加え方を指定する物理エンジンの力モード（例: eFORCE, eIMPULSEなど）
	 */
	static void AddLocalForceAtLocalPosition(const ActorHandle& handle, const Vector3& localForce, const Vector3& localPosition, physx::PxForceMode::Enum mode);

	/**
	 * @brief Actorの特定の位置に力を加える（ローカル座標）
	 * @param handle 力を加えるActorHandle
	 * @param localForce 加える力のベクトル （ローカル座標）
	 * @param position 力を加える位置のベクトル（ローカル座標）
	 * @param mode 力の加え方を指定する物理エンジンの力モード（例: eFORCE, eIMPULSEなど）
	 */
	static void AddLocalForceAtPosition(const ActorHandle& handle, const Vector3& localForce, const Vector3& position, physx::PxForceMode::Enum mode);

	/**
	 * @brief Actorの特定の位置に力を加える（ローカル座標）
	 * @param handle 力を加えるActorHandle
	 * @param force 加える力のベクトル （グローバル座標）
	 * @param localPosition 力を加える位置のベクトル（ローカル座標）
	 * @param mode 力の加え方を指定する物理エンジンの力モード（例: eFORCE, eIMPULSEなど）
	 */
	static void AddForceAtLocalPosition(const ActorHandle& handle, const Vector3& force, const Vector3& localPosition, physx::PxForceMode::Enum mode);

	/**
	 * @brief Actorの特定の位置に力を加える
	 * @param handle 力を加えるActorHandle
	 * @param force 加える力のベクトル （グローバル座標）
	 * @param position 力を加える位置のベクトル（グローバル座標）
	 * @param mode 力の加え方を指定する物理エンジンの力モード（例: eFORCE, eIMPULSEなど）
	 */
	static void AddForceAtPosition(const ActorHandle& handle, const Vector3& force, const Vector3& position, physx::PxForceMode::Enum mode);

	/**
	 * @brief Actorにトルクを加える
	 * @param handle トルクを加えるActorHandle
	 * @param torque 加えるトルクのベクトル (回転軸の方向を表すベクトルで、ベクトルの大きさがトルクの強さを表す)
	 * @param mode トルクの加え方を指定する物理エンジンの力モード（例: eFORCE, eIMPULSEなど）
	 */
	static void AddTorque(const ActorHandle& handle, const Vector3& torque, physx::PxForceMode::Enum mode);

	/**
	 * @brief Actorの速度を直接設定する
	 * @param handle 速度を設定するActorHandle
	 * @param velocity 設定する速度のベクトル
	 */
	static void SetVelocity(const ActorHandle& handle, const Vector3& velocity);

	/**
	 * @brief Actorの速度を取得する
	 * @param handle 速度を取得するActorHandle
	 * @param outVelocity 取得した速度を格納するVector3参照
	 */
	static void GetVelocity(const ActorHandle& handle, Vector3& outVelocity);

	/**
	 * @brief Actorの角速度を直接設定する
	 * @param handle 角速度を設定するActorHandle
	 * @param angularVelocity 設定する角速度のベクトル
	 */
	static void SetAngularVelocity(const ActorHandle& handle, const Vector3& angularVelocity);

	/**
	 * @brief Actorの角速度を取得する
	 * @param handle 角速度を取得するActorHandle
	 * @param outAngularVelocity 取得した角速度を格納するVector3参照
	 */
	static void GetAngularVelocity(const ActorHandle& handle, Vector3& outAngularVelocity);

	/**
	 * @brief キネマティック設定を行う。キネマティックなオブジェクトは物理シミュレーションの影響を受けず、直接位置や回転を設定できます。
	 * @param handle キネマティック設定を行うActorHandle
	 * @param isKinematic キネマティックにするかどうかのフラグ。trueの場合はキネマティック、falseの場合は物理シミュレーションの影響を受けるようになります。
	 */
	static void SetKinematic(const ActorHandle& handle, bool isKinematic);

	/**
	 * @brief キネマティックなActorの目標位置と回転を設定する。キネマティックなActorは物理シミュレーションの影響を受けないため、この関数で直接位置や回転を設定できます。
	 * @param handle 目標位置と回転を設定するActorHandle
	 * @param pos 設定する目標位置のベクトル
	 * @param rot 設定する目標回転のクォータニオン
	 */
	static void SetKinematicTarget(const ActorHandle& handle, const Vector3& pos, const Quaternion& rot);

	/**
	 * @brief Actorのグローバルポーズを直接設定する
	 * @param handle グローバルポーズを設定するActorHandle
	 * @param pos 設定するグローバル位置のベクトル
	 * @param rot 設定するグローバル回転のクォータニオン
	 */
	static void SetGlobalPose(const ActorHandle& handle, const Vector3& pos, const Quaternion& rot);

	/**
	 * @brief Actorのグローバルポーズを取得する
	 * @param handle グローバルポーズを取得するActorHandle
	 * @param outPos 取得したグローバル位置を格納するVector3参照
	 * @param outRot 取得したグローバル回転を格納するQuaternion参照
	 */
	static void GetGlobalPose(const ActorHandle& handle, Vector3& outPos, Quaternion& outRot);

	/**
	 * @brief Shapeのローカルポーズを取得する
	 * @param shapeHandle ローカルポーズを取得するShapeHandle
	 * @param outLocalPos 取得したローカル位置を格納するVector3参照
	 * @param outLocalRot 取得したローカル回転を格納するQuaternion参照
	 */
	static void GetLocalPose(const ShapeHandle& shapeHandle, Vector3& outLocalPos, Quaternion& outLocalRot);

	/**
	 * @brief Shapeのローカルポーズを設定する
	 * @param shapeHandle ローカルポーズを設定するShapeHandle
	 * @param localPos 設定するローカル位置のベクトル
	 * @param localRot 設定するローカル回転のクォータニオン
	 */
	static void SetLocalPose(const ShapeHandle& shapeHandle, const Vector3& localPos, const Quaternion& localRot);

	/**
	 * @brief Shapeのジオメトリを取得する
	 * @param shapeHandle ジオメトリを取得するShapeHandle
	 * @param outGeometry 取得したジオメトリを格納するphysx::PxGeometry参照
	 */
	static void GetGeometry(const ShapeHandle& shapeHandle, physx::PxGeometry& outGeometry);
	
	/**
	 * @brief Shapeのジオメトリを設定する
	 * @param shapeHandle ジオメトリを設定するShapeHandle
	 * @param geometry 設定するジオメトリのphysx::PxGeometry構造体
	 */
	static void SetGeometry(const ShapeHandle& shapeHandle, const physx::PxGeometry& geometry);

	/**
	 * @brief Actorの有効/無効を設定する
	 * @param actorHandle 有効/無効を設定するActorHandle
	 * @param enable Actorを有効にするかどうかのフラグ。trueの場合はActorが有効になり、物理シミュレーションに影響を与えるようになります。falseの場合はActorが無効になり、物理シミュレーションに影響を与えなくなります。
	 */
	static void SetActorEnable(const ActorHandle& actorHandle, bool enable);

	/**
	 * @brief Actorの有効/無効を取得する
	 * @param actorHandle 有効/無効を取得するActorHandle
	 * @return Actorが有効な場合はtrue、無効な場合はfalse
	 */
	static bool IsActorEnabled(const ActorHandle& actorHandle);

	/**
	 * @brief Shapeの有効/無効を設定する
	 * @param shapeHandle 有効/無効を設定するShapeHandle
	 * @param enable Shapeを有効にするかどうかのフラグ。trueの場合はShapeが有効になり、物理シミュレーションに影響を与えるようになります。falseの場合はShapeが無効になり、物理シミュレーションに影響を与えなくなります。
	 * @param colliderData Shapeの有効/無効を設定する際に必要なColliderData構造体。Shapeを有効にする場合は、ColliderDataに必要な情報を設定して渡す必要があります。Shapeを無効にする場合は、ColliderDataは無視されます。
	 */
	static void SetShapeEnable(const ShapeHandle& shapeHandle, bool enable, const ColliderData& colliderData);

	/**
	 * @brief Shapeの有効/無効を取得する
	 * @param shapeHandle 有効/無効を取得するShapeHandle
	 * @return Shapeが有効な場合はtrue、無効な場合はfalse
	 */
	static bool IsShapeEnabled(const ShapeHandle& shapeHandle);
	
	static bool HasActor(Transform* transform); // Transform* に対応する ActorHandle が存在するかを確認する関数

	static bool HasShape(ShapeHandle shapeHandle); // ShapeHandle に対応する PxShape* が存在するかを確認する関数


	static ActorHandle GetActorHandle(Transform* transform); // Transform* に対応する ActorHandle を取得する関数(存在しない場合は-1を返す)

	static ActorHandle CreateActorHandle(); // ActorHandle を新規作成する関数

	static Transform* GetTransform(ActorHandle actorHandle); // ActorHandle に対応する Transform* を取得する関数(存在しない場合はnullptrを返す)

	static physx::PxRigidActor* GetActor(ActorHandle actorHandle); // ActorHandle に対応する PxRigidActor* を取得する関数(存在しない場合はnullptrを返す)

	static physx::PxShape* GetShape(ShapeHandle shapeHandle); // ShapeHandle に対応する PxShape* を取得する関数(存在しない場合はnullptrを返す)

	static ShapeHandle GetShapeHandle(physx::PxShape* shape); // PxShape* に対応する ShapeHandle を取得する関数(存在しない場合は-1を返す)

	//static ShapeHandle AddShape(ActorHandle actorHandle, physx::PxShape* shape); // ActorHandle に対応する PxRigidActor* に PxShape* を追加し、対応する ShapeHandle を返す関数(追加に失敗した場合は-1を返す)

	static ShapeHandle CreateShapeHandle(); // ShapeHandle を新規作成する関数

	static void RegisterShape(ShapeHandle shapeHandle, physx::PxShape* shape); // ShapeHandle と PxShape* を対応付けて登録する関数

	static void RemoveActor(ActorHandle actorHandle); // Transform* と ActorHandle に対応する PxRigidActor* を削除する関数

	static void RemoveShape(ShapeHandle shapeHandle); // ActorHandle と ShapeHandle に対応する PxShape* を削除する関数

	//static void ClearShapes(ActorHandle actorHandle); // ActorHandle に対応する PxRigidActor* からすべての形状を削除する関数

	static physx::PxScene* GetScene() { return pxScene; }

	static physx::PxPhysics* GetPhysics() { return pxPhysics; }

	static physx::PxMaterial* GetMaterial(MaterialHandle handle) { return m_materialMap[handle].pxMaterial; }

	static physx::PxControllerManager* GetControllerManager() { return pxControllerManager; }

private:
	static inline ActorHandle nextActorHandle = 1; // 次に割り当てるActorHandleの値
	
	static inline ShapeHandle nextShapeHandle = 1; // 次に割り当てるShapeHandleの値

	static inline std::unordered_map<ActorHandle, ActorData> m_actorMap; // ActorHandle を PxRigidActor* にマッピングするためのハッシュマップ

	static inline std::map<ShapeHandle, physx::PxShape*> m_shapeMap; // ShapeHandle を PxShape* にマッピングするためのマップ。unordered_map ではなく map を使用している理由は、ShapeHandle が連続した整数であるため、map の方が効率的に管理できると判断したためです。

	static inline std::unordered_map<MaterialHandle, PhysicsMaterial> m_materialMap; // MaterialHandle をキー、PhysicsMaterial を値とするハッシュマップ

	static inline std::vector<std::pair<std::string, MaterialHandle>> m_materialNameMap; // マテリアルの名前とMaterialHandleのペアを格納するベクター。マテリアルの名前からMaterialHandleを検索するために使用します。
	static inline bool m_materialNameMapDirty = false; // マテリアルの名前とMaterialHandleのマップが最新でないことを示すフラグ。マテリアルの追加、削除、特性の変更などが行われた際にtrueに設定され、UpdateMaterialNameMap()が呼び出された際にfalseにリセットされます。

	// --- レイヤー衝突設定の管理 ---

	// レイヤーIDをキー、レイヤーの名前を値とするハッシュマップ。レイヤーIDからレイヤーの名前を取得するために使用します。
	static inline std::unordered_map<Layer, std::string> m_layerNameMap;

	// 追加待ちのRigidbodyとColliderのリスト。これらは、物理エンジンの更新ループの適切なタイミングで物理エンジンに追加されます。
	static std::vector<Rigidbody*> s_pendingRigidbodies;
	static std::vector<Collider*> s_pendingColliders;

	// --- コールバックのインスタンス ---

	static inline SimulationEventCallback m_simulationEventCallback; // 物理エンジンのイベントコールバックを処理するインスタンス

	static inline FilterShader m_filterShader; // クエリフィルタコールバックを処理するインスタンス

	// --- PhysX関連のメンバ変数 ---
	static inline physx::PxDefaultAllocator			pxAllocator;
	static inline physx::PxDefaultErrorCallback		pxErrorCallback;
	static inline physx::PxFoundation*				pxFoundation = nullptr;
	static inline physx::PxPvd*						pxPvd = nullptr;
	static inline physx::PxPhysics*					pxPhysics = nullptr;
	static inline physx::PxDefaultCpuDispatcher*	pxDispatcher = nullptr;
	static inline physx::PxScene*					pxScene = nullptr;
	static inline physx::PxMaterial*				pxMaterial = nullptr;
	static inline physx::PxControllerManager*		pxControllerManager = nullptr;

};