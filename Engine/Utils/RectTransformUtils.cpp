#include "pch.h"
#include "RectTransformUtils.h"
#include "Engine/UI/RectTransform.h"
#include "Engine/Core/Transform.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Rendering/Camera/CameraComponent.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

void RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
	RectTransform* rect,
	const Vector2& newAnchorMin,
	const Vector2& newAnchorMax,
	const Vector2& newPivot) 
{
	if (rect == nullptr || rect->GetParent() == nullptr)
		return;

	//親のサイズ
	RectTransform* parent = rect->GetParent();
	XMFLOAT2 parentSize = parent->GetWorldSize();
	XMVECTOR ParentSize = XMLoadFloat2(&parentSize);
	
	//現在の値を退避
	XMVECTOR OldAnchorMin = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&rect->anchorMin));
	XMVECTOR OldAnchorMax = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&rect->anchorMax));
	XMVECTOR OldPivot = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&rect->pivot));

	XMVECTOR NewAnchorMin = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&newAnchorMin));
	XMVECTOR NewAnchorMax = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&newAnchorMax));
	XMVECTOR NewPivot = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&newPivot));

	// アンカーによる位置変化補正
	XMVECTOR AnchorOffsetDelta =
		((NewAnchorMin - OldAnchorMin) * ParentSize +
			(NewAnchorMax - OldAnchorMax) * ParentSize) * 0.5f;

	// ピボットによる補正
	XMFLOAT2 size = rect->GetWorldSize();
	XMVECTOR Size = XMLoadFloat2(&size);
	XMVECTOR PivotOffsetDelta = (NewPivot - OldPivot) * Size;

	// anchoredPosition を取得して補正
	Vector2 anchoredPosition = rect->GetAnchoredPosition();
	XMVECTOR anchoredPosV = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&anchoredPosition));
	XMVECTOR corrected = anchoredPosV - AnchorOffsetDelta + PivotOffsetDelta;

	Vector2 correctedPosition;
	XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&correctedPosition), corrected);
	rect->SetAnchoredPosition(correctedPosition);

	// 実際に Anchor と Pivot を変更
	rect->SetAnchorMin(newAnchorMin);
	rect->SetAnchorMax(newAnchorMax);
	rect->SetPivot(newPivot);
}

void RectTransformUtils::SetAnchordPositionBy3DTransform(
	RectTransform* rect,
	Transform* targetTransform) 
{
	if (rect == nullptr || targetTransform == nullptr)
		return;
	//3D座標をスクリーン座標に変換
	XMFLOAT2 screenPos;
	{
		//現在のカメラのViewとProjectionを求める
#if 1
		auto cam = SceneManager::GetCurrentScene()->cameraSystem.GetMainCamera();
		XMMATRIX View = cam->GetViewMatrix();
		XMMATRIX Projection = cam->GetProjectionMatrix();
#else
		XMFLOAT4X4 view = Graphics::GetView();
		XMFLOAT4X4 projection = Graphics::GetProjection();
		XMMATRIX View = XMLoadFloat4x4(&view);
		XMMATRIX Projection = XMLoadFloat4x4(&projection);
#endif // 0

		//スクリーンサイズ
		float screenSizeX = 1920, screenSizeY = 1080;
		//Graphics::GetScreenSize(screenSizeX, screenSizeY);

		//ワールド座標をスクリーン座標に変換
		XMVECTOR ScreenPosition = XMVector3Project(
			XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&targetTransform->GetWorldPosition())),
			0.0f,
			0.0f,
			screenSizeX,
			screenSizeY,
			0.0f,
			1.0f,
			Projection,
			View,
			XMMatrixIdentity()
		);
		XMStoreFloat2(&screenPos, ScreenPosition);
	}
	//アンカーポジションとして設定
	rect->SetAnchoredPositionByAnchor({ 0,0 }, { screenPos.x, screenPos.y });
}

Vector3 RectTransformUtils::UIScreenToWorld(
	const Vector2& screenPos,
	float depth)
{
	//現在のカメラのViewとProjectionを求める
	auto cam = SceneManager::GetCurrentScene()->cameraSystem.GetMainCamera();
	XMMATRIX View = cam->GetViewMatrix();
	XMMATRIX Projection = cam->GetProjectionMatrix();
	//スクリーンサイズ
	float screenSizeX = 1920, screenSizeY = 1080;
	//Graphics::GetScreenSize(screenSizeX, screenSizeY);
	//スクリーン座標をワールド座標に変換
	XMVECTOR GetWorldPosition = XMVector3Unproject(
		XMVectorSet(screenPos.x, screenPos.y, depth, 1.0f),
		0.0f,
		0.0f,
		screenSizeX,
		screenSizeY,
		0.0f,
		1.0f,
		Projection,
		View,
		XMMatrixIdentity()
	);
	XMFLOAT3 worldPos;
	XMStoreFloat3(&worldPos, GetWorldPosition);
	return Vector3(worldPos);
}

XMFLOAT2 RectTransformUtils::WorldToUIScreen(
	const Vector3& worldPos)
{
	//現在のカメラのViewとProjectionを求める
	auto cam = SceneManager::GetCurrentScene()->cameraSystem.GetMainCamera();
	XMMATRIX View = cam->GetViewMatrix();
	XMMATRIX Projection = cam->GetProjectionMatrix();
	//スクリーンサイズ
	float screenSizeX = 1920, screenSizeY = 1080;
	//Graphics::GetScreenSize(screenSizeX, screenSizeY);
	//ワールド座標をスクリーン座標に変換
	XMVECTOR ScreenPosition = XMVector3Project(
		XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&worldPos)),
		0.0f,
		0.0f,
		screenSizeX,
		screenSizeY,
		0.0f,
		1.0f,
		Projection,
		View,
		XMMatrixIdentity()
	);
	XMFLOAT2 screenPos;
	XMStoreFloat2(&screenPos, ScreenPosition);
	return screenPos;
}