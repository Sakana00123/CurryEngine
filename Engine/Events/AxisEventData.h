#pragma once
#include <DirectXMath.h>
#include "BaseEventData.h"
struct RaycastResult;

/** @brief MoveDirection を表す列挙型です。 */
enum MoveDirection {
	Up,
	Left,
	Down,
	Right,
	None
};

/** @brief AxisEventData を表すクラスです。 */
class AxisEventData : public BaseEventData
{
public:
	MoveDirection moveDir = MoveDirection::None;
public:
	AxisEventData(EventSystem* eventSystem) : BaseEventData(eventSystem) {}
};