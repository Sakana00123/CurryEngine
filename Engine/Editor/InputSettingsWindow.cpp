#include "pch.h"
#include "InputSettingsWindow.h"
#include "Engine/Input/InputSystem.h"

namespace CurryEngine::Editor
{
	void InputSettingsWindow::Show()
	{
		isOpen = true;
	}
	void InputSettingsWindow::Close()
	{
		isOpen = false;
	}
	void InputSettingsWindow::Draw()
	{
		if (!isOpen) return;
#ifdef USE_IMGUI
		if (ImGui::Begin("Input Settings", &isOpen))
		{
			// アクションキーの一覧を表示し、選択して編集や、追加、削除できるようにする
			for (const auto& [action, keys] : InputSystem::GetInputKeys())
			{
				ImGui::Text("Action: %s", action.c_str());
				for (const auto& key : keys)
				{
					ImGui::Text("  Key: %d, Device: %d", key->GetVKey(), static_cast<int>(key->GetDeviceType()));
				}
				ImGui::Separator();
			}
			// 追加・削除のボタン
			if (ImGui::Button("Add Action"))
			{
				// 新しいアクションを追加する処理
				ImGui::OpenPopup("AddActionPopup");
			}
			if (ImGui::IsPopupOpen("AddActionPopup"))
			{
				if (ImGui::BeginPopup("AddActionPopup"))
				{
					static char newActionName[128] = "";
					ImGui::InputText("Action Name", newActionName, IM_ARRAYSIZE(newActionName));
					static int newActionDevice = 0;
					static const char* deviceTypes[] = { "Keyboard", "Mouse", "GamePad" };
					ImGui::Combo("Device", &newActionDevice, deviceTypes, IM_ARRAYSIZE(deviceTypes));

					static int newActionKey = 0;

					switch (static_cast<InputDevice>(newActionDevice))
					{
						case InputDevice::Keybord:
						{
							ImGui::Text("Keyboard Key");
							ImGui::SameLine();
							ImGui::InputInt("##Key (vKey)", &newActionKey);
							ImGui::SameLine();
							// キャプチャするキーを押すとその vKey を取得する
							static bool captureKey = false;
							if (captureKey)
							{
								// キー入力をキャプチャする処理
								if (InputSystem::inputKeyDown != 0)
								{
									newActionKey = InputSystem::inputKeyDown;
									captureKey = false;
								}
							}
							std::string buttonLabel = captureKey ? "Press a key..." : "Capture Key";
							if (ImGui::Button(buttonLabel.c_str()))
							{
								captureKey = true;
							}
							if (ImGui::IsItemHovered())
							{
								std::u8string tooltipText = u8"Capture Keyを押すと、次に押したキーのvKeyが取得されます。";
								ImGui::SetTooltip("Tips: %s", tooltipText.c_str());
							}
							break;
						}
						case InputDevice::Mouse:
						{
							static const char* mouseButtons[] = { "Left", "Right", "Middle", "X1", "X2" };
							static int mouseButtonVKeys[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
							static int selectedMouseButton = 0;
							ImGui::Text("Mouse Button");
							ImGui::SameLine();
							ImGui::Combo("##Mouse Button", &selectedMouseButton, mouseButtons, IM_ARRAYSIZE(mouseButtons));
							newActionKey = mouseButtonVKeys[selectedMouseButton]; // 選択されたマウスボタンの vKey を設定
							break;
						}
						case InputDevice::GamePad:
						{
							static const char* gamepadButtons[] = { "A", "B", "X", "Y", 
								"LB", "RB",
								"Back", "Start",
								"LS", "RS" };
							static int gamepadButtonVKeys[] = { 
								XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
								XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
								XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_START,
								XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB };
							static int selectedGamepadButton = 0;
							ImGui::Text("GamePad Button");
							ImGui::SameLine();
							ImGui::Combo("##GamePad Button", &selectedGamepadButton, gamepadButtons, IM_ARRAYSIZE(gamepadButtons));
							newActionKey = gamepadButtonVKeys[selectedGamepadButton]; // 選択されたゲームパッドボタンの vKey を設定
							break;
						}
					}
					
					if (ImGui::Button("Add"))
					{
						if (strlen(newActionName) > 0)
						{
							InputSystem::RegisterActionKey(newActionName, newActionKey, static_cast<InputDevice>(newActionDevice));
							newActionName[0] = '\0'; // 入力欄をクリア
							ImGui::CloseCurrentPopup();
						}
					}
					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();


#endif // USE_IMGUI
	}
}
