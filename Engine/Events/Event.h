#pragma once
#include "Engine/Core/Component.h"
#include <functional>

class Event : public Component
{
	bool oneShot;
	bool triggered;
	float time;
	float delay;

	std::function<bool()> condition;					// 起動条件
	std::function<bool(GameObject*)> conditionInThis;   // 起動条件
	std::function<void()> onTriggered;					// 条件を満たしたときの処理
	std::function<void(GameObject*)> onTriggeredThis;   // 条件を満たしたときの処理
public:
	
	Event(bool oneShot = false, float delay = 0.0f) : oneShot(oneShot), delay(delay), time(0.f), triggered(false) {};

	virtual ~Event() override {};

	void Update(float elapsedTime) {
		if ((!condition && !conditionInThis) || (!onTriggered && !onTriggeredThis)) return;
		if (oneShot && triggered) return;

		//条件を満たしていて、かつ遅延が設定されている時
		if (time > 0) {
			time -= elapsedTime;
			if (time <= 0) {
				//起動
				OnTrigger();
				time = 0.f;
			}
		}

		//条件を満たしているか判定
		if (condition ? condition() : conditionInThis(this->gameObject)) {
			//遅延が設定されていたら
			if (delay > 0) {
				time = delay;
				return;
			}
			//起動
			OnTrigger();
		}
	}

	//起動条件を設定
	template<class T>
	void SetTriggerCondition(bool (T::* func)(void), T* instance) {
		condition = std::bind(func, instance);
	}
	//起動条件を設定
	void SetTriggerCondition(bool (*func)()) {
		condition = func;
	}
	//起動条件を設定
	void SetTriggerCondition(bool (*func)(GameObject* object)) {
		conditionInThis = std::bind(func, this->gameObject);
	}
	//条件を満たした時のイベントを設定
	template<class T>
	void SetOnTriggeredEvent(void(T::* func)(void), T* instance) {
		onTriggered = std::bind(func, instance);
	}
	//条件を満たした時のイベントを設定
	void SetOnTriggeredEvent(void (*func)()) {
		onTriggered = func;
	}
	//条件を満たした時のイベントを設定
	void SetOnTriggeredEvent(void (*func)(GameObject* object)) {
		onTriggeredThis = std::bind(func, this->gameObject);
	}

private:
	void OnTrigger() {
		onTriggered ? onTriggered() : onTriggeredThis(this->gameObject);
		triggered = true;
	}
};