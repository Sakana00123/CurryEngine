#include "pch.h"
#include "Engine/Core/Component.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Core/ObjectManager.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Scenes/Scene.h"

// Component クラスのメソッドをスクリプトから呼び出せるようにするためのエクスポート関数

// -------- Component のプロパティアクセス関数 ---------

// --------- Enable ---------

ENGINE_API int Component_GetEnable(uint64_t objectId)
{
	if (GameObject* obj = ObjectManager::Find(ObjectId::FromValue(objectId)))
	{
		for (const auto& comp : obj->GetAllComponents())
		{
			if (comp && comp->id.Value() == objectId)
			{
				return comp->IsEnabled() ? 1 : 0; // 有効なら 1、無効なら 0 を返す
			}
		}
	}
	return 0; // オブジェクトやコンポーネントが見つからない場合は 0 を返す
}

ENGINE_API void Component_SetEnable(uint64_t objectId, int enable)
{
	if (GameObject* obj = ObjectManager::Find(ObjectId::FromValue(objectId)))
	{
		for (const auto& comp : obj->GetAllComponents())
		{
			if (comp && comp->id.Value() == objectId)
			{
				comp->SetEnabled(enable != 0); // 0 以外は有効とみなす
				return;
			}
		}
	}
}

ENGINE_API uint64_t Component_GetOwner(uint64_t objectId)
{
	Scene* currentScene = SceneManager::GetLoadingSceneOrCurrentScene();
	if (!currentScene) return 0; // シーンが存在しない場合は 0 を返す
	auto& cache = currentScene->objectManager->GetComponentCacheMap();
	if (cache.find(ObjectId::FromValue(objectId)) != cache.end())
	{
		auto& comp = cache.at(ObjectId::FromValue(objectId));
		if (comp.lock())
		{
			GameObject* owner = comp.lock()->GetOwner();
			return owner ? owner->GetId().Value() : 0; // 所有者が存在すればそのIDを返し、存在しなければ0を返す
		}
	}
	return 0; // オブジェクトやコンポーネントが見つからない場合は 0 を返す
}

//ENGINE_API uint64_t Component_InstantiateFromId(uint64_t objectId, uint64_t parentId, Vector3 position, Quaternion rotation)
//{
//	if (GameObject* prefab = ObjectManager::Find(ObjectId::FromValue(objectId)))
//	{
//		ObjectId parentObjId = ObjectId::FromValue(parentId);
//		GameObject* parent = parentObjId.IsValid() ? ObjectManager::Find(ObjectId::FromValue(parentId)) : nullptr;
//		GameObject* instance = Component::Instantiate(prefab, parent ? parent->transform : nullptr, position, rotation);
//		return instance ? instance->GetId().Value() : 0;
//	}
//	return 0; // オブジェクトが見つからない場合は 0 を返す
//
//}