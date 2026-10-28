#include "pch.h"
#include "ThirdPersonCameraController.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

REGISTER_COMPONENT(ThirdPersonCameraController, "CameraController")

void ThirdPersonCameraController::Initialize()
{
	
	
}

void ThirdPersonCameraController::Start()
{
	//// 追従ターゲット設定
	//if (targetObjectId.IsValid())
	//{
	//	GameObject* targetObject = ObjectManager::Find(targetObjectId);
	//	if (targetObject)
	//	{
	//		targetTransform = targetObject->transform;
	//	}
	//}
	//// 注視ターゲット設定
	//if (lookAtObjectId.IsValid())
	//{
	//	GameObject* lookAtObject = ObjectManager::Find(lookAtObjectId);
	//	if (lookAtObject)
	//	{
	//		lookAtTransform = lookAtObject->transform;
	//	}
	//}
}

void ThirdPersonCameraController::Update(float deltaTime)
{
	// 回転入力
	if (isEnableAxisInput)
	{
		float rx = InputSystem::GetAxis(Side::Right, Axis::X);
		float ry = InputSystem::GetAxis(Side::Right, Axis::Y);
		int dx{}, dy{};
		InputSystem::GetMouseDelta(dx, dy);
		if (std::fabsf(rx) > 0.01f || std::fabsf(ry) > 0.01f ||
			abs(dx) > 0 || abs(dy) > 0) {
			Vector3 euler = GetOwner()->transform->GetEulerAngles();
			euler.x -= (static_cast<float>(invertY ? -1 : 1) * (ry - static_cast<float>(dy)) * deltaTime * rotationSpeed);
			euler.y += (static_cast<float>(invertX ? -1 : 1) * (rx + static_cast<float>(dx)) * deltaTime * rotationSpeed);
			euler.x = std::clamp(euler.x, minAngleX, maxAngleX);
			GetOwner()->transform->SetRotation(euler);
		}
	}

	// ズーム入力
	if (isEnableZoomInput)
	{
		bool enableInput = false;

#ifdef USE_IMGUI
		// シーンビュー上でのみホイール操作を受け付ける
		float left, top, right, bottom;
		Graphics::GetScreenRect(left, top, right, bottom);

		// マウスがシーンビュー上にあるか
		if (ImGui::IsMouseHoveringRect(ImVec2(left, top), ImVec2(right, bottom), false))
		{
			enableInput = true;
		}
#else
		enableInput = true;
#endif // USE_IMGUI

		// ズーム入力が有効の場合
		if (enableInput)
		{
			float zoomInput = InputSystem::GetWheelDelta();
			if (std::fabsf(zoomInput) > 0.01f) {
				targetDistance -= static_cast<float>(invertZoom ? -1 : 1) * zoomInput * zoomAmount;
			}
		}
	}

	// 距離調整処理
	{
		// 目標距離制限
		targetDistance = std::clamp(targetDistance, minDistance, maxDistance);

		// スムーズズーム処理
		if (useSmoothZoom)
		{
			distance = std::lerp(distance, targetDistance, deltaTime * zoomSpeed);
		}
		else
		{
			distance = targetDistance;
		}

		// 距離制限
		distance = std::clamp(distance, minDistance, maxDistance);
	}

	// 追従処理
	auto targetTransform = std::dynamic_pointer_cast<Transform>(ObjectManager::FindComponent(targetTransformId));
	if (targetTransform)
	{
		XMFLOAT3 forward = GetOwner()->GetTransform()->GetForward();
		XMVECTOR TargetPos = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&targetTransform->GetWorldPosition())) - XMLoadFloat3(&forward) * distance;
		// 位置設定
		if (useSmoothMovement)
		{
			XMFLOAT3 setPos;
			XMStoreFloat3(&setPos, XMVectorLerp(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&GetOwner()->GetTransform()->GetWorldPosition())), TargetPos, deltaTime * followSpeed));
			GetOwner()->GetTransform()->SetPosition(Vector3(setPos));
		}
		else
		{
			XMFLOAT3 setPos;
			XMStoreFloat3(&setPos, TargetPos);
			GetOwner()->GetTransform()->SetPosition(Vector3(setPos));
		}
	}

	// 注視処理
	auto lookAtTransform = std::dynamic_pointer_cast<Transform>(ObjectManager::FindComponent(lookAtTransformId));
	if (lookAtTransform)
	{
		Quaternion rotation;
		XMStoreFloat4(&rotation,
			XMQuaternionSlerp(XMLoadFloat4(&GetOwner()->GetTransform()->GetWorldRotation()),
				Transform::QuaternionLookAt(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&GetOwner()->GetTransform()->GetWorldPosition())),
					XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&lookAtTransform->GetWorldPosition()))), deltaTime * lookAtSpeed));
		GetOwner()->GetTransform()->SetWorldRotation(rotation);
	}
}

#ifdef USE_IMGUI
void ThirdPersonCameraController::DrawProperty(const PropertyDrawContext& context)
{
#if 0
	// followTarget
	{
		ImGui::PushID("followTarget");
		ImGui::Text("followTarget:");
		ImGui::SameLine();

		if (targetTransform) {
			ImGui::Button((targetTransform->GetOwner()->name + "(Transform)").c_str());
		}
		else {
			ImGui::Button("None(Transform)");
		}

		// ドラッグアンドドロップでターゲット設定
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
				IM_ASSERT(payload->DataSize == sizeof(ObjectId*));
				ObjectId* pId = static_cast<ObjectId*>(payload->Data);
				targetObjectId = *pId;
				GameObject* object = ObjectManager::Find(*pId);
				targetTransform = object->transform;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::SameLine();
		// クリアボタン
		if (ImGui::Button("X")) {
			targetTransform = nullptr;
		}
		ImGui::PopID();
	}

	// lookAtTarget
	{
		ImGui::PushID("lookAtTarget");
		ImGui::Text("lookAtTarget:");
		ImGui::SameLine();
		// ターゲット表示
		if (lookAtTransform) {
			ImGui::Button((lookAtTransform->GetOwner()->name + "(Transform)").c_str());
		}
		else {
			ImGui::Button("None(Transform)");
		}

		// ドラッグアンドドロップでターゲット設定
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
				IM_ASSERT(payload->DataSize == sizeof(ObjectId*));
				ObjectId* pId = static_cast<ObjectId*>(payload->Data);
				lookAtObjectId = *pId;
				GameObject* object = ObjectManager::Find(*pId);
				lookAtTransform = object->transform;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::SameLine();
		// クリアボタン
		if (ImGui::Button("X")) {
			lookAtTransform = nullptr;
		}
		ImGui::PopID();
	}
#endif // 0

	// TODO: いろんなヘルパーマクロがごちゃごちゃしてて混乱するので、ヘルパー名前空間にまとめるなどして整理する。
	// その他プロパティ
	Component::DrawProperty(context);
	//ImGui::Checkbox("isEnableAxisInput", &isEnableAxisInput);
	//ImGui::Checkbox("isEnableZoomInput", &isEnableZoomInput);
	//ImGui::Checkbox("invertX", &invertX);
	//ImGui::Checkbox("invertY", &invertY);
	//ImGui::Checkbox("invertZoom", &invertZoom);
	//ImGui::Checkbox("useSmoothMovement", &useSmoothMovement);
	//ImGui::Checkbox("useSmoothZoom", &useSmoothZoom);
	//ImGui::DragFloat("rotationSpeed", &rotationSpeed, 1.f, 1.f, 500.f);
	//ImGui::DragFloat("zoomAmount", &zoomAmount, 0.1f, 0.1f, 10.f);
	//ImGui::DragFloat("zoomSpeed", &zoomSpeed, 1.f, 1.f, 20.f);
	//ImGui::DragFloat("distance", &distance, 0.1f, minDistance, maxDistance);
	//ImGui::DragFloat("targetDistance", &targetDistance, 0.1f, minDistance, maxDistance);
	//ImGui::DragFloat("minDistance", &minDistance, 0.1f, 0.1f, maxDistance - 0.1f);
	//ImGui::DragFloat("maxDistance", &maxDistance, 0.1f, minDistance + 0.1f, 100.f);
	//ImGui::DragFloat("followSpeed", &followSpeed, 0.1f, 1.f, 20.f);
	//ImGui::DragFloat("lookAtSpeed", &lookAtSpeed, 0.1f, 1.f, 20.f);
	//ImGui::DragFloat("minAngleX", &minAngleX, 1.f, -89.f, maxAngleX - 1.f);
	//ImGui::DragFloat("maxAngleX", &maxAngleX, 1.f, minAngleX + 1.f, 89.f);

	//if (ImGui::Button("Reset"))
	//{
	//	isEnableAxisInput = true;
	//	isEnableZoomInput = true;
	//	invertX = false;
	//	invertY = false;
	//	invertZoom = false;
	//	useSmoothMovement = false;
	//	useSmoothZoom = true;
	//	rotationSpeed = 10.f;
	//	zoomAmount = 1.f;
	//	zoomSpeed = 10.f;
	//	distance = 25.f;
	//	targetDistance = 25.f;
	//	minDistance = 2.f;
	//	maxDistance = 50.f;
	//	followSpeed = 10.f;
	//	lookAtSpeed = 10.f;
	//	minAngleX = 0.f;
	//	maxAngleX = 45.f;
	//}


	//C_COMPONENT_PROPERTY_HEADER();
	//C_COMPONENT_PROPERTY_FIELD(cameraComponent, "Camera Component", "カメラコンポーネント");
	//C_COMPONENT_PROPERTY_FIELD(targetTransform, "Target Transform", "追従ターゲットのTransform");
	//C_COMPONENT_PROPERTY_FIELD(lookAtTransform, "LookAt Transform", "注視ターゲットのTransform");
	//C_COMPONENT_PROPERTY_FIELD(isEnableAxisInput, "Enable Axis Input", "入力（Axis）を有効化するか");
	//C_COMPONENT_PROPERTY_FIELD(isEnableZoomInput, "Enable Zoom Input", "ズーム入力を有効化するか");
	//C_COMPONENT_PROPERTY_FIELD(invertX, "Invert X", "X軸反転");
	//C_COMPONENT_PROPERTY_FIELD(invertY, "Invert Y", "Y軸反転");
	//C_COMPONENT_PROPERTY_FIELD(invertZoom, "Invert Zoom", "ズーム反転");
	//C_COMPONENT_PROPERTY_FIELD(useSmoothMovement, "Use Smooth Movement", "スムーズ移動を使用するか");
	//C_COMPONENT_PROPERTY_FIELD(useSmoothZoom, "Use Smooth Zoom", "スムーズズームを使用するか");
	//C_COMPONENT_PROPERTY_FIELD(rotationSpeed, "Rotation Speed", "回転速度");
	//C_COMPONENT_PROPERTY_FIELD(zoomAmount, "Zoom Amount", "ズーム量");
	//C_COMPONENT_PROPERTY_FIELD(zoomSpeed, "Zoom Speed", "ズーム速度(スムーズ時)");
	//C_COMPONENT_PROPERTY_FIELD(distance, "Distance", "カメラ距離(現在値)");
	//C_COMPONENT_PROPERTY_FIELD(targetDistance, "Target Distance", "目標距離(ズーム先)");
	//C_COMPONENT_PROPERTY_FIELD(minDistance, "Min Distance", "最小距離");
	//C_COMPONENT_PROPERTY_FIELD(maxDistance, "Max Distance", "最大距離");
	//C_COMPONENT_PROPERTY_FIELD(followSpeed, "Follow Speed", "追従速度");
	//C_COMPONENT_PROPERTY_FIELD(lookAtSpeed, "LookAt Speed", "注視速度");
	//C_COMPONENT_PROPERTY_FIELD(minAngleX, "Min Angle X", "X軸最小角度");
	//C_COMPONENT_PROPERTY_FIELD(maxAngleX, "Max Angle X", "X軸最大角度");
	//C_COMPONENT_PROPERTY_FOOTER();
}
#endif /// USE_IMGUI

json ThirdPersonCameraController::Serialize() const
{
	json j = Component::Serialize();
	//j["targetObjectId"] = targetObjectId.ToString();
	//j["lookAtObjectId"] = lookAtObjectId.ToString();
	return j;
}

void ThirdPersonCameraController::Deserialize(const json& j)
{
	Component::Deserialize(j);
	/*if (j.contains("targetObjectId"))
	{
		if (j["targetObjectId"].is_string())
		{
			targetObjectId = ObjectId::FromString(j["targetObjectId"].get<std::string>());
		}
		else if (j["targetObjectId"].is_number_integer())
		{
			targetObjectId = ObjectId::FromLegacy(j["targetObjectId"].get<int>());
		}
	}
	if (j.contains("lookAtObjectId"))
	{
		if (j["lookAtObjectId"].is_string())
		{
			lookAtObjectId = ObjectId::FromString(j["lookAtObjectId"].get<std::string>());
		}
		else if (j["lookAtObjectId"].is_number_integer())
		{
			lookAtObjectId = ObjectId::FromLegacy(j["lookAtObjectId"].get<int>());
		}
	}*/
}