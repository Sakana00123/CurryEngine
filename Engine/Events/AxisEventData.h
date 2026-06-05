#pragma once
#include <DirectXMath.h>
#include "BaseEventData.h"
struct RaycastResult;

enum MoveDirection {
	Up,
	Left,
	Down,
	Right,
	None
};

class AxisEventData : public BaseEventData
{
public:
	MoveDirection moveDir = MoveDirection::None;
public:
	AxisEventData(EventSystem* eventSystem) : BaseEventData(eventSystem) {}
};