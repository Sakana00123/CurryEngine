#include "pch.h"
#include "Engine/Physics/Physics.h"
#include "Engine/Physics/Rigidbody.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Core/ObjectManager.h"


static Rigidbody* GetRigidbody(uint64_t objectId)
{
	if (auto gameObject = ObjectManager::Find(ObjectId::FromValue(objectId)))
	{
		return gameObject->GetComponent<Rigidbody>();
	}
	return nullptr;
}

// --- エクスポート関数の実装 ---

ENGINE_API void Rigidbody_AddForce(uint64_t objectId, Vector3 force, int mode)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		rigidbody->AddForce(force, static_cast<ForceMode>(mode));
	}
}

ENGINE_API void Rigidbody_AddTorque(uint64_t objectId, Vector3 force, int mode)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		rigidbody->AddTorque(force, static_cast<ForceMode>(mode));
	}
}

ENGINE_API void Rigidbody_SetVelocity(uint64_t objectId, Vector3 velocity)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		rigidbody->SetVelocity(velocity);
	}
}

ENGINE_API void Rigidbody_SetAngularVelocity(uint64_t objectId, Vector3 angularVelocity)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		rigidbody->SetAngularVelocity(angularVelocity);
	}
}

ENGINE_API void Rigidbody_SetMass(uint64_t objectId, float mass)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		rigidbody->SetMass(mass);
	}
}

ENGINE_API void Rigidbody_SetIsKinematic(uint64_t objectId, bool isKinematic)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		rigidbody->SetKinematic(isKinematic);
	}
}

ENGINE_API void Rigidbody_SetKinematicTarget(uint64_t objectId, Vector3 pos, Quaternion rot)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		rigidbody->SetKinematicTarget(pos, rot);
	}
}

ENGINE_API void Rigidbody_SetUseGravity(uint64_t objectId, bool useGravity)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		rigidbody->SetUseGravity(useGravity);
	}
}

ENGINE_API Vector3 Rigidbody_GetVelocity(uint64_t objectId)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		return rigidbody->GetVelocity();
	}
	return Vector3::Zero;
}

ENGINE_API Vector3 Rigidbody_GetAngularVelocity(uint64_t objectId)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		return rigidbody->GetAngularVelocity();
	}
	return Vector3::Zero;
}

ENGINE_API float Rigidbody_GetMass(uint64_t objectId)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		return rigidbody->GetMass();
	}
	return 1.0f;
}

ENGINE_API bool Rigidbody_GetIsKinematic(uint64_t objectId)
{
	if (Rigidbody* rigidbody = GetRigidbody(objectId))
	{
		return rigidbody->IsKinematic();
	}
	return false;
}