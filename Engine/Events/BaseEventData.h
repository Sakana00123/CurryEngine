#pragma once
class BaseInputModule;
class GameObject;
class EventSystem;

/** @brief AbstractEventData を表すクラスです。 */
class AbstractEventData
{
protected:
	bool used = false;
public:
	void Use() { used = true; }
	bool IsUsed() const { return used; }

	/**
	 * @brief 状態を初期値に戻します。
	 */
	virtual void Reset() = 0;
};

/** @brief BaseEventData を表すクラスです。 */
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