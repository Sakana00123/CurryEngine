#pragma once
#include "Component.h"
#include <DirectXMath.h>
#include "Reflection/TypeSerializerRegistry.h"

#include "Engine/Core/Math/Vector2.h"
#include "Engine/Core/Math/Vector3.h"
#include "Engine/Core/Math/Quaternion.h"

using namespace DirectX;

/**
 * @file
 * @brief 3D/2D 空間での位置・回転・スケールを管理するトランスフォーム。
 * @details ローカル/ワールド行列の更新、クォータニオン/オイラー角の相互変換、
 *          前/右/上方向ベクトルの取得、ワールド側の位置/回転/スケール設定などを提供します。
 */

//座標系と軸の設定
/**
 * @brief 座標系（左右手系、Up 軸）を表す列挙体。
 */
C_ENUM()
enum class CoordinateSystem
{
	LeftHand_YUp,//左手座標系、Y軸上
	LeftHand_ZUp,//左手座標系、Z軸上
	RightHand_YUp,//右手座標系、Y軸上
	RightHand_ZUp,//右手座標系、Z軸上
};
C_REGISTER_TYPE(CoordinateSystem);

//Transformコンポーネント
/**
 * @brief 位置・回転・スケールとローカル/ワールド行列を管理するコンポーネント。
 * @details クォータニオン/オイラー角の相互変換、各種ベクトル取得、ワールド側の設定関数を備えます。
 */
class Transform : public Component
{
	C_REFLECT(Transform)
public:
	/** @brief ローカル座標の位置。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetPosition"), CurryEngine::PropertyAttributes::Setter("SetPosition"))
	Vector3 position;
	/** @brief ローカル回転（クォータニオン）。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetRotation"), CurryEngine::PropertyAttributes::Setter("SetRotation"), CurryEngine::PropertyAttributes::CustomDrawer("Quaternion_Euler"))
	Quaternion rotation;
	/** @brief ローカルスケール。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetScale"), CurryEngine::PropertyAttributes::Setter("SetScale"))
	Vector3 scale;

	/** @brief ローカル回転（オイラー角、度）。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly)
	Vector3 m_eulerAngles;

	/** @brief 座標系の設定。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Range(0, 3), CurryEngine::PropertyAttributes::HideInInspector)
	CoordinateSystem coordinateSystem = CoordinateSystem::LeftHand_YUp;
protected:
	bool m_eulerDirty = true; // オイラー角がローカル回転と同期しているか
	XMFLOAT4X4 local;
	XMFLOAT4X4 world;
	bool needsUpdate;//Uodateが呼び出されてから値が変更されたかどうか
	bool changedThisFrame;// このフレームで値が変更されたかどうか
	Vector3 worldPosition;
	Quaternion worldRotation;
	Vector3 worldScale;
private:
#ifdef USE_IMGUI
	bool enableScaleLink = false; // スケールのリンクを有効にするか。true の場合、エディタ上で編集するとき、X/Y/Z すべてのスケールを同時に変更します。
	Vector3 lastValidScale = Vector3(1, 1, 1);// 最後に保存された有効なスケールの値(0を含まない)。スケールのリンクが有効な場合、これを基準にスケールを変更します。
#endif // _DEBUG
public:
	/**
	 * @brief 既定コンストラクタ。初期値を設定します。
	 * @details ローカル/ワールド行列を単位行列、スケールを 1、座標系を LeftHand_YUp に設定します。
	 */
	Transform() :
		position(0, 0, 0),
		rotation(0, 0, 0, 1),
		//eulerAngles(0, 0, 0),
		scale(1, 1, 1),
		local(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1),
		world(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1),
		needsUpdate(true),
		changedThisFrame(false),
		worldPosition(0, 0, 0),
		worldRotation(0, 0, 0, 1),
		worldScale(1, 1, 1)
	{}
	virtual ~Transform() override = default;

	/** @brief 終了処理として呼び出されます。*/
	void OnDestroy() override;

	/** @brief 最初の初期化処理として呼び出されます。*/
	void Awake() override;

	/** @brief `XMVECTOR` をクォータニオンに変換。*/
	static Quaternion XMVectorToQuaternion(const XMVECTOR& vector);
	/** @brief 任意軸回りの回転を表すクォータニオンを生成。*/
	static Quaternion QuaternionRotationAxis(const XMFLOAT3& axis, float angle);
	/** @brief クォータニオンを指定軸の回転角に変換。*/
	static float QuaternionToAxisAngle(const XMFLOAT3& axis, const Quaternion& q);
	/** @brief クォータニオンを `XMVECTOR` に変換。*/
	static XMVECTOR QuaternionToXMVector(const Quaternion& q);
	/** @brief クォータニオン同士の乗算。*/
	static Quaternion QuaternionMultiply(const Quaternion& q1, const Quaternion& q2);
	/** @brief ベクトルの LookAt を表すクォータニオンを計算。*/
	static XMVECTOR QuaternionLookAt(const XMVECTOR& Original, const XMVECTOR& Target);
	
	/** @brief 今回のフレームで値が変更されたかを返します。*/
	bool IsChangedThisFrame() const;

	/** @brief ローカル位置を取得。*/
	C_FUNCTION()
	Vector3 GetPosition();
	/** @brief ローカル回転（クォータニオン）を取得。*/
	C_FUNCTION()
	Quaternion GetRotation();
	/** @brief ローカル回転（オイラー角）を取得。*/
	C_FUNCTION()
	Vector3 GetEulerAngles();
	/** @brief ローカルスケールを取得。*/
	C_FUNCTION()
	Vector3 GetScale();

	/** @brief ローカル位置を設定。*/
	C_FUNCTION()
	void SetPosition(const Vector3& position);
	/** @brief ローカル位置に加算（移動）。*/
	void Translate(const Vector3& translate);
	/** @brief ローカル回転（クォータニオン）を設定。*/
	C_FUNCTION()
	void SetRotation(const Quaternion& rotation);
	/** @brief ローカル回転（オイラー角）を設定。*/
	C_FUNCTION()
	void SetRotation(const Vector3& eulerAngles);
	/** @brief ローカル回転に乗算（クォータニオン）。*/
	C_FUNCTION()
	void Rotate(const Quaternion& rotate);
	/** @brief ローカル回転に加算（オイラー角）。*/
	C_FUNCTION()
	void Rotate(const Vector3& eulerAngles);

	/** @brief ローカルスケールを設定。*/
	C_FUNCTION()
	void SetScale(const Vector3& scale);
	/** @brief ローカルスケールを等倍で設定。*/
	C_FUNCTION()
	void SetScale(float scale);
	/** @brief ローカルスケールを乗算。*/
	C_FUNCTION()
	void Scaling(const Vector3& scaling);
	/** @brief ローカルスケールを等倍で乗算。*/
	C_FUNCTION()
	void Scaling(float scaling);

	/** @brief 変更フラグを立て、再計算を要求します。*/
	void MarkNeedsUpdate();

	/** @brief フレーム更新時に行列を更新します。*/
	void Update(float deltaTime) override;

	/** @brief フレーム更新後に変更フラグをリセットしたり、変更があった場合、変更を通知します。*/
	void LateUpdate(float deltaTime) override;

	/** @brief 変更があればローカル/ワールド行列を更新します。*/
	void UpdateTransform();

	/** @brief ローカル行列を取得します。*/
	const XMFLOAT4X4& GetLocal();

	/** @brief ワールド行列を取得します。*/
	const XMFLOAT4X4& GetWorld();

	/** @brief ワールド位置を取得。*/
	const Vector3& GetWorldPosition();
	/** @brief ワールド回転（クォータニオン）を取得。*/
	const Quaternion& GetWorldRotation();
	/** @brief ワールドスケールを取得。*/
	const Vector3& GetWorldScale();

	/** @brief 前方向ベクトル（ワールド）を取得。*/
	Vector3 GetForward();

	/** @brief 右方向ベクトル（ワールド）を取得。*/
	Vector3 GetRight();

	/** @brief 上方向ベクトル（ワールド）を取得。*/
	Vector3 GetUp();
	
	/** @brief ワールド位置を設定。*/
	void SetWorldPosition(const Vector3& worldPos);
	/** @brief ワールドスケールを設定。*/
	void SetWorldScale(const Vector3& worldScale);
	/** @brief ワールドスケールを等倍で設定。*/
	void SetWorldScale(float worldScale);

	/** @brief ワールド回転（クォータニオン）を設定。*/
	void SetWorldRotation(const Quaternion& worldRotation);
	/** @brief ワールド回転（オイラー角）を設定。*/
	void SetWorldRotation(const Vector3& worldEuler);

#ifdef USE_IMGUI
	/** @brief インスペクタ用プロパティ表示。*/
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

	/** @brief シリアライズ。*/
	json Serialize() const override;
	/** @brief デシリアライズ。*/
	void Deserialize(const json& j) override;
};