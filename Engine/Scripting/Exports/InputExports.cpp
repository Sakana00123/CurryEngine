#include "pch.h"
#include "Engine/Input/InputSystem.h"


// ================= Keyboard 直打ち =================

ENGINE_API bool Input_GetKey(int vKey)
{
	if (!InputSystem::IsInputEnabled()) return false;
	return (GetAsyncKeyState(vKey) & 0x8000) != 0;
}

ENGINE_API bool Input_GetKeyDown(int vKey)
{
	if (!InputSystem::IsInputEnabled()) return false;
	return InputSystem::GetKeyTrigger(vKey);
}

ENGINE_API bool Input_GetKeyUp(int vKey)
{
	if (!InputSystem::IsInputEnabled()) return false;
	return InputSystem::GetKeyRelease(vKey);
}

// ================= アクション名指定 =================

ENGINE_API bool Input_GetAction(const char* action)
{
	if (!InputSystem::IsInputEnabled()) return false;
	return InputSystem::GetInputState(action, InputStateMask::None, DeviceFlags::All);
}

ENGINE_API bool Input_GetActionDown(const char* action)
{
	if (!InputSystem::IsInputEnabled()) return false;
	return InputSystem::GetInputState(action, InputStateMask::Trigger, DeviceFlags::All);
}

ENGINE_API bool Input_GetActionUp(const char* action)
{
	if (!InputSystem::IsInputEnabled()) return false;
	return InputSystem::GetInputState(action, InputStateMask::Release, DeviceFlags::All);
}

// ================= スティック・マウス座標 =================

// マウスのバーチャルキー(左/右/中ボタン、サイドボタン)の配列
//static const int mouseVKeys[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
//
//ENGINE_API bool Input_GetMouseButton(int button)
//{
//	if (!InputSystem::IsInputEnabled()) return false;
//	if (button <= 0 || button >= static_cast<int>(std::size(mouseVKeys))) return false;
//	return GetAsyncKeyState(mouseVKeys[button]) & 0x8000 != 0;
//}
//
//ENGINE_API bool Input_GetMouseButtonDown(int button)
//{
//	if (!InputSystem::IsInputEnabled()) return false;
//	if (button < 0 || button >= static_cast<int>(std::size(mouseVKeys))) return false;
//	return InputSystem::GetKeyTrigger(mouseVKeys[button]);
//}
//
//ENGINE_API bool Input_GetMouseButtonUp(int button)
//{
//	if (!InputSystem::IsInputEnabled()) return false;
//	if (button < 0 || button >= static_cast<int>(std::size(mouseVKeys))) return false;
//	return InputSystem::GetKeyRelease(mouseVKeys[button]);
//}


ENGINE_API float Input_GetAxis(int side, int axis)
{
	if (!InputSystem::IsInputEnabled()) return 0.f;
	return InputSystem::GetAxis(static_cast<Side>(side), static_cast<Axis>(axis));
}

ENGINE_API int Input_GetAxisRaw(int side, int axis)
{
	if (!InputSystem::IsInputEnabled()) return 0;
	return InputSystem::GetAxisRaw(static_cast<Side>(side), static_cast<Axis>(axis));
}

ENGINE_API int Input_GetMouseDeltaX()
{
	if (!InputSystem::IsInputEnabled()) return 0;
	int x, y;
	InputSystem::GetMouseDelta(x, y);
	return x;
}

ENGINE_API int Input_GetMouseDeltaY()
{
	if (!InputSystem::IsInputEnabled()) return 0;
	int x, y;
	InputSystem::GetMouseDelta(x, y);
	return y;
}

ENGINE_API int Input_GetMousePositionX()
{
	if (!InputSystem::IsInputEnabled()) return 0;
	return InputSystem::GetMousePositionX();
}

ENGINE_API int Input_GetMousePositionY()
{
	if (!InputSystem::IsInputEnabled()) return 0;
	return InputSystem::GetMousePositionY();
}

ENGINE_API float Input_GetMouseScrollDeltaX()
{
	if (!InputSystem::IsInputEnabled()) return 0.f;
	return InputSystem::GetWheelDelta();
}

// ================= カーソル制御 =================

ENGINE_API void Input_SetCursorLock(bool lock, bool changeVisible)
{
	InputSystem::SetCursorLock(lock, changeVisible);
}

ENGINE_API bool Input_GetCursorLock()
{
	return InputSystem::IsCursorLock();
}

ENGINE_API bool Input_GetCursorVisible()
{
	return InputSystem::IsCursorVisible();
}

// ================= デバイス状態 =================

ENGINE_API bool Input_IsGamepadConnected()
{
	return InputSystem::IsGamepadConnected();
}