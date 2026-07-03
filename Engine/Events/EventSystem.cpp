#include "pch.h"
#include "EventSystem.h"
#include "Engine/UI/GraphicRaycaster.h"
#include "BaseInputModule.h"
#include "InputModule.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Core/GameObject.h"
#include "EventHandlers.h"

EventSystem EventSystem::current;

void EventSystem::SetSelectedGameObject(GameObject* obj)
{
	currentSelectedGameObject = GetSelectedGameObject(); // 現在の選択オブジェクトを更新

	if (currentSelectedGameObject != obj) {
		if (auto inputModule = activeModule.lock())
		{
			if (currentSelectedGameObject)
			{
				// 選択解除イベントを発行
				inputModule->ExecuteEvent<IDeselectHandler>(currentSelectedGameObject, nullptr, &IDeselectHandler::Execute);
			}
			if (obj)
			{
				// 選択イベントを発行
				inputModule->ExecuteEvent<ISelectHandler>(obj, nullptr, &ISelectHandler::Execute);
			}
		}
	}
	currentSelectedGameObject = obj;
	currentSelectedGameObjectId = obj ? obj->GetId() : ObjectId::Invalid();
}

GameObject* EventSystem::GetSelectedGameObject()
{
	Scene* scene = SceneManager::GetCurrentScene();
	if (scene == nullptr) return nullptr; // シーンが存在しない場合は nullptr を返す

	if (currentSelectedGameObjectId.IsValid()) {
		GameObject* obj = scene->FindGameObjectById(currentSelectedGameObjectId);
		if (obj != nullptr) {
			return obj;
		}
	}
	return nullptr;
}

void EventSystem::Update(float elapsedTime) {
	//破棄処理
	auto& erases = GetCurrent()->erases;
	auto& raycasters = GetCurrent()->raycasters;
	auto& activeModule = GetCurrent()->activeModule;
	if (!erases.empty()) {
		raycasters.erase(std::remove_if(raycasters.begin(), raycasters.end(),
			[&](const auto& raycaster) {
				return std::any_of(erases.begin(), erases.end(),
					[&](const auto& erase) {
						return raycaster.lock() == erase.lock();
					});
			}),
			raycasters.end());
		erases.clear();
	}
	//InputModuleの更新
	if (!activeModule.expired()) {
		activeModule.lock()->Process(elapsedTime);
	}
	else {
		activeModule.reset();
	}
}

RaycastResult EventSystem::RaycastAll() {
	RaycastResult result{};
	std::vector<RaycastResult> results{};

	if (!current.activeModule.expired())
	{
		for (auto& raycaster : GetCurrent()->raycasters)
		{
			if (auto raycasterPtr = raycaster.lock()) {
				if (raycasterPtr->IsEnabled()) {
					raycasterPtr->Raycast(current.activeModule.lock()->GetEventData(), results);
				}
			}
		}
	}

	if (results.size() > 0) {
		result = results.back();//一番最後に描画されるUIがヒット対象
	}
	return result;
}

EventId EventSystem::nextEventId = 1;
EventId EventSystem::Register(Handler handler)
{
	EventId id = nextEventId++;
	GetCurrent()->eventHandlers[id] = handler;
	return id;
}

void EventSystem::Invoke(EventId id)
{
	auto& handlers = GetCurrent()->eventHandlers;
	auto it = handlers.find(id);
	if (it != handlers.end()) {
		it->second();
	}
}