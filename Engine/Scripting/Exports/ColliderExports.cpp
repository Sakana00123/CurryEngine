#include "pch.h"
#include "Engine/Physics/Collider.h"



ENGINE_API void Collider_SetIsTrigger(uint64_t ownerId, uint64_t componentId, bool isTrigger)
{
	if (auto object = ObjectManager::Find(ObjectId::FromValue(ownerId)))
	{
		for (auto& component : object->GetComponents<Collider>())
		{
			if (component->GetId().Value() == componentId)
			{
				component->SetTrigger(isTrigger);
				return;
			}
		}
	}
}

//ENGINE_API void Collider_SetMaterial(uint64_t ownerId, uint64_t componentId, MaterialHandle materialHandle)
//{
//	if (auto object = ObjectManager::Find(ObjectId::FromValue(ownerId)))
//	{
//		for (auto& component : object->GetComponents<Collider>())
//		{
//			if (component->GetId().Value() == componentId)
//			{
//				component->SetMaterial(materialHandle);
//				break;
//			}
//		}
//	}
//}

ENGINE_API bool Collider_GetIsTrigger(uint64_t ownerId, uint64_t componentId)
{
	if (auto object = ObjectManager::Find(ObjectId::FromValue(ownerId)))
	{
		for (auto& component : object->GetComponents<Collider>())
		{
			if (component->GetId().Value() == componentId)
			{
				return component->IsTrigger();
			}
		}
	}
	return false; // デフォルトは false を返す
}