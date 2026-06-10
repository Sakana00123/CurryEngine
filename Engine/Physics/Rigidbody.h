#pragma once
#include "Engine/Core/Component.h"
#include "Physics.h"
//#include "Engine/Core/Math/Vector3.h"
//#include "Engine/Core/Math/Quaternion.h"
//#include "Engine/Core/Reflection/TypeSerializerRegistry.h"

C_ENUM()
enum class ForceMode
{
	Force,          // 継続的な力（質量に影響される）
	Impulse,        // 瞬間的な衝撃（質量に影響される）
	VelocityChange, // 質量に関係なく即座に速度を変化させる
	Acceleration    // 質量に関係なく継続的な加速度を与える
};
C_REGISTER_TYPE(ForceMode)

C_ENUM()
enum class RigidbodyInterpolation
{
	None,       // 補間なし
	Interpolate, // 前フレームと現在フレームの間で補間
	Extrapolate  // 現在フレームと次フレームの間で補間
};
C_REGISTER_TYPE(RigidbodyInterpolation)

C_ENUM()
enum class RigidbodyCollisionDetectionMode
{
	Discrete,   // 離散的な衝突検出（高速だが高速移動オブジェクトのすり抜けが発生する可能性あり）
	Continuous, // 連続的な衝突検出（高速移動オブジェクトのすり抜けを防止）
	ContinuousDynamic, // 動的オブジェクト同士の連続的な衝突検出
	ContinuousSpeculative // 予測的な連続衝突検出（高速移動オブジェクトのすり抜けをさらに減少させる）
};
C_REGISTER_TYPE(RigidbodyCollisionDetectionMode)

C_ENUM()
enum class RigidbodySleepMode
{
	NeverSleep, // 常にアクティブ
	StartAwake, // シーン開始時はアクティブ、条件を満たすとスリープ
	StartSleep, // シーン開始時はスリープ、条件を満たすとアクティブ
	CustomSleepTimeout // カスタムのスリープタイムアウトを使用
};
C_REGISTER_TYPE(RigidbodySleepMode)

C_ENUM()
enum class RigidbodyConstraints
{
	None = 0,
	FreezePositionX = 1,	// 1 << 0
	FreezePositionY = 2,	// 1 << 1
	FreezePositionZ = 4,	// 1 << 2
	FreezeRotationX = 8,	// 1 << 3
	FreezeRotationY = 16,	// 1 << 4
	FreezeRotationZ = 32,	// 1 << 5
	FreezePosition = 7,		// FreezePositionX | FreezePositionY | FreezePositionZ
	FreezeRotation = 56,	// FreezeRotationX | FreezeRotationY | FreezeRotationZ
	FreezeAll = 63			// FreezePositionX | FreezePositionY | FreezePositionZ | FreezeRotationX | FreezeRotationY | FreezeRotationZ
};
C_REGISTER_TYPE(RigidbodyConstraints)


class Rigidbody : public Component
{
	C_REFLECT(Rigidbody)
public:
	//Vector3 velocity;
	Vector3 acceleration;
	//Vector3 force;
	
	
	C_PROPERTY()
	float mass = 1.0f; // 質量
	C_PROPERTY()
	bool isKinematic = false; // キネマティックかどうか（物理エンジンの力を受けない）
	C_PROPERTY()
	bool useGravity = true; // 重力の影響を受けるかどうか
	C_PROPERTY()
	bool useCCD = false; // 連続的な衝突検出を使用するかどうか
	
	//bool useDefaultPhysicsMaterial = true; // デフォルトの物理マテリアルを使用するかどうか

	C_PROPERTY()
	RigidbodyConstraints constraints = RigidbodyConstraints::None; // 移動や回転の制約

	C_PROPERTY()
	float sleepThreshold = 0.005f; // スリープ状態になる速度の閾値


	//const Vector3 gravity = { 0.0f, -9.8f, 0.0f };
	const Vector3 gravity = { 0.0f, -360.0f, 0.0f };
public:
	// --- 物理エンジンに対する操作 ---

	// これらの関数は、物理エンジンに対して力や速度を直接操作するためのインターフェースです。

	/**
	 * @brief 剛体に力を加えます。
	 * @param force 加える力のベクトル。
	 * @param mode 力の加え方を指定するForceMode。デフォルトはForceで、継続的な力を加えるモードです。
	 */
	C_FUNCTION()
	void AddForce(Vector3 force, ForceMode mode);

	/**
	 * @brief 剛体の特定の位置に力を加えます。
	 * @param force 加える力のベクトル。
	 * @param position 力を加える位置のワールド座標。
	 * @param mode 力の加え方を指定するForceMode。デフォルトはForceで、継続的な力を加えるモードです。
	 */
	C_FUNCTION()
	void AddForceAtPosition(Vector3 force, Vector3 position, ForceMode mode);

	/**
	 * @brief 剛体にトルクを加えます。
	 * @param torque 加えるトルクのベクトル。
	 * @param mode トルクの加え方を指定するForceMode。デフォルトはForceで、継続的なトルクを加えるモードです。
	 */
	C_FUNCTION()
	void AddTorque(Vector3 torque, ForceMode mode);

	/**
	 * @brief 剛体の速度を直接設定します。
	 * @param velocity 設定する速度のベクトル。
	 */
	C_FUNCTION()
	void SetVelocity(Vector3 velocity);

	/**
	 * @brief 剛体の現在の速度を取得します。
	 * @return 現在の速度のベクトル。
	 */
	C_FUNCTION()
	Vector3 GetVelocity() const;

	/**
	 * @brief 剛体の角速度を直接設定します。
	 * @param angularVelocity 設定する角速度のベクトル。
	 */
	C_FUNCTION()
	void SetAngularVelocity(Vector3 angularVelocity);

	/**
	 * @brief 剛体の現在の角速度を取得します。
	 * @return 現在の角速度のベクトル。
	 */
	C_FUNCTION()
	Vector3 GetAngularVelocity() const;

	/**
	 * @brief 剛体の質量を設定します。
	 * @param mass 設定する質量の値。
	 */
	void SetMass(float mass);

	/**
	 * @brief 剛体の現在の質量を取得します。
	 * @return 現在の質量の値。
	 */
	float GetMass() const;

	/**
	 * @brief 剛体の慣性テンソルを設定します。
	 * @param inertiaTensor 設定する慣性テンソルのベクトル。通常は各軸に対する慣性モーメントを表します。
	 */
	void SetInertiaTensor(const Vector3& inertiaTensor);

	/**
	 * @brief キネマティック設定を行います。キネマティックなオブジェクトは物理シミュレーションの影響を受けず、直接位置や回転を設定できます。
	 * @param isKinematic キネマティックにするかどうかのフラグ。trueの場合はキネマティック、falseの場合は物理シミュレーションの影響を受けるようになります。
	 */
	void SetKinematic(bool isKinematic);

	/**
	 * @brief 剛体がキネマティックかどうかを取得します。
	 * @return trueの場合はキネマティック、falseの場合は物理シミュレーションの影響を受けるようになります。
	 */
	bool IsKinematic() const;

	/**
	 * @brief 剛体の位置を直接設定します（キネマティックなオブジェクトに対して）。
	 * @param pos 設定する位置のベクトル。
	 * @param rot 設定する回転のクォータニオン。
	 */
	void SetKinematicTarget(const Vector3& pos, const Quaternion& rot);

	/**
	 * @brief 剛体のグローバルポーズを直接設定します。
	 * @param pos 設定するグローバル位置のベクトル。
	 * @param rot 設定するグローバル回転のクォータニオン。
	 */
	void SetGlobalPose(const Vector3& pos, const Quaternion& rot);

	/**
	 * @brief 剛体のグローバルポーズを取得します。
	 * @param outPos グローバル位置を格納するためのVector3参照。
	 * @param outRot グローバル回転を格納するためのQuaternion参照。
	 */
	void GetGlobalPose(Vector3& outPos, Quaternion& outRot);

	/**
	 * @brief 剛体をアクティブな状態にします。アクティブな剛体は物理シミュレーションの影響を受け、動作します。
	 */
	C_FUNCTION()
	void WakeUp();

	/**
	 * @brief 剛体をスリープ状態にします。スリープ状態の剛体は物理シミュレーションの影響を受けず、計算コストを削減できます。
	 */
	C_FUNCTION()
	void PutToSleep();

	/**
	 * @brief 剛体がスリープ状態かどうかを取得します。
	 * @return trueの場合はスリープ状態、falseの場合はアクティブな状態です。
	 */
	C_FUNCTION()
	bool IsSleeping() const;

	/**
	 * @brief 剛体の重力の影響を設定します。
	 * @param useGravity 重力の影響を受けるかどうかのフラグ。trueの場合は重力の影響を受け、falseの場合は重力の影響を受けません。
	 */
	void SetUseGravity(bool useGravity);

	/**
	 * @brief 剛体の連続的な衝突検出の使用を設定します。
	 * @param useCCD 連続的な衝突検出を使用するかどうかのフラグ。trueの場合は連続的な衝突検出を使用し、falseの場合は使用しません。
	 */
	void SetUseCCD(bool useCCD);

	/**
	 * @brief 剛体の移動や回転の制約を設定します。
	 * @param constraints 設定するRigidbodyConstraintsの値。複数の制約を組み合わせることができます。
	 */
	void SetConstraints(RigidbodyConstraints constraints);

	/**
	 * @brief 剛体の現在の移動や回転の制約を取得します。
	 * @return 現在設定されているRigidbodyConstraintsの値。
	 */
	RigidbodyConstraints GetConstraints() const;

	/**
	 * @brief 剛体の線形減衰を設定します。
	 * @param damping 設定する線形減衰の値。通常は0以上の値を指定します。
	 */
	void SetLinearDamping(float damping);

	/**
	 * @brief 剛体の現在の線形減衰を取得します。
	 * @return 現在設定されている線形減衰の値。
	 */
	float GetLinearDamping() const;

	/**
	 * @brief 剛体の線形抵抗を設定します。
	 * @param linearDrag 設定する線形抵抗の値。通常は0以上の値を指定します。
	 */
	void SetLinearDrag(float linearDrag);

	/**
	 * @brief 剛体の現在の線形抵抗を取得します。
	 * @return 現在設定されている線形抵抗の値。
	 */
	float GetLinearDrag() const;

	/**
	 * @brief 剛体の最大線形速度を設定します。
	 * @param maxLinearVelocity 設定する最大線形速度の値。通常は0以上の値を指定します。
	 */
	void SetMaxLinearVelocity(float maxLinearVelocity);

	/**
	 * @brief 剛体の現在の最大線形速度を取得します。
	 * @return 現在設定されている最大線形速度の値。
	 */
	float GetMaxLinearVelocity() const;

	/**
	 * @brief 剛体の角減衰を設定します。
	 * @param damping 設定する角減衰の値。通常は0以上の値を指定します。
	 */
	void SetAngularDamping(float damping);
	
	/**
	 * @brief 剛体の現在の角減衰を取得します。
	 * @return 現在設定されている角減衰の値。
	 */
	float GetAngularDamping() const;

	/**
	 * @brief 剛体の角抵抗を設定します。
	 * @param angularDrag 設定する角抵抗の値。通常は0以上の値を指定します。
	 */
	void SetAngularDrag(float angularDrag);

	/**
	 * @brief 剛体の現在の角抵抗を取得します。
	 * @return 現在設定されている角抵抗の値。
	 */
	float GetAngularDrag() const;

	/**
	 * @brief 剛体の最大角速度を設定します。
	 * @param maxAngularVelocity 設定する最大角速度の値。通常は0以上の値を指定します。
	 */
	void SetMaxAngularVelocity(float maxAngularVelocity);

	/**
	 * @brief 剛体の現在の最大角速度を取得します。
	 * @return 現在設定されている最大角速度の値。
	 */
	float GetMaxAngularVelocity() const;

	/**
	 * @brief 剛体のスリープ閾値を設定します。
	 * @param sleepThreshold 設定するスリープ閾値の値。
	 */
	void SetSleepThreshold(float sleepThreshold);

	/**
	 * @brief 剛体の現在のスリープ閾値を取得します。
	 * @return 現在設定されているスリープ閾値の値。
	 */
	float GetSleepThreshold() const;

public:
	Rigidbody() = default;
	virtual ~Rigidbody() override = default;

	void Awake() override;

	void Register();
	void PostColliderRegister();

	void OnDestroy() override;

	void Finalize() override;

	void OnEnable() override;

	void OnDisable() override;

	void Update(float deltaTime) override;

	void FixedUpdate(float fixedDeltaTime) override;

	void LateUpdate(float deltaTime) override;

	void OnGround();

	void OnTriggerEnter(CollisionInfo info) {
		
	}

#ifdef USE_IMGUI
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


	bool isGround = false;

private:
	ActorHandle m_actorHandle = INVALID_ACTOR_HANDLE;
	MaterialHandle m_materialHandle = DEFAULT_MATERIAL_HANDLE;

};