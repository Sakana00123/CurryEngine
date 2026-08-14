#include "pch.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/SceneManager.h"


ENGINE_API int Scene_GetAllIds(uint64_t* outBuffer, int bufferSize)
{
	const auto& allObjects = SceneManager::GetCurrentScene()->GetAllSceneObjects();
	int count = min(static_cast<int>(allObjects.size()), bufferSize);
	for (int i = 0; i < count; ++i)
	{
		outBuffer[i] = allObjects[i]->GetId().Value();
	}
	return count; // 見つかったオブジェクトの数を返す
}

ENGINE_API int Scene_FindAllByType(const char* typeName, uint64_t* outBuffer, int bufferSize)
{
	const auto& allObjects = SceneManager::GetCurrentScene()->GetAllSceneObjects();
	int count = 0;
	for (const auto& obj : allObjects)
	{
		bool equal = false;
		for (const auto& comp : obj->GetAllComponents())
		{
			if (!comp) continue; // コンポーネントが無効な場合はスキップ
			std::string compName = comp->GetTypeName();
			equal = strcmp(typeName, compName.c_str()) == 0; // 名前が一致するかを比較
			if (equal)
			{
				break; // 一致するコンポーネントが見つかったらループを抜ける
			}
		}
		if (equal)
		{
			if (count < bufferSize)
			{
				outBuffer[count] = obj->GetId().Value();
			}
			count++;
		}
	}
	return count; // 見つかったオブジェクトの数を返す
}
