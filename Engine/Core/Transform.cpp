#include "pch.h"
#include "Transform.h"
#include "GameObject.h"
#ifdef USE_IMGUI
#include <ImGuizmo.h>
#include "Engine/Rendering/Pipeline/Graphics.h"
#endif // USE_IMGUI

#include "Engine/Physics/Physics.h"


REGISTER_COMPONENT_WITH_ATTRIBUTES(Transform, "Core", ComponentAttributes::DisallowMultiple | ComponentAttributes::HideInAddComponentMenu | ComponentAttributes::ExecuteInEditMode, {})


void Transform::OnDestroy()
{
	// 念のため、Physicsに、このTransformが破棄されたことを通知しておく
	Physics::OnTrnasformDestroyed(this);
}

void Transform::Awake()
{
	priority = 0;// Transformは常に最初に更新されるようにする
}

Quaternion Transform::XMVectorToQuaternion(const XMVECTOR& vector)
{ 
	Quaternion q;
	XMStoreFloat4(&q, vector); 
	return q; 
}

Quaternion Transform::QuaternionRotationAxis(const XMFLOAT3& axis, float angle)
{
	Quaternion q;
	XMStoreFloat4(&q, XMQuaternionRotationAxis(XMLoadFloat3(&axis), (angle > XM_2PI) ? angle - XM_2PI : angle));
	return q;
}

float Transform::QuaternionToAxisAngle(const XMFLOAT3& axis, const Quaternion& q)
{
	float angle;
	XMVECTOR Axis = XMLoadFloat3(&axis);
	XMQuaternionToAxisAngle(&Axis, &angle, XMLoadFloat4(&q));
	return angle; 
}

XMVECTOR Transform::QuaternionToXMVector(const Quaternion& q)
{
	return XMLoadFloat4(&q);
}

Quaternion Transform::QuaternionMultiply(const Quaternion& q1, const Quaternion& q2)
{
	Quaternion q; 
	XMStoreFloat4(&q, XMQuaternionMultiply(XMLoadFloat4(&q1), XMLoadFloat4(&q2)));
	return q; 
}

XMVECTOR Transform::QuaternionLookAt(const XMVECTOR& Original, const XMVECTOR& Target)
{
	XMVECTOR Forward = XMVector3Normalize(XMVectorSubtract(Target, Original));
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Right = XMVector3Normalize(XMVector3Cross(Up, Forward));
	Up = XMVector3Cross(Forward, Right);
	XMMATRIX Rotation = XMMatrixIdentity();
	Rotation.r[0] = Right; Rotation.r[1] = Up; Rotation.r[2] = Forward;
	XMVECTOR Quaternion = XMQuaternionRotationMatrix(Rotation);
	return Quaternion;
}

Vector3 Transform::QuaternionToEuler(const Quaternion& rotation)
{
	// クォータニオンを回転行列に変換
	XMFLOAT4X4 rotationMatrix;
	XMStoreFloat4x4(&rotationMatrix, XMMatrixRotationQuaternion(XMLoadFloat4(&rotation)));
	//ジンバルロック判定
	float sx = rotationMatrix.m[2][1];
	bool unlocked = std::abs(sx) < 0.99999f;
	// オイラー角を計算
	Vector3 eulerAngles{};
	eulerAngles.x = unlocked ? asinf(sx) : atan2f(rotationMatrix.m[2][1], rotationMatrix.m[2][2]);
	eulerAngles.y = unlocked ? atan2f(-rotationMatrix.m[2][0], rotationMatrix.m[2][2]) : 0;
	eulerAngles.z = unlocked ? atan2f(-rotationMatrix.m[0][1], rotationMatrix.m[1][1]) : atan2f(rotationMatrix.m[1][0], rotationMatrix.m[0][0]);
	// ラジアンから度に変換
	eulerAngles.x = -XMConvertToDegrees(eulerAngles.x);
	eulerAngles.y = -XMConvertToDegrees(eulerAngles.y);
	eulerAngles.z = -XMConvertToDegrees(eulerAngles.z);

	return eulerAngles;
}

Quaternion Transform::EulerToQuaternion(const Vector3& eulerAngles)
{
	Quaternion q;
	XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(eulerAngles.x), XMConvertToRadians(eulerAngles.y), XMConvertToRadians(eulerAngles.z)));
	return q;
}

bool Transform::IsChangedThisFrame() const
{
	return changedThisFrame;
}

Vector3 Transform::GetPosition()
{
	return position;
}

Quaternion Transform::GetRotation()
{
	return rotation;
}

Vector3 Transform::GetEulerAngles()
{
	if (m_eulerDirty) {
		m_eulerAngles = QuaternionToEuler(rotation); // ローカル回転からオイラー角を計算して保存
		m_eulerDirty = false; // オイラー角がローカル回転と同期している状態
	}
	return m_eulerAngles;
}

Vector3 Transform::GetScale()
{
	return scale;
}

void Transform::SetPosition(const Vector3& position)
{
	this->position = position;
	MarkNeedsUpdate();
}

void Transform::Translate(const Vector3& translate)
{
	position.x += translate.x;
	position.y += translate.y;
	position.z += translate.z;
	MarkNeedsUpdate();
}

void Transform::SetRotation(const Quaternion& rotation)
{
	this->rotation = rotation;
	//this->eulerAngles = QuaternionToEuler(rotation);
	m_eulerDirty = true; // ローカル回転がオイラー角と同期していない状態
	MarkNeedsUpdate();
}

void Transform::SetRotation(const Vector3& eulerAngles)
{
	this->m_eulerAngles = eulerAngles; // オイラー角をローカル回転に変換して保存
	this->rotation = EulerToQuaternion(eulerAngles); // ローカル回転をオイラー角に変換して保存
	m_eulerDirty = false; // オイラー角がローカル回転と同期している状態
	MarkNeedsUpdate();
}
void Transform::Rotate(const Quaternion& rotate)
{
	SetRotation(QuaternionMultiply(rotation, rotate));
}
void Transform::Rotate(const Vector3& eulerAngles)
{
	Rotate(EulerToQuaternion(eulerAngles));
}

void Transform::SetScale(const Vector3& scale)
{
	this->scale = scale;
	MarkNeedsUpdate();
}

void Transform::SetScale(float scale)
{
	this->scale.x = this->scale.y = this->scale.z = scale;
	MarkNeedsUpdate();
}

void Transform::Scaling(const Vector3& scaling)
{
	scale *= scaling;
	MarkNeedsUpdate();
}

void Transform::Scaling(float scaling)
{
	scale *= scaling;
	MarkNeedsUpdate();
}

void Transform::MarkNeedsUpdate()
{
	needsUpdate = true;
	changedThisFrame = true;

	// 子供のTransformも更新が必要なため、再帰的に呼び出す
	for (auto child : GetOwner()->children) {
		if (child && child->transform) {
			child->transform->MarkNeedsUpdate();
		}
	}
}

void Transform::Update(float deltaTime)
{
	UpdateTransform();
}

void Transform::LateUpdate(float deltaTime)
{
	if (changedThisFrame)
	{
		// Transformが変更されたことを通知
		for (auto& component : GetOwner()->GetAllComponents())
		{
			if (component)
			{
				component->OnTransformChanged();
			}
		}
		changedThisFrame = false;// 通知が完了したのでフラグをリセット
	}
}

void Transform::UpdateTransform()
{
	if (!needsUpdate) return;
	//座標系と軸の変換行列
	const DirectX::XMFLOAT4X4 coordinateSystemTransforms[]{
		{ 1,0,0,0,0, 1,0,0,0,0,1,0,0,0,0,1}, //0:LHS Y-UP
		{ 1,0,0,0,0, 1,0,0,0,0,1,0,0,0,0,1}, //0:LHS Z-UP
		{-1,0,0,0,0, 1,0,0,0,0,1,0,0,0,0,1}, //0:RHS Y-UP
		{-1,0,0,0,0,-1,0,0,0,0,1,0,0,0,0,1}, //0:RHS Z-UP
	};
	XMMATRIX C{ XMLoadFloat4x4(&coordinateSystemTransforms[static_cast<int>(coordinateSystem)]) };
	XMMATRIX S{ XMMatrixScaling(scale.x, scale.y, scale.z) };
	XMMATRIX R{ XMMatrixRotationQuaternion(QuaternionToXMVector(rotation)) };
	XMMATRIX T{ XMMatrixTranslation(position.x, position.y, position.z) };
	XMMATRIX L{ C * S * R * T };
	XMStoreFloat4x4(&local, L);//ローカル座標を保存
	XMMATRIX W = (GetOwner()->parent) ? L * XMLoadFloat4x4(&GetOwner()->parent->transform->GetWorld()) : L;
	XMStoreFloat4x4(&world, W);//ワールド座標を保存
	XMVECTOR Scale, Rotation, Position;//ワールド座標を保存
	if (XMMatrixDecompose(&Scale, &Rotation, &Position, W)) {
		XMFLOAT3 s, p;
		XMFLOAT4 r;
		XMStoreFloat3(&s, Scale);
		XMStoreFloat4(&r, Rotation);
		XMStoreFloat3(&p, Position);
		worldScale = Vector3(s);
		worldRotation = r;
		worldPosition = Vector3(p);
	}

	// 更新が完了したのでフラグをリセット
	needsUpdate = false;
}

const XMFLOAT4X4& Transform::GetLocal()
{
	UpdateTransform();
	return local;
}

const XMFLOAT4X4& Transform::GetWorld()
{
	UpdateTransform();
	return world;
}

const Vector3& Transform::GetWorldPosition()
{
	UpdateTransform();
	return worldPosition;
}

const Quaternion& Transform::GetWorldRotation()
{
	UpdateTransform();
	return worldRotation;
}

const Vector3& Transform::GetWorldScale()
{
	UpdateTransform();
	return worldScale;
}

void Transform::SetWorldPosition(const Vector3& worldPos)
{
	if (GetOwner()->parent) {
		if (GetOwner()->parent->transform) {
			//親のワールド行列の逆行列を取得
			XMMATRIX InverseWorld = XMMatrixInverse(nullptr, XMLoadFloat4x4(&GetOwner()->parent->transform->GetWorld()));
			//worldPosを親のローカル座標に変換
			XMFLOAT3 worldPosFloat3 = worldPos;
			XMVECTOR worldPosVec = XMLoadFloat3(&worldPosFloat3);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&position), XMVector3TransformCoord(worldPosVec, InverseWorld));
		}
	}
	else {
		position = worldPos;
	}
	MarkNeedsUpdate();
}

void Transform::SetWorldScale(const Vector3& worldScale)
{
	if (GetOwner()->parent) {
		if (GetOwner()->parent->transform) {
			scale = worldScale / GetOwner()->parent->transform->GetWorldScale();
		}
	}
	else {
		scale = worldScale;
	}
	MarkNeedsUpdate();
}

void Transform::SetWorldScale(float worldScale)
{
	SetWorldScale({ worldScale, worldScale, worldScale });
}

void Transform::SetWorldRotation(const Quaternion& worldRotation)
{
	if (GetOwner()->parent) {
		if (GetOwner()->parent->transform) {
			//親のワールド回転を取得
			XMVECTOR wRot = XMLoadFloat4(&GetOwner()->parent->transform->GetWorldRotation());
			//ローカル回転を算出
			Quaternion q;
			XMStoreFloat4(&q, XMQuaternionMultiply(
				XMQuaternionInverse(wRot),
				XMLoadFloat4(&worldRotation)
			));
			SetRotation(q);
		}
	}
	else {
		SetRotation(worldRotation);
	}
}

void Transform::SetWorldRotation(const Vector3& worldEuler)
{
	if (GetOwner()->parent) {
		SetWorldRotation(EulerToQuaternion(worldEuler));
	}
	else {
		SetRotation(worldEuler);
	}
}

Vector3 Transform::GetForward()
{
	UpdateTransform();
	//+Z方向ベクトルを回転させる
	XMVECTOR forward = XMVectorSet(0, 0, 1, 0);
	Quaternion worldRot = GetWorldRotation();
	forward = XMVector3Rotate(forward, QuaternionToXMVector(worldRot));
	Vector3 f;
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&f), forward);
	return f;
}

Vector3 Transform::GetRight()
{
	UpdateTransform();
	//X+方向ベクトルを回転させる
	XMVECTOR right = XMVectorSet(1, 0, 0, 0);
	Quaternion worldRot = GetWorldRotation();
	right = XMVector3Rotate(right, QuaternionToXMVector(worldRot));
	Vector3 r;
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&r), right);
	return r;
}

Vector3 Transform::GetUp()
{
	UpdateTransform();
	//Y+方向ベクトルを回転させる
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	Quaternion worldRot = GetWorldRotation();
	up = XMVector3Rotate(up, QuaternionToXMVector(worldRot));
	Vector3 u;
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&u), up);
	return u;
}

void Transform::DrawProperty()
{
#ifdef USE_IMGUI
	IMGUI_PROPERTY_BEGIN();

	// 位置の編集
	{
		static Vector3 prevPosition;
		IMGUI_PROPERTY("Position");
		if (ImGui::DragFloat3("##Position", &position.x)) {
			MarkNeedsUpdate();
		}

		if (ImGui::IsItemActivated()) // 編集開始時に現在の値を保存
		{
			prevPosition = position;
		}
		if (ImGui::IsItemDeactivatedAfterEdit()) // 編集終了時にコマンドを発行
		{
			Vector3 newPosition = position;
			if (newPosition.x != prevPosition.x || newPosition.y != prevPosition.y || newPosition.z != prevPosition.z) {
				std::string newPositionStr = "(" + std::to_string(newPosition.x) + ", " + std::to_string(newPosition.y) + ", " + std::to_string(newPosition.z) + ")";
				std::string prevPositionStr = "(" + std::to_string(prevPosition.x) + ", " + std::to_string(prevPosition.y) + ", " + std::to_string(prevPosition.z) + ")";
				IMGUI_PROPERTY_COMMAND_CUSTOM("position", newPosition, prevPosition, newPositionStr, prevPositionStr, [this](const Vector3& value) {
					SetPosition(value);
					});
			}
			prevPosition = newPosition;
		}
	}

	static Vector3 editorEuler;
	static Vector3 prevEuler;
	static bool isEditing = false;

	if (!isEditing)
	{
		// 編集中でなければ現在の回転を取得
		editorEuler = GetEulerAngles();
		// 前回の値を更新
		prevEuler = editorEuler;
	}

#if 0
	IMGUI_PROPERTY("Rotation");
	if (ImGui::DragFloat3("##Rotation", &editorEuler.x))
	{
		// 回転が変更された場合、差分を計算してクォータニオンに反映
		Vector3 delta = editorEuler - prevEuler;
		Quaternion deltaQuat = EulerToQuaternion(delta);
		rotation = QuaternionMultiply(deltaQuat, rotation);
		prevEuler = editorEuler;
		MarkNeedsUpdate();
		isEditing = true;
	}

	if (isEditing && ImGui::IsItemDeactivatedAfterEdit())
	{
		isEditing = false;
	}
#else
	{
		static Vector3 prevValue; /* 前回の値を保持する静的変数 */
		static Vector3 editorEuler; /* 編集中のオイラー角を保持する静的変数 */
		static bool isEditing = false; /* 編集中かどうかを追跡するフラグ */
		Quaternion* value = &rotation;
		if (!isEditing) /* 編集開始前に現在の値をオイラー角に変換して保存 */
		{
			editorEuler = GetEulerAngles();
		}

		IMGUI_PROPERTY("Rotation");
		bool valueChanged = false; // 値が変更されたかを追跡するフラグ
		valueChanged |= ImGui::DragFloat3("##rotation", &editorEuler.x);
		if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
		{
			prevValue = GetEulerAngles();
		}
		if (ImGui::IsItemDeactivatedAfterEdit()) /* 編集終了後にコマンドを発行 */
		{
			// 変更されたオイラー角と前回のオイラー角の差分を計算してクォータニオンに変換
			Vector3 newValue = editorEuler;
			{
				IMGUI_PROPERTY_COMMAND_CUSTOM("rotation", newValue, prevValue,
					"(" + std::to_string(newValue.x) + ", " + std::to_string(newValue.y) + ", " + std::to_string(newValue.z) + ")",
					"(" + std::to_string(prevValue.x) + ", " + std::to_string(prevValue.y) + ", " + std::to_string(prevValue.z) + ")",
					[this](const Vector3& rot) {SetRotation(rot); });
			}
			prevValue = newValue; /* 前回の値を更新 */
			GetTransform()->SetRotation(newValue); /* Transform の回転を更新 */
			isEditing = false; /* 編集終了 */
		}
		if (valueChanged) /* 値が変更された場合は Transform の回転を更新 */
		{
			GetTransform()->SetRotation(editorEuler);
			isEditing = true; /* 編集中 */
		}
	}
#endif // 0

	// スケールの編集
	{
		static Vector3 prevScale;
		IMGUI_PROPERTY("Scale");
		if (ImGui::DragFloat3("##Scale", &scale.x)) {
			MarkNeedsUpdate();
		}

		if (ImGui::IsItemActivated()) // 編集開始時に現在の値を保存
		{
			prevScale = scale;
		}
		if (ImGui::IsItemDeactivatedAfterEdit()) // 編集終了時にコマンドを発行
		{
			Vector3 newScale = scale;
			if (newScale.x != prevScale.x || newScale.y != prevScale.y || newScale.z != prevScale.z) {
				std::string newScaleStr = "(" + std::to_string(newScale.x) + ", " + std::to_string(newScale.y) + ", " + std::to_string(newScale.z) + ")";
				std::string prevScaleStr = "(" + std::to_string(prevScale.x) + ", " + std::to_string(prevScale.y) + ", " + std::to_string(prevScale.z) + ")";
				IMGUI_PROPERTY_COMMAND_CUSTOM("scale", newScale, prevScale, newScaleStr, prevScaleStr, [this](const Vector3& value) {
					SetScale(value);
					});
			}
			prevScale = newScale;
		}
	}

	IMGUI_PROPERTY_END();
#endif // USE_IMGUI
}

json Transform::Serialize() const
{
	/*json j;
	j["position"] = { position.x, position.y, position.z };
	j["rotation"] = { rotation.x, rotation.y, rotation.z, rotation.w };
	j["scale"] = { scale.x, scale.y, scale.z };
	j["eulerAngles"] = { eulerAngles.x, eulerAngles.y, eulerAngles.z };
	j["coordinateSystem"] = static_cast<int>(coordinateSystem);
	return j;*/
	return {};
}

void Transform::Deserialize(const json& j)
{
	/*if (j.contains("position")) {
		json arr = j["position"];
		if (arr.is_array() && arr.size() == 3) {
			position.x = arr[0].get<float>();
			position.y = arr[1].get<float>();
			position.z = arr[2].get<float>();
		}
	}
	if (j.contains("rotation")) {
		json arr = j["rotation"];
		if (arr.is_array() && arr.size() == 4) {
			rotation.x = arr[0].get<float>();
			rotation.y = arr[1].get<float>();
			rotation.z = arr[2].get<float>();
			rotation.w = arr[3].get<float>();
		}
	}
	if (j.contains("scale")) {
		json arr = j["scale"];
		if (arr.is_array() && arr.size() == 3) {
			scale.x = arr[0].get<float>();
			scale.y = arr[1].get<float>();
			scale.z = arr[2].get<float>();
		}
	}
	if (j.contains("eulerAngles")) {
		json arr = j["eulerAngles"];
		if (arr.is_array() && arr.size() == 3) {
			eulerAngles.x = arr[0].get<float>();
			eulerAngles.y = arr[1].get<float>();
			eulerAngles.z = arr[2].get<float>();
		}
	}
	if (j.contains("coordinateSystem")) {
		int cs = j["coordinateSystem"].get<int>();
		if (cs >= 0 && cs <= 3) coordinateSystem = static_cast<CoordinateSystem>(cs);
	}
	MarkNeedsUpdate();*/
}