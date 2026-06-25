#include "pch.h"
#include "EditorCamera.h"
#include <algorithm>
#include "Engine/Editor/Console.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Core/Time.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

void EditorCamera::Initialize()
{
	//デフォルト距離制限
	SetClampDistance(0.001f, 1000.0f);
}

void EditorCamera::Update(float elapsedTime)
{

#ifdef USE_IMGUI
	if (updateFlagFunction ? updateFlagFunction() : false) {
		if (float wheelDelta = InputSystem::GetWheelDelta()) {
			distance -= wheelDelta * 0.1f; // ホイールの回転量に応じて距離を調整
			distance = std::clamp(distance, minDistance, maxDistance);
		}
	}
#endif // USE_IMGUI

	//DebugCameraは常に更新
	elapsedTime = Time::UnscaledDeltaTime();

	//カメラ操作
	if (GetAsyncKeyState(moveKey) || GetAsyncKeyState(rotateKey) || GetAsyncKeyState(panKey))
	{
		if (!isMoving)
		{
			isMoving = updateFlagFunction ? updateFlagFunction() : false;
		}
		else
		{
			//回転処理
			float rx = InputSystem::GetAxis(Side::Right, Axis::X);
			float ry = InputSystem::GetAxis(Side::Right, Axis::Y);
			int dx{}, dy{};
			InputSystem::GetMouseDelta(dx, dy);

			// 右ボタン押下中に回転
			if (GetAsyncKeyState(rotateKey))
			{
				if (std::fabsf(rx) > 0.01f || std::fabsf(ry) > 0.01f ||
					abs(dx) > 0 || abs(dy) > 0)
				{
					Vector3 euler = (rotation).ToEuler();
					euler.x -= ((ry - dy) * elapsedTime * rotateSpeed);
					euler.y += ((rx + dx) * elapsedTime * rotateSpeed);
					euler.x = std::clamp(euler.x, minAngleX, maxAngleX);
					rotation = Quaternion::FromEuler(euler);
				}
			}
			else
			{
				rx = 0;
				ry = 0;
			}

			//カメラ移動
			Vector3 move{};
			// 回転行列の取得
			XMMATRIX Rotation = DirectX::XMMatrixRotationQuaternion(XMLoadFloat4(&rotation));
			// 入力の取得
			float axisX = 0.0f;
			float axisY = 0.0f;
			float axisZ = 0.0f;

			XMVECTOR Move{};
			if (GetAsyncKeyState(moveKey))
			{
				// キーボード入力
				axisX = InputSystem::GetAxis(Side::Left, Axis::X);
				axisZ = InputSystem::GetAxis(Side::Left, Axis::Y);

				// スピードアップ
				float scale = GetAsyncKeyState(VK_LSHIFT) ? 2.0f : 1.0f;

				// 上下移動
				int up = 0;
				if (GetAsyncKeyState('E')) up++;
				if (GetAsyncKeyState('Q')) up--;

				// 移動値の計算
				{
					if (std::fabsf(axisX) > 0.01f) {
						XMVECTOR Right = Rotation.r[0];
						Move += Right * axisX;
					}
					if (std::fabsf(axisY) > 0.01f) {
						XMVECTOR Up = Rotation.r[1];
						Move += Up * axisY;
					}
					if (std::fabsf(axisZ) > 0.01f) {
						XMVECTOR Forward = Rotation.r[2];
						Move += Forward * axisZ;
					}
					if (up != 0) {
						XMVECTOR Up = XMVectorSet(0, 1, 0, 0);
						Move += Up * static_cast<float>(up);
					}

					XMFLOAT3 m{};
					DirectX::XMStoreFloat3(&m, XMVector3Normalize(Move) * speed * scale * distance * elapsedTime);
					move = Vector3(m);
				}
			}
			else if (GetAsyncKeyState(panKey))
			{
				// マウスホイールボタン押下中はマウス移動で平行移動
				axisX = static_cast<float>(-dx) * 0.1f;
				axisY = static_cast<float>(dy) * 0.1f;

				// 移動値の計算
				{
					if (std::fabsf(axisX) > 0.0001f) {
						XMVECTOR Right = Rotation.r[0];
						Move += Right * axisX;
					}
					if (std::fabsf(axisY) > 0.0001f) {
						XMVECTOR Up = Rotation.r[1];
						Move += Up * axisY;
					}

					XMFLOAT3 m{};
					DirectX::XMStoreFloat3(&m, Move * distance * 0.1f);
					move = Vector3(m);
				}
			}
			// 移動の適用
			position += move;
		}
	}
	else
	{
		// ボタンが離されたら移動フラグをリセット
		isMoving = false;
	}
}

XMMATRIX EditorCamera::GetViewMatrix()
{
	XMVECTOR Quaternion = XMLoadFloat4(&rotation);
	XMVECTOR Forward = XMVector3TransformNormal(
		XMVectorSet(0, 0, -1, 0), // Unityのforwardと反対 (-Z)
		XMMatrixRotationQuaternion(Quaternion) // 回転を適用
	);
	XMVECTOR Focus = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&position)) + Forward;
	XMVECTOR Eye = Focus + Forward * distance;
	XMVECTOR Up = XMVector3TransformNormal(
		XMVectorSet(0, 1, 0, 0),
		XMMatrixRotationQuaternion(Quaternion)
	);
	return XMMatrixLookAtLH(Eye, Focus, Up);
}

XMMATRIX EditorCamera::GetProjectionMatrix()
{
	return XMMatrixPerspectiveFovLH(XMConvertToRadians(fieldOfView), aspect, nearZ, farZ);
}

void EditorCamera::ScreenPointToRay(const Vector2& screenPos, Vector3& outOrigin, Vector3& outDirection)
{
	UINT num{ 1 };
	D3D11_VIEWPORT viewport{};
	Graphics::GetDeviceContext()->RSGetViewports(&num, &viewport);

	XMMATRIX View = GetViewMatrix();
	XMMATRIX Projection = GetProjectionMatrix();
	XMFLOAT3 mousePos{ screenPos.x, screenPos.y, 0.0f };
	XMVECTOR Start = XMVector3Unproject(
		XMLoadFloat3(&mousePos),
		viewport.TopLeftX,
		viewport.TopLeftY,
		viewport.Width,
		viewport.Height,
		viewport.MinDepth,
		viewport.MaxDepth,
		Projection,
		View,
		XMMatrixIdentity()
	);
	mousePos.z = 1.0f;
	XMVECTOR End = XMVector3Unproject(
		XMLoadFloat3(&mousePos),
		viewport.TopLeftX,
		viewport.TopLeftY,
		viewport.Width,
		viewport.Height,
		viewport.MinDepth,
		viewport.MaxDepth,
		Projection,
		View,
		XMMatrixIdentity()
	);
	XMVECTOR RayDirection = XMVector3Normalize(XMVectorSubtract(End, Start));
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&outDirection), RayDirection);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&outOrigin), Start);
}

void EditorCamera::DrawProperty()
{
#ifdef USE_IMGUI
	ImGui::DragFloat("distance", &distance, 1.0f, minDistance, maxDistance);
	ImGui::DragFloat("fieldOfView", &fieldOfView, 1.0f, 1.0f, 179.0f);
#endif // USE_IMGUI
}

json EditorCamera::Serialize() const
{
	json j;
	j["position"] = { position.x, position.y, position.z };
	j["rotation"] = { rotation.x, rotation.y, rotation.z, rotation.w };
	j["distance"] = distance;
	j["minDistance"] = minDistance;
	j["maxDistance"] = maxDistance;
	j["speed"] = speed;
	j["rotateSpeed"] = rotateSpeed;
	j["maxAngleX"] = maxAngleX;
	j["minAngleX"] = minAngleX;
	j["fieldOfView"] = fieldOfView;
	j["aspect"] = aspect;
	j["nearZ"] = nearZ;
	j["farZ"] = farZ;
	return j;
}

void EditorCamera::Deserialize(const json& j)
{
	if (j.contains("position")) {
		auto pos = j["position"];
		position = Vector3(pos[0], pos[1], pos[2]);
	}
	if (j.contains("rotation")) {
		auto rot = j["rotation"];
		rotation = Quaternion{ rot[0], rot[1], rot[2], rot[3] };
	}
	if (j.contains("distance")) {
		distance = j["distance"].get<float>();
	}
	if (j.contains("minDistance")) {
		minDistance = j["minDistance"].get<float>();
		minDistance = 0.1f; // 最小距離は0.1fに設定
	}
	if (j.contains("maxDistance")) {
		maxDistance = j["maxDistance"].get<float>();
	}
	if (j.contains("speed")) {
		speed = j["speed"].get<float>();
	}
	if (j.contains("rotateSpeed")) {
		rotateSpeed = j["rotateSpeed"].get<float>();
	}
	if (j.contains("maxAngleX")) {
		maxAngleX = j["maxAngleX"].get<float>();
	}
	if (j.contains("minAngleX")) {
		minAngleX = j["minAngleX"].get<float>();
	}
	if (j.contains("fieldOfView")) {
		fieldOfView = j["fieldOfView"].get<float>();
	}
	if (j.contains("aspect")) {
		aspect = j["aspect"].get<float>();
	}
	if (j.contains("nearZ")) {
		nearZ = j["nearZ"].get<float>();
	}
	if (j.contains("farZ")) {
		farZ = j["farZ"].get<float>();
	}
}