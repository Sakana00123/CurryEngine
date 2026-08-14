#include "pch.h"
#include "Engine/Core/Component.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Core/ObjectManager.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Scenes/Scene.h"

// Component クラスのメソッドをスクリプトから呼び出せるようにするためのエクスポート関数

// -------- Component のプロパティアクセス関数 ---------

static std::shared_ptr<Component> FindComponentById(uint64_t componentId)
{
	Scene* currentScene = SceneManager::GetLoadingSceneOrCurrentScene();
	if (!currentScene) return nullptr; // シーンが存在しない場合は nullptr を返す
	auto& cache = currentScene->objectManager->GetComponentCacheMap();
	auto it = cache.find(ObjectId::FromValue(componentId));
	if (it != cache.end())
	{
		return it->second.lock(); // weak_ptr を shared_ptr に変換して返す
	}
	return nullptr; // コンポーネントが見つからない場合は nullptr を返す
}

// --------- Enable ---------

ENGINE_API bool Component_IsValid(uint64_t objectId)
{
	if (auto comp = FindComponentById(objectId))
	{
		return true; // コンポーネントが見つかれば有効
	}
	LOG_WARNING(std::format("Component_IsValid: Component with ID %llu not found.", objectId));
	return false; // コンポーネントが見つからない場合は無効
}


ENGINE_API int Component_GetEnable(uint64_t objectId)
{
	if (auto comp = FindComponentById(objectId))
	{
		return comp->IsEnabled() ? 1 : 0; // 有効なら 1、無効なら 0 を返す
	}
	LOG_WARNING(std::format("Component_GetEnable: Component with ID %llu not found.", objectId));
	return 0; // オブジェクトやコンポーネントが見つからない場合は 0 を返す
}

ENGINE_API void Component_SetEnable(uint64_t objectId, int enable)
{
	if (auto comp = FindComponentById(objectId))
	{
		comp->SetEnabled(enable != 0); // enable が 0 でなければ有効にする
	}
	LOG_WARNING(std::format("Component_SetEnable: Component with ID %llu not found.", objectId));
}

ENGINE_API uint64_t Component_GetOwner(uint64_t objectId)
{
	if (auto comp = FindComponentById(objectId))
	{
		if (GameObject* owner = comp->GetOwner())
		{
			return owner->GetId().Value(); // 所有者の ID を返す
		}
	}
	LOG_WARNING(std::format("Component_GetOwner: Component with ID %llu not found or has no owner.", objectId));
	return 0; // オブジェクトやコンポーネントが見つからない場合は 0 を返す
}

ENGINE_API void Component_Destroy(uint64_t objectId)
{
	if (auto comp = FindComponentById(objectId))
	{
		comp->Destroy(); // コンポーネントを破棄する
	}
	else
	{
		LOG_WARNING(std::format("Component_Destroy: Component with ID %llu not found.", objectId));
	}
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
