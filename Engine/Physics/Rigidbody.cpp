#include "pch.h"
#include "Rigidbody.h"
#include "Physics.h"
#include "Collider.h"

REGISTER_COMPONENT(Rigidbody, "Physics")

void Rigidbody::Awake()
{
	// 物理エンジンに剛体を登録する処理をここに追加します。
	Physics::RegisterPendingRigidbody(this);
}

void Rigidbody::Register()
{
	// 物理エンジンに剛体を登録する処理をここに追加します。
	m_actorHandle = Physics::RegisterBody(GetTransform(), true);

	// キネマティック設定の適用
	Physics::SetKinematic(m_actorHandle, isKinematic);

	// 重力の使用設定の適用
	Physics::SetUseGravity(m_actorHandle, useGravity);

	// CCDの設定の適用
	Physics::SetUseCCD(m_actorHandle, useCCD);

	// スリープ閾値の設定の適用
	Physics::SetSleepThreshold(m_actorHandle, sleepThreshold);

	// 制約の設定の適用
	Physics::SetConstraints(m_actorHandle, physx::PxRigidDynamicLockFlags(static_cast<physx::PxU8>(constraints)));
}

void Rigidbody::PostColliderRegister()
{
	// コライダーの登録後に剛体を更新する処理をここに追加します。
	// 例えば、コライダーの形状やサイズに基づいて質量や慣性を再計算するなどの処理が考えられます。
	
	// 質量の設定の適用
	Physics::SetMass(m_actorHandle, mass);
}

void Rigidbody::OnDestroy()
{
	// 物理エンジンから剛体を削除する処理をここに追加します。
	Physics::UnregisterBody(GetTransform());
}

void Rigidbody::Finalize()
{
	Physics::UnregisterPendingRigidbody(this); // 登録保留リストから削除
	// 物理エンジンから剛体を削除する処理をここに追加します。
	Physics::RemoveActor(m_actorHandle);
	m_actorHandle = INVALID_ACTOR_HANDLE; // ハンドルを無効化
}

void Rigidbody::OnEnable()
{
	//// 物理エンジンに剛体を登録する処理をここに追加します。
	//m_actorHandle = Physics::RegisterBody(GetTransform(), true);

	//// キネマティック設定の適用
	//Physics::SetKinematic(m_actorHandle, isKinematic);

	//// 重力の使用設定の適用
	//Physics::SetUseGravity(m_actorHandle, useGravity);

	//// 質量の設定の適用
	//Physics::SetMass(m_actorHandle, mass);

	//// 制約の設定の適用
	//Physics::SetConstraints(m_actorHandle, physx::PxRigidDynamicLockFlags(static_cast<physx::PxU8>(constraints)));

	// Actorを有効化する処理をここに追加します。
	Physics::SetActorEnable(m_actorHandle, true);
}

void Rigidbody::OnDisable()
{
	//// 物理エンジンから剛体を削除する処理をここに追加します。
	//Physics::UnregisterBody(GetTransform());

	// Actorを無効化する処理をここに追加します。
	Physics::SetActorEnable(m_actorHandle, false);
}

// --- 物理エンジンに対する操作の実装 ---

void Rigidbody::AddForce(Vector3 force, ForceMode mode)
{
	if (isKinematic)
	{
		// キネマティックなオブジェクトには力を加えない
		Console::LogWarning(std::format("Cannot add force to a kinematic Rigidbody ({}).", GetOwner()->GetName()));
		return;
	}

	// 物理エンジンに力を加える処理をここに追加します。
	Physics::AddForce(m_actorHandle, force, static_cast<physx::PxForceMode::Enum>(mode));
}

void Rigidbody::AddForceAtPosition(Vector3 force, Vector3 position, ForceMode mode)
{
	// 物理エンジンに特定の位置に力を加える処理をここに追加します。
	Physics::AddForceAtPosition(m_actorHandle, force, position, static_cast<physx::PxForceMode::Enum>(mode));
}

void Rigidbody::AddTorque(Vector3 torque, ForceMode mode)
{
	// 物理エンジンにトルクを加える処理をここに追加します。
	Physics::AddTorque(m_actorHandle, torque, static_cast<physx::PxForceMode::Enum>(mode));
}

void Rigidbody::SetVelocity(Vector3 velocity)
{
	// 物理エンジンに速度を直接設定する処理をここに追加します。
	Physics::SetVelocity(m_actorHandle, velocity);
}

Vector3 Rigidbody::GetVelocity() const
{
	// 物理エンジンから速度を直接取得する処理をここに追加します。
	Vector3 velocity;
	Physics::GetVelocity(m_actorHandle, velocity);
	return velocity;
}

void Rigidbody::SetAngularVelocity(Vector3 angularVelocity)
{
	// 物理エンジンに角速度を直接設定する処理をここに追加します。
	Physics::SetAngularVelocity(m_actorHandle, angularVelocity);
}

Vector3 Rigidbody::GetAngularVelocity() const
{
	// 物理エンジンから角速度を直接取得する処理をここに追加します。
	Vector3 angularVelocity;
	Physics::GetAngularVelocity(m_actorHandle, angularVelocity);
	return angularVelocity;
}

void Rigidbody::SetMass(float mass)
{
	// 物理エンジンに質量を直接設定する処理をここに追加します。
	float clampedMass = (std::max)(mass, 0.0001f); // 質量が0以下にならないように最低値を設定
	this->mass = clampedMass; // 内部の質量プロパティも更新
	Physics::SetMass(m_actorHandle, clampedMass);
}

float Rigidbody::GetMass() const
{
	// 物理エンジンから質量を直接取得する処理をここに追加します。
	return mass; // 内部の質量プロパティを返す
}

void Rigidbody::SetInertiaTensor(const Vector3& inertiaTensor)
{
	// 物理エンジンに慣性テンソルを直接設定する処理をここに追加します。
	Physics::SetInertiaTensor(m_actorHandle, inertiaTensor);
}

void Rigidbody::SetKinematic(bool isKinematic)
{
	// 物理エンジンにキネマティック設定を直接設定する処理をここに追加します。
	this->isKinematic = isKinematic; // 内部のキネマティックプロパティも更新
	Physics::SetKinematic(m_actorHandle, isKinematic);
}

bool Rigidbody::IsKinematic() const
{
	// 物理エンジンからキネマティック設定を直接取得する処理をここに追加します。
	return isKinematic; // 内部のキネマティックプロパティを返す
}

void Rigidbody::SetKinematicTarget(const Vector3& pos, const Quaternion& rot)
{
	// 物理エンジンにキネマティックターゲットを直接設定する処理をここに追加します。
	Physics::SetKinematicTarget(m_actorHandle, pos, rot);
	// キネマティックターゲットを設定した後、Transformの位置と回転も更新しておきます。
	GetTransform()->SetWorldPosition(pos);
	GetTransform()->SetWorldRotation(rot);
}

void Rigidbody::SetGlobalPose(const Vector3& pos, const Quaternion& rot)
{
	// 物理エンジンにグローバルポーズを直接設定する処理をここに追加します。
	Physics::SetGlobalPose(m_actorHandle, pos, rot);
}

void Rigidbody::GetGlobalPose(Vector3& outPos, Quaternion& outRot)
{
	// 物理エンジンからグローバルポーズを直接取得する処理をここに追加します。
	Physics::GetGlobalPose(m_actorHandle, outPos, outRot);
}

void Rigidbody::WakeUp()
{
	// 物理エンジンの剛体を起こす処理をここに追加します。
	Physics::WakeUp(m_actorHandle);
}

void Rigidbody::PutToSleep()
{
	// 物理エンジンの剛体を寝かせる処理をここに追加します。
	Physics::PutToSleep(m_actorHandle);
}

bool Rigidbody::IsSleeping() const
{
	// 物理エンジンの剛体がスリープ状態かどうかを直接取得する処理をここに追加します。
	return Physics::IsSleeping(m_actorHandle);
}

void Rigidbody::SetUseGravity(bool useGravity)
{
	// 物理エンジンに重力の使用設定を直接設定する処理をここに追加します。
	this->useGravity = useGravity; // 内部の重力使用プロパティも更新
	Physics::SetUseGravity(m_actorHandle, useGravity);
}

void Rigidbody::SetUseCCD(bool useCCD)
{
	// 物理エンジンにCCDの使用設定を直接設定する処理をここに追加します。
	this->useCCD = useCCD; // 内部のCCD使用プロパティも更新
	Physics::SetUseCCD(m_actorHandle, useCCD);
}

void Rigidbody::SetConstraints(RigidbodyConstraints constraints)
{
	// 物理エンジンに移動や回転の制約を直接設定する処理をここに追加します。
	this->constraints = constraints; // 内部の制約プロパティも更新
	Physics::SetConstraints(m_actorHandle, physx::PxRigidDynamicLockFlags(static_cast<physx::PxU8>(constraints)));
}

RigidbodyConstraints Rigidbody::GetConstraints() const
{
	// 物理エンジンから移動や回転の制約を直接取得する処理をここに追加します。
	//physx::PxRigidDynamicLockFlags flags = Physics::GetConstraints(m_actorHandle);
	//return RigidbodyConstraints(static_cast<physx::PxU8>(flags));
	return constraints; // 内部の制約プロパティを返す
}

void Rigidbody::SetLinearDamping(float damping)
{
	// 物理エンジンに線形減衰を直接設定する処理をここに追加します。
	Physics::SetLinearDamping(m_actorHandle, damping);
}

float Rigidbody::GetLinearDamping() const
{
	// 物理エンジンから線形減衰を直接取得する処理をここに追加します。
	return Physics::GetLinearDamping(m_actorHandle);
}

void Rigidbody::SetLinearDrag(float drag)
{
	// 物理エンジンに線形ドラッグを直接設定する処理をここに追加します。
	Physics::SetLinearDrag(m_actorHandle, drag);
}

float Rigidbody::GetLinearDrag() const
{
	// 物理エンジンから線形ドラッグを直接取得する処理をここに追加します。
	return Physics::GetLinearDrag(m_actorHandle);
}

void Rigidbody::SetMaxLinearVelocity(float maxVelocity)
{
	// 物理エンジンに最大線形速度を直接設定する処理をここに追加します。
	Physics::SetMaxLinearVelocity(m_actorHandle, maxVelocity);
}

float Rigidbody::GetMaxLinearVelocity() const
{
	// 物理エンジンから最大線形速度を直接取得する処理をここに追加します。
	return Physics::GetMaxLinearVelocity(m_actorHandle);
}

void Rigidbody::SetAngularDamping(float damping)
{
	// 物理エンジンに角減衰を直接設定する処理をここに追加します。
	Physics::SetAngularDamping(m_actorHandle, damping);
}

float Rigidbody::GetAngularDamping() const
{
	// 物理エンジンから角減衰を直接取得する処理をここに追加します。
	return Physics::GetAngularDamping(m_actorHandle);
}

void Rigidbody::SetAngularDrag(float drag)
{
	// 物理エンジンに角ドラッグを直接設定する処理をここに追加します。
	Physics::SetAngularDrag(m_actorHandle, drag);
}

float Rigidbody::GetAngularDrag() const
{
	// 物理エンジンから角ドラッグを直接取得する処理をここに追加します。
	return Physics::GetAngularDrag(m_actorHandle);
}

void Rigidbody::SetMaxAngularVelocity(float maxVelocity)
{
	// 物理エンジンに最大角速度を直接設定する処理をここに追加します。
	Physics::SetMaxAngularVelocity(m_actorHandle, maxVelocity);
}

float Rigidbody::GetMaxAngularVelocity() const
{
	// 物理エンジンから最大角速度を直接取得する処理をここに追加します。
	return Physics::GetMaxAngularVelocity(m_actorHandle);
}

void Rigidbody::SetSleepThreshold(float threshold)
{
	// 物理エンジンにスリープ閾値を直接設定する処理をここに追加します。
	Physics::SetSleepThreshold(m_actorHandle, threshold);
}

float Rigidbody::GetSleepThreshold() const
{
	// 物理エンジンからスリープ閾値を直接取得する処理をここに追加します。
	return Physics::GetSleepThreshold(m_actorHandle);
}

// --- 更新処理の実装 ---

void Rigidbody::Update(float deltaTime)
{
	// PhysXを使用している場合は、物理エンジンの更新に任せるため、ここでは何もしません。
}

void Rigidbody::FixedUpdate(float fixedDeltaTime)
{
#ifdef USE_PHYSX
	// PhysXを使用している場合は、物理エンジンの更新に任せるため、ここでは何もしません。
#endif
}

void Rigidbody::LateUpdate(float deltaTime)
{
#ifdef USE_PHYSX
	// PhysXを使用している場合は、物理エンジンの更新に任せるため、ここでは何もしません。

	
#endif
}

#ifdef USE_IMGUI
void Rigidbody::DrawProperty(const PropertyDrawContext& context)
{

	Component::DrawProperty(context); // 基底クラスのプロパティ描画も呼び出す

	IMGUI_PROPERTY_BEGIN();
	//ImGui::Text("Actor Handle: %d", m_actorHandle);
	// 物理エンジンのプロパティを編集するGUI要素をここに追加します。

#if 0
	// 質量を編集するための入力フィールド
	bool isMassChanged = false;
	IMGUI_PROPERTY_FLOAT("mass", mass, isMassChanged, 0.1f, 0.0001f, FLT_MAX, "%.4f");
	//if (ImGui::InputFloat("mass", &mass, 0.1f))
	if (isMassChanged)
	{
		// 質量が変更されたときの処理をここに追加します。
		SetMass(mass);
	}

	// 重力の使用設定を編集するためのチェックボックス
	bool isUseGravityChanged = false;
	IMGUI_PROPERTY_BOOL("useGravity", useGravity, isUseGravityChanged);
	//if (ImGui::Checkbox("useGravity", &useGravity))
	if (isUseGravityChanged)
	{
		// 重力の使用設定が変更されたときの処理をここに追加します。
		SetUseGravity(useGravity);
	}

	// キネマティック設定を編集するためのチェックボックス
	bool isKinematicChanged = false;
	IMGUI_PROPERTY_BOOL("isKinematic", isKinematic, isKinematicChanged);
	//if (ImGui::Checkbox("isKinematic", &isKinematic))
	if (isKinematicChanged)
	{
		// キネマティック設定が変更されたときの処理をここに追加します。
		SetKinematic(isKinematic);
	}

	// CCDの使用設定を編集するためのチェックボックス
	bool isUseCCDChanged = false;
	IMGUI_PROPERTY_BOOL("useCCD", useCCD, isUseCCDChanged);
	//if (ImGui::Checkbox("useCCD", &useCCD))
	if (isUseCCDChanged)
	{
		// CCDの使用設定が変更されたときの処理をここに追加します。
		SetUseCCD(useCCD);
	}

	bool isSleepThresholdChanged = false;
	IMGUI_PROPERTY_FLOAT("sleepThreshold", sleepThreshold, isSleepThresholdChanged, 0.001f, 0.0f, FLT_MAX, "%.6f");
	//if (ImGui::InputFloat("sleepThreshold", &sleepThreshold, 0.001f))
	if (isSleepThresholdChanged)
	{
		// スリープ閾値が変更されたときの処理をここに追加します。
		SetSleepThreshold(sleepThreshold);
	}
#endif // 0


	// 移動や回転の制約を編集するためのGUI要素をここに追加します。
	RigidbodyConstraints newConstraints = constraints;
	bool isConstraintsChanged = false;
	IMGUI_PROPERTY("Constraints");
	if (ImGui::TreeNodeEx("##Constraints", ImGuiTreeNodeFlags_None))
	{
		// すべての制約を一括で設定するためのチェックボックス
		isConstraintsChanged |= ImGui::CheckboxFlags("Freeze All", (unsigned int*)&newConstraints, static_cast<unsigned int>(RigidbodyConstraints::FreezeAll));
		
		// PositionとRotationの制約を分けて表示するためのツリー構造
		{
			// Positionを一括で設定するためのチェックボックス
			isConstraintsChanged |= ImGui::CheckboxFlags("##Freeze Position", (unsigned int*)&newConstraints, static_cast<unsigned int>(RigidbodyConstraints::FreezePosition));
			ImGui::SameLine();
			// Positionでの制約
			if (ImGui::TreeNodeEx("Freeze Position", ImGuiTreeNodeFlags_None))
			{
				isConstraintsChanged |= ImGui::CheckboxFlags("X", (unsigned int*)&newConstraints, static_cast<unsigned int>(RigidbodyConstraints::FreezePositionX));
				ImGui::SameLine();
				isConstraintsChanged |= ImGui::CheckboxFlags("Y", (unsigned int*)&newConstraints, static_cast<unsigned int>(RigidbodyConstraints::FreezePositionY));
				ImGui::SameLine();
				isConstraintsChanged |= ImGui::CheckboxFlags("Z", (unsigned int*)&newConstraints, static_cast<unsigned int>(RigidbodyConstraints::FreezePositionZ));
				ImGui::TreePop();
			}

			// Rotationを一括で設定するためのチェックボックス
			isConstraintsChanged |= ImGui::CheckboxFlags("##Freeze Rotation", (unsigned int*)&newConstraints, static_cast<unsigned int>(RigidbodyConstraints::FreezeRotation));
			ImGui::SameLine();
			// Rotationでの制約
			if (ImGui::TreeNodeEx("Freeze Rotation", ImGuiTreeNodeFlags_None))
			{
				isConstraintsChanged |= ImGui::CheckboxFlags("X", (unsigned int*)&newConstraints, static_cast<unsigned int>(RigidbodyConstraints::FreezeRotationX));
				ImGui::SameLine();
				isConstraintsChanged |= ImGui::CheckboxFlags("Y", (unsigned int*)&newConstraints, static_cast<unsigned int>(RigidbodyConstraints::FreezeRotationY));
				ImGui::SameLine();
				isConstraintsChanged |= ImGui::CheckboxFlags("Z", (unsigned int*)&newConstraints, static_cast<unsigned int>(RigidbodyConstraints::FreezeRotationZ));
				ImGui::TreePop();
			}
		}

		if (isConstraintsChanged)
		{
			// 制約の設定が変更されたときの処理をここに追加します。
			IMGUI_PROPERTY_COMMAND_CUSTOM("constraints", newConstraints, constraints, std::to_string(static_cast<int>(newConstraints)), std::to_string(static_cast<int>(constraints)),
				[this](const RigidbodyConstraints& v) {
					SetConstraints(v); /* constraints プロパティを更新 */
				});
		}
		ImGui::TreePop();
	}

	IMGUI_PROPERTY_END();

	// --- テスト用のGUI要素 ---
	{
		ImGui::SeparatorText(reinterpret_cast<const char*>(u8"テスト用"));

		// テスト用の力を加えるボタン
		static Vector3 forceToAdd(0, 0, 10);

		ImGui::InputFloat3("Force to Add", &forceToAdd.x);

		if (ImGui::Button("Add Force"))
		{
			AddForce(forceToAdd, ForceMode::Impulse);
		}
	}

}
#endif // USE_IMGUI
