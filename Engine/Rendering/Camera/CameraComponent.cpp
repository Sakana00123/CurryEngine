#include "pch.h"
#include "CameraComponent.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Rendering/Camera/CameraSystem.h"


REGISTER_COMPONENT(CameraComponent, "Camera")

void CameraComponent::Initialize()
{

}

#ifdef USE_IMGUI
void CameraComponent::DrawProperty(const PropertyDrawContext& context)
{
	//ImGui::DragFloat("Field Of View", &fieldOfView, 1.0f, 1.0f, 120.0f);
	//ImGui::DragFloat("Near Clip", &nearClip, 0.01f, 0.01f, 10.0f);
	//ImGui::DragFloat("Far Clip", &farClip, 1.f, 100.0f, 10000.0f);

	Component::DrawProperty(context);

}
#endif // USE_IMGUI

void CameraComponent::OnEnable()
{
	// カメラが有効になったとき、カメラシステムにメインカメラの更新が必要であることを通知
	if (auto* scene = GetOwner()->GetScene())
	{
		scene->cameraSystem.NotifyCameraChanged();
	}
}

void CameraComponent::OnDisable()
{
	// カメラが無効になったとき、カメラシステムにメインカメラの更新が必要であることを通知
	if (auto* scene = GetOwner()->GetScene())
	{
		scene->cameraSystem.NotifyCameraChanged();
	}
}

void CameraComponent::ScreenPointToRay(const Vector2& screenPos, Vector3& outOrigin, Vector3& outDirection) const
{
	if (!GetOwner() || !GetOwner()->GetTransform())
	{
		outOrigin = Vector3::Zero;
		outDirection = Vector3::Forward;
		return;
	}
	//UINT num{ 1 };
	//D3D11_VIEWPORT viewport{};
	//Graphics::GetDeviceContext()->RSGetViewports(&num, &viewport);
	float topLeftX = 0.0f;
	float topLeftY = 0.0f;
	float width = 1920.0f;
	float height = 1080.0f;
	D3D11_VIEWPORT viewport{ topLeftX, topLeftY, width, height, 0.0f, 1.0f };
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

XMMATRIX CameraComponent::GetViewMatrix() const
{
	XMMATRIX World = XMLoadFloat4x4(&GetOwner()->transform->GetWorld());
	XMVECTOR Scale, Quaternion, Eye;//Scaleは使用しない
	XMMatrixDecompose(&Scale, &Quaternion, &Eye, World);
	
	XMVECTOR Forward = XMVector3TransformNormal(
		XMVectorSet(0, 0, 1, 0), // 前方向ベクトル
		XMMatrixRotationQuaternion(Quaternion) // 回転を適用
	);
	XMVECTOR Focus = Eye + Forward;
	XMVECTOR Up = XMVector3TransformNormal(
		XMVectorSet(0, 1, 0, 0),
		XMMatrixRotationQuaternion(Quaternion)
	);
	return XMMatrixLookAtLH(Eye, Focus, Up);
}

XMMATRIX CameraComponent::GetProjectionMatrix() const
{
	return XMMatrixPerspectiveFovLH(
		XMConvertToRadians(fieldOfView),
		aspect,
		nearClip,
		farClip
	);
}