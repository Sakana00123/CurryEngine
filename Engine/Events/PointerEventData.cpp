#include "pch.h"
#include "PointerEventData.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

void PointerEventData::SetPointerEnter(GameObject* obj)
{
	pointerEnter = obj;
	pointerEnterId = (obj != nullptr) ? obj->GetId() : ObjectId::Invalid();
}

void PointerEventData::SetPointerPress(GameObject* obj)
{
	pointerPress = obj;
	pointerPressId = (obj != nullptr) ? obj->GetId() : ObjectId::Invalid();
}

void PointerEventData::SetLastPress(GameObject* obj)
{
	lastPress = obj;
	lastPressId = (obj != nullptr) ? obj->GetId() : ObjectId::Invalid();
}

void PointerEventData::SetPointerDrag(GameObject* obj)
{
	pointerDrag = obj;
	pointerDragId = (obj != nullptr) ? obj->GetId() : ObjectId::Invalid();
}


GameObject* PointerEventData::GetPointerEnter() const
{
	Scene* scene = SceneManager::GetCurrentScene();
	if (scene == nullptr) return nullptr; // シーンが存在しない場合は nullptr を返す

	if (pointerEnterId.IsValid()) {
		GameObject* obj = scene->FindGameObjectById(pointerEnterId);
		if (obj != nullptr) {
			return obj;
		}
	}
	return nullptr;
}

GameObject* PointerEventData::GetPointerPress() const
{
	Scene* scene = SceneManager::GetCurrentScene();
	if (scene == nullptr) return nullptr; // シーンが存在しない場合は nullptr を返す
	if (pointerPressId.IsValid()) {
		GameObject* obj = scene->FindGameObjectById(pointerPressId);
		if (obj != nullptr) {
			return obj;
		}
	}
	return nullptr;
}

GameObject* PointerEventData::GetLastPress() const
{
	Scene* scene = SceneManager::GetCurrentScene();
	if (scene == nullptr) return nullptr; // シーンが存在しない場合は nullptr を返す
	if (lastPressId.IsValid()) {
		GameObject* obj = scene->FindGameObjectById(lastPressId);
		if (obj != nullptr) {
			return obj;
		}
	}
	return nullptr;
}

GameObject* PointerEventData::GetPointerDrag() const
{
	Scene* scene = SceneManager::GetCurrentScene();
	if (scene == nullptr) return nullptr; // シーンが存在しない場合は nullptr を返す
	if (pointerDragId.IsValid()) {
		GameObject* obj = scene->FindGameObjectById(pointerDragId);
		if (obj != nullptr) {
			return obj;
		}
	}
	return nullptr;
}