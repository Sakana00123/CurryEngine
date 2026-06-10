#include "pch.h"
#include "Transform.h"
#include "GameObject.h"
#ifdef USE_IMGUI
#include <ImGuizmo.h>
#include "Engine/Rendering/Pipeline/Graphics.h"
#include <Engine\Resources\ResourceManager.h>
#include <imgui_internal.h>
#include "Engine/EditorSupport/PropertyDrawHelper.h"
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
		m_eulerAngles = rotation.ToEuler(); // ローカル回転からオイラー角を計算して保存
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

#ifdef USE_IMGUI
// ImGuiのSliderScalarNは、複数のスカラー値を同時に編集するための関数ですが、デフォルトでは、スカラー値が変更されたときにコマンドを発行する機能がありません。そこで、SliderCustomScalarN関数を定義して、スカラー値が変更されたときにコマンドを発行できるようにします。
namespace ImGui
{
	inline bool DragCustomScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags[3])
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		bool value_changed = false;
		BeginGroup();
		PushID(label);
		PushMultiItemsWidths(components, CalcItemWidth());
		size_t type_size = DataTypeGetInfo(data_type)->Size;
		for (int i = 0; i < components; i++)
		{
			PushID(i);
			BeginDisabled(flags[i] & ImGuiSliderFlags_ReadOnly);
			if (i > 0)
				SameLine(0, g.Style.ItemInnerSpacing.x);
			value_changed |= DragScalar("", data_type, p_data, v_speed, p_min, p_max, format, flags[i]);
			EndDisabled();
			PopID();
			PopItemWidth();
			p_data = (void*)((char*)p_data + type_size);
		}
		PopID();

		const char* label_end = FindRenderedTextEnd(label);
		if (label != label_end)
		{
			SameLine(0, g.Style.ItemInnerSpacing.x);
			TextEx(label, label_end);
		}

		EndGroup();
		return value_changed;
	}

	inline bool DragCustomFloat3(const char* label, float v[3], float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags[3])
	{
		return DragCustomScalarN(label, ImGuiDataType_Float, v, 3, v_speed, &v_min, &v_max, format, flags);
	}
}
#endif // USE_IMGUI
#ifdef USE_IMGUI

void Transform::DrawProperty(const PropertyDrawContext& context)
{
	Component::DrawProperty(context); // 基底クラスの描画を呼び出す
	return;
	IMGUI_PROPERTY_BEGIN();

	Transform* transform = dynamic_cast<Transform*>(context.Primary());
	if (transform)
	{
		// 位置の編集
		{
			Vector3& position = transform->position;

			const PropertyInfo* posInfo = transform->GetClassMeta()->FindProperty("position");
			bool mixedValue = posInfo ? CurryEngine::PropertyDrawHelper::HasMixedValues<Vector3>(context, *posInfo) : false;
			const char* format = mixedValue ? "---" : "%.3f";

			static Vector3 prevPosition;
			IMGUI_PROPERTY("Position");
			if (ImGui::DragFloat3("##Position", &position.x, 1.0f, 0.0f, 0.0f, format)) {
				//MarkNeedsUpdate();
				CurryEngine::PropertyDrawHelper::ApplyToAll<Vector3>(context, *posInfo, position);
				for (size_t i = 1; i < context.targets.size(); i++) {
					if (auto* t = dynamic_cast<Transform*>(context.targets[i])) {
						t->MarkNeedsUpdate();
					}
				}
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
			if (scale.LengthSq() != 0)
			{
				lastValidScale = scale;
			}
			Vector3 prevThisFrameScale = scale;
			ImGuiSliderFlags flags[3] = {
				(!enableScaleLink || (lastValidScale.x != 0)) ? ImGuiSliderFlags_None : ImGuiSliderFlags_ReadOnly,
				(!enableScaleLink || (lastValidScale.y != 0)) ? ImGuiSliderFlags_None : ImGuiSliderFlags_ReadOnly,
				(!enableScaleLink || (lastValidScale.z != 0)) ? ImGuiSliderFlags_None : ImGuiSliderFlags_ReadOnly
			};
			if (ImGui::DragCustomFloat3("##Scale", &scale.x, 0.01f, 0.0f, 0.0f, "%.3f", flags)) {
				if (scale.LengthSq() == 0 && prevThisFrameScale.LengthSq() != 0)
				{
					lastValidScale = prevThisFrameScale; // スケールが0になった場合は、最後に有効だったスケールを保存しておく
				}
				if (enableScaleLink) {
					// スケールリンクが有効な場合、どの軸が変更されたかを判定して、変更された軸のスケールの変化率を計算し、他の軸にも同じ変化率を適用する
					if (prevThisFrameScale.x == prevThisFrameScale.y && prevThisFrameScale.y == prevThisFrameScale.z) {
						// すべての軸が同じ値の場合、どの軸が変更されたかを判定できないため、最後に保存された有効なスケールの値を基準に変更する
						if (scale.x != prevThisFrameScale.x) {
							scale.y = scale.z = scale.x;
						}
						else if (scale.y != prevThisFrameScale.y) {
							scale.x = scale.z = scale.y;
						}
						else if (scale.z != prevThisFrameScale.z) {
							scale.x = scale.y = scale.z;
						}
					}
					else if (lastValidScale.LengthSq() != 0)
					{
						int changedAxis = -1;
						if (scale.x != lastValidScale.x) changedAxis = 0;
						else if (scale.y != lastValidScale.y) changedAxis = 1;
						else if (scale.z != lastValidScale.z) changedAxis = 2;

						if (changedAxis != -1) {
							float scaleFactor = 1.0f;
							switch (changedAxis) {
							case 0: scaleFactor = scale.x / lastValidScale.x; break;
							case 1: scaleFactor = scale.y / lastValidScale.y; break;
							case 2: scaleFactor = scale.z / lastValidScale.z; break;
							}
							for (int i = 0; i < 3; i++) {
								if (i != changedAxis) {
									switch (i) {
									case 0: scale.x = lastValidScale.x * scaleFactor; break;
									case 1: scale.y = lastValidScale.y * scaleFactor; break;
									case 2: scale.z = lastValidScale.z * scaleFactor; break;
									}
								}
							}
						}
					}
				}
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

			// スケールリンクフラグの編集
			{
				ImGui::SameLine();
				auto iconTex = ResourceManager::GetOrLoad<AssetTexture>("./Data/Icon/editorIcons.png");
				float buttonSize = 22.0f;
				float paddingX = 4.0f;

				ImVec2 linkIconUV0 = ImVec2(0.75f, 0.0f);
				ImVec2 linkIconUV1 = ImVec2(1.0f, 0.25f);

				ImVec2 unLinkIconUV0 = ImVec2(0.0f, 0.25f);
				ImVec2 unLinkIconUV1 = ImVec2(0.25f, 0.5f);

				if (ImGui::ImageButton("##ScaleLink", iconTex->GetSRV(), ImVec2(buttonSize, buttonSize),
					enableScaleLink ? linkIconUV0 : unLinkIconUV0,
					enableScaleLink ? linkIconUV1 : unLinkIconUV1))
				{
					enableScaleLink = !enableScaleLink;
				}
			}
		}
	}

	IMGUI_PROPERTY_END();
}
#endif // USE_IMGUI

json Transform::Serialize() const
{
	json j = Component::Serialize();
	j["enableScaleLink"] = enableScaleLink;
	return {};
}

void Transform::Deserialize(const json& j)
{
	Component::Deserialize(j);
	enableScaleLink = j.value("enableScaleLink", false);
}