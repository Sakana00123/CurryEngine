#include "pch.h"
#include "ColliderUtils.h"
#include "Engine/Input/InputSystem.h"
#include <DirectXCollision.h>

#include "Engine/Rendering/Camera/CameraComponent.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

#include "Engine/Physics/BoxCollider.h"
#include "Engine/Physics/SphereCollider.h"


bool ColliderUtils::Intersect(const XMFLOAT3& point, BoxCollider* collider) {
	bool isHit = false;
	Vector3 angles = collider->gameObject->transform->GetEulerAngles();
	//AABB
	if (static_cast<int>(angles.x) == 0 &&
		static_cast<int>(angles.y) == 0 &&
		static_cast<int>(angles.z) == 0)
	{
		XMFLOAT3 pos = collider->gameObject->transform->GetWorldPosition();
		XMFLOAT3 scale = collider->size;
		isHit = ((pos.x - (0.5f * scale.x) <= point.x && point.x <= pos.x + (0.5f * scale.x)) &&
			(pos.y - (0.5f * scale.y) <= point.y && point.y <= pos.y + (0.5f * scale.y)) &&
			(pos.z - (0.5f * scale.z) <= point.z && point.z <= pos.z + (0.5f * scale.z)));
	}
	//OBB
	else {

	}
	return isHit;
}
bool ColliderUtils::Intersect(const XMFLOAT3& point, SphereCollider* collider) {
	float radius = collider->radius;
	XMFLOAT3 p = collider->gameObject->transform->GetWorldPosition();
	float dx = point.x - p.x;
	float dy = point.y - p.y;
	float dz = point.z - p.z;
	return (dx * dx) / (radius * radius) + (dy * dy) / (radius * radius) + (dz * dz) / (radius * radius) <= 1.f;
}
bool ColliderUtils::Intersect(BoxCollider* b0, BoxCollider* b1) {
	XMFLOAT3 p0 = b0->gameObject->transform->GetWorldPosition(), p1 = b1->gameObject->transform->GetWorldPosition();
	XMFLOAT3 s0 = b0->size, s1 = b1->size;
	XMFLOAT3 min0 = { p0.x - s0.x * 0.5f, p0.y - s0.y * 0.5f, p0.z - s0.z * 0.5f };
	XMFLOAT3 max0 = { p0.x + s0.x * 0.5f, p0.y + s0.y * 0.5f, p0.z + s0.z * 0.5f };
	XMFLOAT3 min1 = { p1.x - s1.x * 0.5f, p1.y - s1.y * 0.5f, p1.z - s1.z * 0.5f };
	XMFLOAT3 max1 = { p1.x + s1.x * 0.5f, p1.y + s1.y * 0.5f, p1.z + s1.z * 0.5f };
	return (max0.x >= min1.x && max1.x >= min0.x && max0.y >= min1.y && max1.y >= min0.y && max0.z >= min1.z && max1.z >= min0.z);
}
bool ColliderUtils::Intersect(BoxCollider* boxCollider, SphereCollider* sphereCollider) {
	Transform* box = boxCollider->gameObject->transform;
	Transform* sphere = sphereCollider->gameObject->transform;
	XMFLOAT3 closestPoint{};
	XMFLOAT3 boxSize = boxCollider->size;
	float sphereRadius = sphereCollider->radius;
	closestPoint.x = max(box->GetWorldPosition().x - boxSize.x, min(sphere->GetWorldPosition().x, box->GetWorldPosition().x + boxSize.x));
	closestPoint.y = max(box->GetWorldPosition().y - boxSize.y, min(sphere->GetWorldPosition().y, box->GetWorldPosition().y + boxSize.y));
	closestPoint.z = max(box->GetWorldPosition().z - boxSize.z, min(sphere->GetWorldPosition().z, box->GetWorldPosition().z + boxSize.z));

	float distanceSq = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&closestPoint) - XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&sphere->GetWorldPosition()))));
	return distanceSq < (sphereRadius * sphereRadius);
}
bool ColliderUtils::Intersect(SphereCollider* s0, SphereCollider* s1) {
	XMVECTOR P0 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&s0->gameObject->transform->GetWorldPosition()));
	XMVECTOR P1 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&s1->gameObject->transform->GetWorldPosition()));
	XMVECTOR V = P0 - P1;
	float length = XMVectorGetX(XMVector3Length(V));
	XMVECTOR S0{ s0->radius, s0->radius, s0->radius, 0 };
	XMVECTOR S1{ s1->radius, s1->radius, s1->radius, 0 };
	float range = XMVectorGetX(S0 + S1);
	return (range >= length);
}

bool ColliderUtils::Raycast(BoxCollider* collider, HitResult& hitResult) {

	if (!collider) return false;

	XMFLOAT3 mousePos{};
	UINT num{ 1 };
	D3D11_VIEWPORT viewport{};
	Graphics::GetDeviceContext()->RSGetViewports(&num, &viewport);
	InputSystem::GetMousePosition(&mousePos.x);

	auto cam = SceneManager::GetCurrentScene()->cameraSystem.GetMainCamera();
	XMVECTOR CameraPosition = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&cam->gameObject->transform->GetWorldPosition()));
	XMMATRIX View = cam->GetViewMatrix();
	XMMATRIX Projection = cam->GetProjectionMatrix();
	
	mousePos.z = 0.0f;
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

	XMVECTOR Direction = XMVector3Normalize(End - Start);
	XMFLOAT3 max = collider->Max();
	XMFLOAT3 min = collider->Min();
	float y = min.y = max.y;
	//左上A、右上B、左下C、右下D
	XMVECTOR A = { min.x, y, max.z, 0 };
	XMVECTOR B = { max.x, max.y, max.z, 0 };
	XMVECTOR C = { min.x, min.y, min.z, 0 };
	XMVECTOR D = { max.x, y, min.z, 0 };
	// ABC, CBD
	float distance = 10000.f;
	if (DirectX::TriangleTests::Intersects(Start, Direction, A, B, C, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, C, B, D, distance)) {
		//結果を格納
		XMStoreFloat3(&hitResult.hitPosition, Start + (Direction * distance));
		//上面のみ判定なので上方向をいれる
		hitResult.hitNormal = { 0,1,0 };
		return true;
	}
	return false;
}

//下面以外判定
bool ColliderUtils::Raycast(BoxCollider* collider, float& distance) {
	XMFLOAT3 mousePos{};
	UINT num{ 1 };
	D3D11_VIEWPORT viewport{};
	Graphics::GetDeviceContext()->RSGetViewports(&num, &viewport);
	InputSystem::GetMousePosition(&mousePos.x);

	auto cam = SceneManager::GetCurrentScene()->cameraSystem.GetMainCamera();
	XMVECTOR CameraPosition = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&cam->gameObject->transform->GetWorldPosition()));
	XMMATRIX View = cam->GetViewMatrix();
	XMMATRIX Projection = cam->GetProjectionMatrix();

	mousePos.z = 0.0f;
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

	XMVECTOR Direction = XMVector3Normalize(End - Start);
	XMFLOAT3 max = collider->Max();
	XMFLOAT3 min = collider->Min();

	//上：左上T0、右上T1、左下T2、右下T3
	XMVECTOR T0 = { min.x, max.y, max.z, 0 };
	XMVECTOR T1 = { max.x, max.y, max.z, 0 };
	XMVECTOR T2 = { min.x, max.y, min.z, 0 };
	XMVECTOR T3 = { max.x, max.y, min.z, 0 };

	//前：左上F0、右上F1、左下F2、右下F3
	XMVECTOR F0 = { min.x, max.y, min.z, 0 };
	XMVECTOR F1 = { max.x, max.y, min.z, 0 };
	XMVECTOR F2 = { min.x, min.y, min.z, 0 };
	XMVECTOR F3 = { max.x, min.y, min.z, 0 };

	//後ろ：左上B0、右上B1、左下B2、右下B3
	XMVECTOR B0 = { max.x, max.y, max.z, 0 };
	XMVECTOR B1 = { min.x, max.y, max.z, 0 };
	XMVECTOR B2 = { max.x, min.y, max.z, 0 };
	XMVECTOR B3 = { min.x, min.y, max.z, 0 };

	//左：左上L0、右上L1、左下L2、右下L3
	XMVECTOR L0 = { min.x, max.y, max.z, 0 };
	XMVECTOR L1 = { min.x, max.y, min.z, 0 };
	XMVECTOR L2 = { min.x, min.y, max.z, 0 };
	XMVECTOR L3 = { min.x, min.y, min.z, 0 };

	//右：左上R0、右上R1、左下R2、右下R3
	XMVECTOR R0 = { max.x, max.y, min.z, 0 };
	XMVECTOR R1 = { max.x, max.y, max.z, 0 };
	XMVECTOR R2 = { max.x, min.y, min.z, 0 };
	XMVECTOR R3 = { max.x, min.y, max.z, 0 };

	if (DirectX::TriangleTests::Intersects(Start, Direction, T0, T1, T2, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, T2, T1, T3, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, F0, F1, F2, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, F2, F1, F3, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, B0, B1, B2, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, B2, B1, B3, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, L0, L1, L2, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, L2, L1, L3, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, R0, R1, R2, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, R2, R1, R3, distance)
		) {
		return true;
	}
	return false;
}

bool ColliderUtils::Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, BoxCollider* collider, HitResult& hitResult) {
	if (!collider) return false;
	XMVECTOR Start = XMLoadFloat3(&origin);
	XMVECTOR Direction = XMLoadFloat3(&direction);
	XMFLOAT3 max = collider->Max();
	XMFLOAT3 min = collider->Min();
	float y = min.y = max.y;
	//左上A、右上B、左下C、右下D
	XMVECTOR A = { min.x, y, max.z, 0 };
	XMVECTOR B = { max.x, max.y, max.z, 0 };
	XMVECTOR C = { min.x, min.y, min.z, 0 };
	XMVECTOR D = { max.x, y, min.z, 0 };
	// ABC, CBD
	float distance = 10000.f;
	if (DirectX::TriangleTests::Intersects(Start, Direction, A, B, C, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, C, B, D, distance)) {
		//結果を格納
		XMStoreFloat3(&hitResult.hitPosition, Start + (Direction * distance));
		//上面のみ判定なので上方向をいれる
		hitResult.hitNormal = { 0,1,0 };
		return true;
	}
	return false;
}

bool ColliderUtils::Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, BoxCollider* collider, float radius, HitResult& hitResult) {
	if (!collider) return false;
	XMVECTOR Start = XMLoadFloat3(&origin);
	XMVECTOR Direction = XMLoadFloat3(&direction);

	// 円の範囲内にいるか判定(Y成分は無視)
	{
		XMFLOAT3 center = collider->gameObject->transform->GetWorldPosition();
		XMFLOAT2 origin2D = { origin.x, origin.z };
		XMFLOAT2 center2D = { center.x, center.z };
		XMVECTOR toCenter2D = XMLoadFloat2(&origin2D) - XMLoadFloat2(&center2D);
		float distSq2D = XMVectorGetX(XMVector3LengthSq(toCenter2D));
		if (distSq2D > radius * radius) {
			return false; // 円の範囲外
		}
	}
	Vector3 preSize = collider->size;
	Vector3 size = { radius * 2.f, preSize.y, radius * 2.f };
	collider->size = size;

	XMFLOAT3 max = collider->Max();
	XMFLOAT3 min = collider->Min();

	collider->size = preSize;

	float y = min.y = max.y;
	//左上A、右上B、左下C、右下D
	XMVECTOR A = { min.x, y, max.z, 0 };
	XMVECTOR B = { max.x, max.y, max.z, 0 };
	XMVECTOR C = { min.x, min.y, min.z, 0 };
	XMVECTOR D = { max.x, y, min.z, 0 };
	// ABC, CBD
	float distance = 10000.f;
	if (DirectX::TriangleTests::Intersects(Start, Direction, A, B, C, distance) ||
		DirectX::TriangleTests::Intersects(Start, Direction, C, B, D, distance)) {
		//結果を格納
		XMStoreFloat3(&hitResult.hitPosition, Start + (Direction * distance));
		//上面のみ判定なので上方向をいれる
		hitResult.hitNormal = { 0,1,0 };
		return true;
	}
	return false;
}

bool ColliderUtils::Raycast(const XMFLOAT3& origin, const XMFLOAT3& center, float radius, HitResult& hitResult) {
	XMVECTOR Start = XMLoadFloat3(&origin);
	XMVECTOR Direction = XMVectorSet(0, -1, 0, 0);
	XMVECTOR Center = XMLoadFloat3(&center);

	// 円の範囲内にいるか判定(Y成分は無視)
	{
		XMFLOAT2 origin2D = { origin.x, origin.z };
		XMFLOAT2 center2D = { center.x, center.z };
		XMVECTOR toCenter2D = XMLoadFloat2(&origin2D) - XMLoadFloat2(&center2D);
		float distSq2D = XMVectorGetX(XMVector3LengthSq(toCenter2D));
		if (distSq2D > radius * radius) {
			return false; // 円の範囲外
		}
	}
	
	// 平面の法線ベクトル
	XMVECTOR PlaneNormal = { 0.f, 1.f, 0.f, 0.f };
	// レイと平面の交点までの距離を計算
	float denom = XMVectorGetX(XMVector3Dot(Direction, PlaneNormal));
	// 平行の場合は交差しない
	if (fabs(denom) > 1e-6f) {
		XMVECTOR diff = Center - Start;
		float t = XMVectorGetX(XMVector3Dot(diff, PlaneNormal)) / denom;
		// 交点がレイの始点より前にある場合は交差しない
		if (t >= 0.f) {
			XMVECTOR IntersectionPoint = Start + Direction * t;
			// 円形平面内に交点があるか確認
			XMVECTOR toCenter = IntersectionPoint - Center;
			toCenter = XMVectorSetY(toCenter, 0.f); // Y成分を無視
			float distSq = XMVectorGetX(XMVector3LengthSq(toCenter));
			if (distSq <= radius * radius) {
				// 結果を格納
				XMStoreFloat3(&hitResult.hitPosition, IntersectionPoint);
				hitResult.hitNormal = { 0.f, 1.f, 0.f }; // 平面の法線
				return true;
			}
		}
	}
	return false;
}