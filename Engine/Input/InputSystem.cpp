#include "pch.h"
#include "InputSystem.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI


static float ApplyLinearDeadzone(float value, float maxValue, float deadZoneSize) {
	if (value < -deadZoneSize) value += deadZoneSize;
	else if (value > deadZoneSize) value -= deadZoneSize;
	else return 0;
	//０〜１にスケーリング
	float scaledValue = value / (maxValue - deadZoneSize);
	return std::max<float>(-1.f, std::min<float>(scaledValue, 1.f));
}
static void ApplyStickDeadzone(float x, float y, DeadZoneMode deadZoneMode,
	float maxValue, float deadZoneSize, _Out_ float& resultX, _Out_ float& resultY)
{
	switch (deadZoneMode)
	{
	case DeadZoneMode::IndependentAxes:
		resultX = ApplyLinearDeadzone(x, maxValue, deadZoneSize);
		resultY = ApplyLinearDeadzone(y, maxValue, deadZoneSize);
		break;
	case DeadZoneMode::Circular:
	{
		float dist = sqrtf(x * x + y * y);
		float wanted = ApplyLinearDeadzone(dist, maxValue, deadZoneSize);
		float scale = (wanted > 0.f) ? (wanted / dist) : 0.f;
		resultX = std::max<float>(-1.f, std::min<float>(x * scale, 1.f));
		resultY = std::max<float>(-1.f, std::min<float>(y * scale, 1.f));
		break;
	}
	default://Deadzone::None
		resultX = ApplyLinearDeadzone(x, maxValue, 0);
		resultY = ApplyLinearDeadzone(y, maxValue, 0);
		break;
	}
}

void InputKey::Update(float elapsedTime)
{
	oldPressTime = pressTime;
	pressTime = (static_cast<USHORT>(GetAsyncKeyState(vKey)) & 0x8000) ? pressTime + elapsedTime : 0.0f;
}

void GamePad::Update(float elapsedTime)
{
	oldPressTime = pressTime;
	XINPUT_STATE state = InputSystem::GetXInputState();
	switch (keyType)
	{
	case KeyType::Key:
		pressTime = (state.Gamepad.wButtons & vKey) ? pressTime + elapsedTime : 0.0f;
		break;
	case KeyType::LeftTrigger:
		pressTime = (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? pressTime + elapsedTime : 0.0f;
		break;
	case KeyType::RightTrigger:
		pressTime = (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? pressTime + elapsedTime : 0.0f;
		break;
	}
}


// コンストラクタ
InputSystem::InputSystem()
{

}

//初期化
void InputSystem::Initialize()
{
	directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Up)] = std::make_unique<Keybord>('W');
	directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Left)] = std::make_unique<Keybord>('A');
	directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Down)] = std::make_unique<Keybord>('S');
	directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Right)] = std::make_unique<Keybord>('D');

	directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Up)] = std::make_unique<Keybord>('I');
	directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Left)] = std::make_unique<Keybord>('J');
	directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Down)] = std::make_unique<Keybord>('K');
	directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Right)] = std::make_unique<Keybord>('L');

#if 0
	navigationKeys[static_cast<size_t>(Direction::Up)].emplace_back(std::make_unique<Keybord>('W'));
	navigationKeys[static_cast<size_t>(Direction::Up)].emplace_back(std::make_unique<Keybord>(VK_UP));
	navigationKeys[static_cast<size_t>(Direction::Up)].emplace_back(std::make_unique<GamePad>(XINPUT_GAMEPAD_DPAD_UP));

	navigationKeys[static_cast<size_t>(Direction::Left)].emplace_back(std::make_unique<Keybord>('A'));
	navigationKeys[static_cast<size_t>(Direction::Left)].emplace_back(std::make_unique<Keybord>(VK_LEFT));
	navigationKeys[static_cast<size_t>(Direction::Left)].emplace_back(std::make_unique<GamePad>(XINPUT_GAMEPAD_DPAD_LEFT));

	navigationKeys[static_cast<size_t>(Direction::Down)].emplace_back(std::make_unique<Keybord>('S'));
	navigationKeys[static_cast<size_t>(Direction::Down)].emplace_back(std::make_unique<Keybord>(VK_DOWN));
	navigationKeys[static_cast<size_t>(Direction::Down)].emplace_back(std::make_unique<GamePad>(XINPUT_GAMEPAD_DPAD_DOWN));

	navigationKeys[static_cast<size_t>(Direction::Right)].emplace_back(std::make_unique<Keybord>('D'));
	navigationKeys[static_cast<size_t>(Direction::Right)].emplace_back(std::make_unique<Keybord>(VK_RIGHT));
	navigationKeys[static_cast<size_t>(Direction::Right)].emplace_back(std::make_unique<GamePad>(XINPUT_GAMEPAD_DPAD_RIGHT));
#endif // 0



	inputKeys.clear();

	// 全VKを事前登録
	auto registerVKey = [&](int vk) {
		if (!vKeyMap.contains(vk)) {
			auto key = std::make_unique<Keybord>(vk);
			vKeyMap[vk] = key.get();
			// アクションには紐付けず、専用リストで管理
			rawKeys.push_back(std::move(key));
		}
		};

	// アルファベット
	for (int vk = 'A'; vk <= 'Z'; ++vk) {
		registerVKey(vk);
	}
	// 数字
	for (int vk = '0'; vk <= '9'; ++vk) {
		registerVKey(vk);
	}
	// ファンクションキー
	for (int vk = VK_F1; vk <= VK_F12; ++vk) {
		registerVKey(vk);
	}
	// よく使うキー
	registerVKey(VK_SPACE);
	registerVKey(VK_RETURN);
	registerVKey(VK_ESCAPE);
	registerVKey(VK_BACK);
	registerVKey(VK_TAB);
	registerVKey(VK_SHIFT);
	registerVKey(VK_CONTROL);
	registerVKey(VK_MENU);       // Alt
	registerVKey(VK_LSHIFT);
	registerVKey(VK_RSHIFT);
	registerVKey(VK_LCONTROL);
	registerVKey(VK_RCONTROL);
	registerVKey(VK_LMENU);
	registerVKey(VK_RMENU);

	// 矢印キー
	registerVKey(VK_UP);
	registerVKey(VK_DOWN);
	registerVKey(VK_LEFT);
	registerVKey(VK_RIGHT);

	// その他
	registerVKey(VK_INSERT);
	registerVKey(VK_DELETE);
	registerVKey(VK_HOME);
	registerVKey(VK_END);
	registerVKey(VK_PRIOR);      // Page Up
	registerVKey(VK_NEXT);       // Page Down

	// マウスボタン
	registerVKey(VK_LBUTTON);
	registerVKey(VK_RBUTTON);
	registerVKey(VK_MBUTTON);


	//アクションとキーの登録
	inputKeys["attack"].emplace_back(std::make_unique<Mouse>(VK_RBUTTON));

	//inputKeys["test0"].emplace_back(std::make_unique<Mouse>(VK_LBUTTON));
	//inputKeys["test0"].emplace_back(std::make_unique<GamePad>(XINPUT_GAMEPAD_LEFT_SHOULDER));
	//inputKeys["test0"].emplace_back(std::make_unique<GamePad>(0, KeyType::LeftTrigger));

	//inputKeys["jump"].emplace_back(std::make_unique<Keybord>(VK_SPACE));

	inputKeys["ok"].emplace_back(std::make_unique<Mouse>(VK_LBUTTON));
	inputKeys["ok"].emplace_back(std::make_unique<Keybord>(VK_RETURN));

	inputKeys["Shift"].emplace_back(std::make_unique<Keybord>(VK_SHIFT));

	inputKeys["Alt"].emplace_back(std::make_unique<Keybord>(VK_MENU));
	inputKeys["Enter"].emplace_back(std::make_unique<Keybord>(VK_RETURN));
	inputKeys["Delete"].emplace_back(std::make_unique<Keybord>(VK_DELETE));

	inputKeys["Up"].emplace_back(std::make_unique<Keybord>(VK_UP));
	inputKeys["Up"].emplace_back(std::make_unique<Keybord>('W'));
	inputKeys["Left"].emplace_back(std::make_unique<Keybord>(VK_LEFT));
	inputKeys["Left"].emplace_back(std::make_unique<Keybord>('A'));
	inputKeys["Down"].emplace_back(std::make_unique<Keybord>(VK_DOWN));
	inputKeys["Down"].emplace_back(std::make_unique<Keybord>('S'));
	inputKeys["Right"].emplace_back(std::make_unique<Keybord>(VK_RIGHT));
	inputKeys["Right"].emplace_back(std::make_unique<Keybord>('D'));


	inputKeys["Space"].emplace_back(std::make_unique<Keybord>(VK_SPACE));


	inputKeys["Pause"].emplace_back(std::make_unique<Keybord>(VK_BACK));

	inputKeys["Backspace"].emplace_back(std::make_unique<Keybord>(VK_BACK));

}

//終了化
void InputSystem::Finalize()
{

}

// 更新処理
void InputSystem::Update(float elapsedTime)
{
	DWORD xinputResult = XInputGetState(static_cast<DWORD>(slot), &state);
	isGamePadConnected = (xinputResult == ERROR_SUCCESS);

	//入力情報の更新
	{
		if(isGamePadConnected)
		{
			//ゲームパッドのAxisLeft更新
			ApplyStickDeadzone(static_cast<float>(state.Gamepad.sThumbLX), static_cast<float>(state.Gamepad.sThumbLY),
				deadZoneMode, 32767.f, static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE),
				m_axis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::X)], m_axis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::Y)]);
			//ゲームパッドのAxisRight更新
			ApplyStickDeadzone(static_cast<float>(state.Gamepad.sThumbRX), static_cast<float>(state.Gamepad.sThumbRY),
				deadZoneMode, 32767.f, static_cast<float>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE),
				m_axis[static_cast<size_t>(Side::Right)][static_cast<size_t>(Axis::X)], m_axis[static_cast<size_t>(Side::Right)][static_cast<size_t>(Axis::Y)]);
		}
		else
		{
			//移動キー更新処理
			for (auto& keys : directionKeys) {
				for (auto& key : keys) {
					key->Update(elapsedTime);
				}
			}
			
			//値更新
#if 0
			ApplyStickDeadzone(
				static_cast<float>(directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Right)]->IsPressed()) -
				static_cast<float>(directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Left)]->IsPressed()),
				static_cast<float>(directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Up)]->IsPressed()) -
				static_cast<float>(directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Down)]->IsPressed()),
				deadZoneMode, 1.f, 0.f,
				m_axis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::X)], m_axis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::Y)]);
#else
			
			float right = 0.f;
			float left = 0.f;
			for (auto& key : inputKeys["Right"]) {
				if (key->IsPressed()) {
					right = 1.f;
					break;
				}
			}
			for (auto& key : inputKeys["Left"]) {
				if (key->IsPressed()) {
					left = 1.f;
					break;
				}
			}
			float up = 0.f;
			float down = 0.f;
			for (auto& key : inputKeys["Up"]) {
				if (key->IsPressed()) {
					up = 1.f;
					break;
				}
			}
			for (auto& key : inputKeys["Down"]) {
				if (key->IsPressed()) {
					down = 1.f;
					break;
				}
			}
			ApplyStickDeadzone(
				right - left,
				up - down,
				deadZoneMode, 1.f, 0.f,
				m_axis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::X)], m_axis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::Y)]);
#endif // 0


			ApplyStickDeadzone(
				static_cast<float>(directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Right)]->IsPressed()) -
				static_cast<float>(directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Left)]->IsPressed()),
				static_cast<float>(directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Up)]->IsPressed()) -
				static_cast<float>(directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Down)]->IsPressed()),
				deadZoneMode, 1.f, 0.f,
				m_axis[static_cast<size_t>(Side::Right)][static_cast<size_t>(Axis::X)], m_axis[static_cast<size_t>(Side::Right)][static_cast<size_t>(Axis::Y)]);
		}
		//ボタンの入力更新処理
		for (auto& actionKeys : inputKeys) {
			for (auto& key : actionKeys.second)	{
				key->Update(elapsedTime);
			}
		}
		// 全キーの更新処理
		for (auto& key : rawKeys) {
			key->Update(elapsedTime);
		}
	}
	// カーソル位置の取得
	POINT cursor;
	::GetCursorPos(&cursor);
	ScreenToClient(Graphics::GetHwnd(), &cursor);

	// マウス座標更新
	mousePositionX[1] = mousePositionX[0];
	mousePositionY[1] = mousePositionY[0];
#ifdef USE_IMGUI
//#if 0
	float x, y;
	Graphics::GetScreenSize(x, y);
	float left, top, right, bottom;
	Graphics::GetScreenRect(left, top, right, bottom);
	mousePositionX[0] = static_cast<int>(static_cast<float>(cursor.x - left) / (right - left) * x);
	mousePositionY[0] = static_cast<int>(static_cast<float>(cursor.y - top) / (bottom - top) * y);
#else
	// 1920x1080に収まるようにスケーリングして座標を更新
	float screenWidth, screenHeight;
	Graphics::GetScreenSize(screenWidth, screenHeight);
	mousePositionX[0] = static_cast<int>(static_cast<float>(cursor.x) / screenWidth * 1920.f);
	mousePositionY[0] = static_cast<int>(static_cast<float>(cursor.y) / screenHeight * 1080.f);
#endif // USE_IMGUI

	//アクティブデバイス判定

	//コントローラー
	{
		auto buttons = state.Gamepad.wButtons;
		auto lx = state.Gamepad.sThumbLX;
		auto ly = state.Gamepad.sThumbLY;
		// ボタンが押された or スティックが動いたらアクティブデバイスを切り替え
		if (buttons != 0 || abs(lx) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || abs(ly) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			activeDevice = InputDevice::GamePad;
		}
	}
	//キーボード
	{
		for (int vk = 0x08; vk <= 0xFE; ++vk) {
			if (GetAsyncKeyState(vk) & 0x8000) {
				activeDevice = InputDevice::Keybord;
			}
		}
	}
	//マウス
	{
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000 ||
			GetAsyncKeyState(VK_RBUTTON) & 0x8000 ||
			mousePositionX[0] != mousePositionX[1] ||
			mousePositionY[0] != mousePositionY[1]) {
			activeDevice = InputDevice::Mouse;
		}
	}
}

void InputSystem::EndFrame()
{
	//入力情報クリア
	inputChar = {};
	inputKeyDown = {};
	inputString.clear();
	keyString.clear();
	wheelDelta = 0.f;
}

LRESULT InputSystem::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_CHAR:
	{
		inputChar = wParam;
		inputString += static_cast<wchar_t>(wParam);
		break;
	}
	case WM_KEYDOWN:
	{
		inputKeyDown = wParam;
		keyString += static_cast<char>(wParam);
		break;
	}
	case WM_MOUSEWHEEL:
	{
		wheelDelta += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
		break;
	}
	default:
		break;
	}
	return 0;
}

bool InputSystem::GetInputState(const std::string& action, InputStateMask state, DeviceFlags flag)
{
	// 入力無効時は常に false
	if (!inputEnabled) return false;

	auto it = inputKeys.find(action);
	if (it != inputKeys.end())
	{
		const auto& keys = it->second;
		for (auto& key : keys) {
			switch (flag)
			{
			case DeviceFlags::KeyboardOnly:
				if (key->GetDeviceType() != InputDevice::Keybord) continue;
				break;
			case DeviceFlags::MouseOnly:
				if (key->GetDeviceType() != InputDevice::Mouse) continue;
				break;
			case DeviceFlags::GamePadOnly:
				if (key->GetDeviceType() != InputDevice::GamePad) continue;
				break;
			case DeviceFlags::KeyboardAndMouse:
				if (key->GetDeviceType() == InputDevice::GamePad) continue;
				break;
			case DeviceFlags::KeyboardAndGamePad:
				if (key->GetDeviceType() == InputDevice::Mouse) continue;
				break;
			case DeviceFlags::MouseAndGamePad:
				if (key->GetDeviceType() == InputDevice::Keybord) continue;
				break;
			}
			switch (state)
			{
			case InputStateMask::Trigger:
				if (key->IsTrigger()) return true;
				break;
			case InputStateMask::Release:
				if (key->IsRelease()) return true;
				break;
			default:
				if (key->IsPressed()) return true;
				break;
			}
		}
	}
	return false;
}

void InputSystem::RegisterKey(const std::string& action, std::unique_ptr<InputKey> key)
{
	int vKey = key->GetVKey();
	// 既に同じ vKey のキーが登録されていないかチェック
	if (!vKeyMap.contains(vKey)) {
		vKeyMap[vKey] = key.get(); // vKey と InputKey* をマッピングに登録
		rawKeys.push_back(std::move(key)); // 登録されたキーを rawKeys に保持
	}
	inputKeys[action].emplace_back(std::move(key));
}

bool InputSystem::GetKeyTrigger(int vKey)
{
	auto it = vKeyMap.find(vKey);
	return (it != vKeyMap.end()) ? it->second->IsTrigger() : false;
}

bool InputSystem::GetKeyRelease(int vKey)
{
	auto it = vKeyMap.find(vKey);
	return (it != vKeyMap.end()) ? it->second->IsRelease() : false;
}

float InputSystem::GetAxis(Side side, Axis axis)
{
	return m_axis[static_cast<size_t>(side)][static_cast<size_t>(axis)];
}

int InputSystem::GetAxisRaw(Side side, Axis axis)
{
	return static_cast<int>(round(GetAxis(side, axis)));
}

Direction InputSystem::GetAxisDirection()
{
	int ax = GetAxisRaw(Side::Left, Axis::X);
	int ay = GetAxisRaw(Side::Left, Axis::Y);
	if (ax == 1) return Direction::Right;
	if (ax == -1) return Direction::Left;
	if (ay == 1) return Direction::Up;
	if (ay == -1) return Direction::Down;
	return Direction::None;
}

void InputSystem::GetMouseDelta(int& x, int& y)
{
	x = mousePositionX[0] - mousePositionX[1];
	y = mousePositionY[0] - mousePositionY[1];
}

int InputSystem::GetMousePositionX()
{
	return mousePositionX[0];
}

int InputSystem::GetMousePositionY()
{
	return mousePositionY[0];
}

void InputSystem::GetMousePosition(float position[2])
{
	position[0] = static_cast<float>(mousePositionX[0]);
	position[1] = static_cast<float>(mousePositionY[0]);
}

Vector2 InputSystem::GetMousePosition()
{
	return Vector2(static_cast<float>(mousePositionX[0]), static_cast<float>(mousePositionY[0]));
}

int InputSystem::GetOldMousePositionX()
{
	return mousePositionX[1];
}

int InputSystem::GetOldMousePositionY()
{
	return mousePositionY[1];
}

float InputSystem::GetWheelDelta()
{
	return wheelDelta;
}

void InputSystem::SetCursorLock(bool lock, bool changeVisible)
{
	cursolLock = lock;
	if (changeVisible)
		SetCursorVisible(!lock);
}

bool InputSystem::IsCursorLock()
{
	return cursolLock;
}

bool InputSystem::IsCursorVisible()
{
	return cursolVisible;
}