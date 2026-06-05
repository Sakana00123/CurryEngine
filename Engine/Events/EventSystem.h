#pragma once
#include "Engine/Input/InputSystem.h"
#include "Engine/Core/ObjectId.h"
#include <vector>
#include <memory>
#include <string>
#include <Windows.h>
#include <DirectXMath.h>
class InputModule;
class GraphicRaycaster;
class GameObject;
struct RaycastResult;

// ƒCƒxƒ“ƒgID‚ÌŒ^’è‹`
using EventId = uint32_t;

class EventSystem
{
	static EventSystem current;
	static std::vector<EventSystem> eventSystems;
	
	GameObject* currentSelectedGameObject = nullptr;
	ObjectId currentSelectedGameObjectId = ObjectId::Invalid();
	std::weak_ptr<InputModule> activeModule;
	std::vector<std::weak_ptr<GraphicRaycaster>> raycasters;
	std::vector<std::weak_ptr<GraphicRaycaster>> erases;
public:

	void SetSelectedGameObject(GameObject* obj);
	GameObject* GetSelectedGameObject();

	void Reset() {
		currentSelectedGameObject = nullptr;
		activeModule.reset();
		raycasters.clear();
		erases.clear();
	}

	static EventSystem* GetCurrent() { return &current; }

	static void Update(float elapsedTime);

	static RaycastResult RaycastAll();

	static void RegisterGraphicRaycaster(std::shared_ptr<GraphicRaycaster> raycaster)
	{
		// d•¡“o˜^‚ğ–h~
		if (raycaster == nullptr) {
			return; // –³Œø‚ÈƒŒƒCƒLƒƒƒXƒ^[‚Í“o˜^‚µ‚È‚¢
		}
		for (auto& existingRaycaster : GetCurrent()->raycasters) {
			if (existingRaycaster.lock() == raycaster) {
				return; // Šù‚É“o˜^‚³‚ê‚Ä‚¢‚éê‡‚Í‰½‚à‚µ‚È‚¢
			}
		}
		// “o˜^
		GetCurrent()->raycasters.emplace_back(raycaster);
	}
	static void UnregisterGraphicRaycaster(std::shared_ptr<GraphicRaycaster> raycaster)
	{
		// ”jŠüƒŠƒXƒg‚É’Ç‰Á
		GetCurrent()->erases.emplace_back(raycaster);
	}

	static void RegisterInputModule(std::shared_ptr<InputModule> inputModule) {
		GetCurrent()->activeModule = inputModule;
	}
	static void UnregisterInputModule() {
		GetCurrent()->activeModule.reset();
	}

public:
	// ƒCƒxƒ“ƒgƒnƒ“ƒhƒ‰‚ÌŒ^’è‹`
	using Handler = std::function<void()>;

	// ƒCƒxƒ“ƒg“o˜^A”­s
	static EventId Register(Handler handler);
	static void Invoke(EventId id);

private:
	std::unordered_map<EventId, Handler> eventHandlers;
	static EventId nextEventId;
};