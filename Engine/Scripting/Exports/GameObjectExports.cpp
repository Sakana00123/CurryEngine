#include "pch.h"
#include "Engine/Core/GameObject.h"

// GameObject クラスのメソッドをスクリプトから呼び出せるようにするためのエクスポート関数


// -------- GameObject のプロパティアクセス関数 ---------

// --------- Name ---------

ENGINE_API const char* GameObject_GetName(uint64_t objectId)
{
	if (GameObject* obj = ObjectManager::Find(ObjectId::FromValue(objectId)))
	{
		return obj->name.c_str();
	}
	return ""; // オブジェクトが見つからない場合は空文字を返す
}

ENGINE_API void GameObject_SetName(uint64_t objectId, const char* name)
{
	if (GameObject* obj = ObjectManager::Find(ObjectId::FromValue(objectId)))
	{
		obj->name = name;
	}
}

// --------- Valid ---------

ENGINE_API int Entity_IsValid(uint64_t objectId)
{
	if (GameObject* obj = ObjectManager::Find(ObjectId::FromValue(objectId)))
	{
		return 1; // オブジェクトが見つかった場合は 1 を返す
	}
	return 0; // オブジェクトが見つからない場合は 0 を返す
}

// --------- HasComponent ---------

ENGINE_API int Entity_HasComponent(uint64_t objectId, const char* componentName)
{
	if (GameObject* obj = ObjectManager::Find(ObjectId::FromValue(objectId)))
	{
		for (const auto& comp : obj->GetAllComponents())
		{
			if (!comp) continue; // コンポーネントが無効な場合はスキップ
			bool equal = strcmp(componentName, comp->name.c_str()) == 0; // 名前が一致するかを比較

			if (equal)
			{
				return 1; // コンポーネントが見つかった場合は 1 を返す
			}
		}
	}
	return 0; // オブジェクトやコンポーネントが見つからない場合は 0 を返す
}


// --------- Destroy ---------

ENGINE_API void Entity_Destroy(uint64_t objectId)
{
	if (GameObject* obj = ObjectManager::Find(ObjectId::FromValue(objectId)))
	{
		obj->Destroy();
	}
}

// --------- GetComponent ---------

ENGINE_API int GameObject_GetComponentIds(uint64_t ownerId, const char* componentName, uint64_t* outBuffer, int bufferSize)
{
	if (GameObject* obj = ObjectManager::Find(ObjectId::FromValue(ownerId)))
	{
		std::vector<uint64_t> componentIds;
		bool returnAll = componentName == nullptr;
		if (componentName && componentName[0] == '\0') {
			returnAll = true; // 空文字もすべてのコンポーネントを返す条件とする
		}
		for (const auto& comp : obj->GetAllComponents())
		{
			if (!comp) continue; // コンポーネントが無効な場合はスキップ
			if (returnAll) {
				componentIds.push_back(comp->id.Value());
				continue; // すべてのコンポーネントを返す場合は、名前の比較をスキップしてIDを追加
			}
			std::string compName = comp->GetTypeName();
			bool equal = strcmp(componentName, compName.c_str()) == 0; // 名前が一致するかを比較
			// componentName が null または空文字の場合はすべてのコンポーネントIDを返す
			if (equal || returnAll)
			{
				componentIds.push_back(comp->id.Value());
			}
		}
		int count = min(static_cast<int>(componentIds.size()), bufferSize);
		for (int i = 0; i < count; ++i)
		{
			outBuffer[i] = componentIds[i];
		}
		return count; // 見つかったコンポーネントの数を返す
	}
	return 0; // オブジェクトが見つからない場合は 0 を返す
}


// --------- ActiveSelf ---------


// --------- Active ---------

ENGINE_API void GameObject_SetActive(uint64_t objectId, bool active)
{
	if (GameObject* obj = ObjectManager::Find(ObjectId::FromValue(objectId)))
	{
		obj->SetActive(active);
	}
}

ENGINE_API bool GameObject_IsActive(uint64_t objectId)
{
	if (GameObject* obj = ObjectManager::Find(ObjectId::FromValue(objectId)))
	{
		return obj->IsActive();
	}
	return false; // オブジェクトが見つからない場合は 0 を返す
}

// --------- Layer ---------


// ---------------- Instantiate ----------------

ENGINE_API uint64_t GameObject_InstantiateFromId(uint64_t prefabId, uint64_t parentId, Vector3 position, Quaternion rotation)
{
	try
	{
		ObjectId objId = ObjectId::FromValue(prefabId);
		GameObject* original = ObjectManager::Find(objId);
		assert(original && "Original GameObject not found for instantiation."); // デバッグ用のアサーション
		if (!original) return 0; // オブジェクトが見つからない場合は 0 を返す
		ObjectId parentObjId = ObjectId::FromValue(parentId);
		GameObject* parent = parentObjId.IsValid() ? ObjectManager::Find(ObjectId::FromValue(parentId)) : nullptr;
		GameObject* instance = Component::Instantiate(original, parent ? parent->transform : nullptr, position, rotation);
		assert(instance && "Failed to instantiate GameObject."); // デバッグ用のアサーション
		return instance ? instance->GetId().Value() : 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
		return 0; // JSONの解析に失敗した場合は 0 を返す
	}
}

ENGINE_API uint64_t GameObject_InstantiateFromResource(const char* resourcePath, uint64_t parentId, Vector3 position, Quaternion rotation)
{
	try
	{
		ObjectId parentObjId = ObjectId::FromValue(parentId);
		GameObject* parent = parentObjId.IsValid() ? ObjectManager::Find(ObjectId::FromValue(parentId)) : nullptr;
		GameObject* instance = Component::Instantiate(resourcePath, parent ? parent->transform : nullptr, position, rotation);
		return instance ? instance->GetId().Value() : 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
		return 0; // JSONの解析に失敗した場合は 0 を返す
	}
}