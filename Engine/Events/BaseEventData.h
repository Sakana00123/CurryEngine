#pragma once
class BaseInputModule;
class GameObject;
class EventSystem;

class AbstractEventData
{
protected:
	bool used = false;
public:
	void Use() { used = true; }
	bool IsUsed() const { return used; }

	virtual void Reset() = 0;
};

class BaseEventData : public AbstractEventData
{
protected:
	EventSystem* eventSystem;
	GameObject* selectedObject = nullptr;
public:
	BaseEventData(EventSystem* eventSystem) : eventSystem(eventSystem) {}
	EventSystem* GetEventSystem() const { return eventSystem; }

	void Reset() override { selectedObject = nullptr; }
};