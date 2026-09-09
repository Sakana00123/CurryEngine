#include "pch.h"
#include "Engine/Physics/Physics.h"
#include "Engine/Physics/Collider.h"

ENGINE_API void Physics_SetGravity(float x, float y, float z)
{
	Physics::SetGravity(Vector3(x, y, z));
}

ENGINE_API Vector3 Physics_GetGravity()
{
	Vector3 gravity = Physics::GetGravity();
	return gravity;
}

struct RaycastHitExport
{
	Vector3 point; // 衝突点の位置
	Vector3 normal; // 衝突面の法線
	float distance; // レイの発射点から衝突点までの距離
	uint64_t colliderId; // 衝突したコライダのID
};

ENGINE_API bool Physics_Raycast(Vector3 origin, Vector3 direction, RaycastHitExport* outHit, float maxDistance, int layerMask)
{
	RaycastHit hitInfo;
	bool hit = Physics::Raycast(origin, direction, maxDistance, hitInfo, layerMask);
	if (hit && outHit)
	{
		outHit->point = hitInfo.point;
		outHit->normal = hitInfo.normal;
		outHit->distance = hitInfo.distance;
		outHit->colliderId = hitInfo.collider ? hitInfo.collider->GetId().Value() : 0; // colliderがnullptrの場合はIDを0に設定
	}
	return hit;
}
