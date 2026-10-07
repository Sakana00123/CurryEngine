#include "pch.h"
#include "InputSettingsWindow.h"
#include "Engine/Input/InputSystem.h"

namespace CurryEngine::Editor
{
#ifdef USE_IMGUI
	namespace
	{
		/**
		 * @brief バーチャルキーコードからキー名を取得します。
		 * @param key バーチャルキーコード
		 * @return キー名
		 */
		std::string GetKeyboardKeyName(int key)
		{
			// A-Z, 0-9 の場合はそのまま返す
			if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
				return std::string(1, static_cast<char>(key));

			// Windows API で適切なキー名が取得できない場合の例外処理
			switch (key)
			{
			case VK_LSHIFT: return "Left Shift";
			case VK_RSHIFT: return "Right Shift";
			case VK_SHIFT: return "Shift";
			case VK_LCONTROL: return "Left Control";
			case VK_RCONTROL: return "Right Control";
			case VK_CONTROL: return "Control";
			case VK_LMENU: return "Left Alt";
			case VK_RMENU: return "Right Alt";
			case VK_MENU: return "Alt";
			case VK_SPACE: return "Space";
			case VK_DELETE: return "Delete";
			case VK_INSERT: return "Insert";
			case VK_HOME: return "Home";
			case VK_END: return "End";
			case VK_UP: return "Up Arrow";
			case VK_DOWN: return "Down Arrow";
			case VK_LEFT: return "Left Arrow";
			case VK_RIGHT: return "Right Arrow";
			}

			// Windows API を使ってキー名を取得する
			const UINT scanCode = MapVirtualKeyW(static_cast<UINT>(key), MAPVK_VK_TO_VSC_EX);
			LONG keyData = static_cast<LONG>((scanCode & 0xff) << 16);
			if ((scanCode & 0xff00) == 0xe000) keyData |= 1 << 24;
			wchar_t name[128]{};
			if (scanCode != 0 && GetKeyNameTextW(keyData, name, IM_ARRAYSIZE(name)) > 0)
			{
				char utf8[512]{};
				if (WideCharToMultiByte(CP_UTF8, 0, name, -1, utf8, sizeof(utf8), nullptr, nullptr) > 0)
					return utf8;
			}
			return "Key " + std::to_string(key);
		}

		/**
		 * @brief 入力キーからラベルを取得します。
		 * @param key 入力キー
		 * @return ラベル
		 */
		std::string GetBindingLabel(const InputKey& key)
		{
			switch (key.GetDeviceType())
			{
			case InputDevice::Keybord:
				return GetKeyboardKeyName(key.GetVKey()) + " [Keyboard]";
			case InputDevice::Mouse:
				switch (key.GetVKey())
				{
				case VK_LBUTTON: return "Left Button [Mouse]";
				case VK_RBUTTON: return "Right Button [Mouse]";
				case VK_MBUTTON: return "Middle Button [Mouse]";
				case VK_XBUTTON1: return "X1 Button [Mouse]";
				case VK_XBUTTON2: return "X2 Button [Mouse]";
				default: return "Button " + std::to_string(key.GetVKey()) + " [Mouse]";
				}
			case InputDevice::GamePad:
				if (const auto* gamepad = dynamic_cast<const GamePad*>(&key))
				{
					if (gamepad->GetKeyType() == KeyType::LeftTrigger) return "Left Trigger [Gamepad]";
					if (gamepad->GetKeyType() == KeyType::RightTrigger) return "Right Trigger [Gamepad]";
				}
				switch (key.GetVKey())
				{
				case XINPUT_GAMEPAD_A: return "Button A [Gamepad]";
				case XINPUT_GAMEPAD_B: return "Button B [Gamepad]";
				case XINPUT_GAMEPAD_X: return "Button X [Gamepad]";
				case XINPUT_GAMEPAD_Y: return "Button Y [Gamepad]";
				case XINPUT_GAMEPAD_LEFT_SHOULDER: return "Left Shoulder [Gamepad]";
				case XINPUT_GAMEPAD_RIGHT_SHOULDER: return "Right Shoulder [Gamepad]";
				case XINPUT_GAMEPAD_LEFT_THUMB: return "Left Stick Press [Gamepad]";
				case XINPUT_GAMEPAD_RIGHT_THUMB: return "Right Stick Press [Gamepad]";
				case XINPUT_GAMEPAD_BACK: return "Back [Gamepad]";
				case XINPUT_GAMEPAD_START: return "Start [Gamepad]";
				case XINPUT_GAMEPAD_DPAD_UP: return "D-pad Up [Gamepad]";
				case XINPUT_GAMEPAD_DPAD_DOWN: return "D-pad Down [Gamepad]";
				case XINPUT_GAMEPAD_DPAD_LEFT: return "D-pad Left [Gamepad]";
				case XINPUT_GAMEPAD_DPAD_RIGHT: return "D-pad Right [Gamepad]";
				default: return "Button " + std::to_string(key.GetVKey()) + " [Gamepad]";
				}
			default: return "Unknown input";
			}
		}

		/**
		 * @brief 行マーカーを描画します。
		 * @param position 描画位置
		 * @param height 高さ
		 * @param color 色
		 */
		void DrawRowMarker(const ImVec2& position, float height, ImU32 color)
		{
			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2(position.x, position.y + 1.0f),
				ImVec2(position.x + 4.0f, position.y + height - 1.0f), color);
		}

		/**
		 * @brief 入力バインディングの編集状態を表す構造体。
		 */
		struct BindingEditor
		{
			int device = static_cast<int>(InputDevice::Keybord);
			int key = VK_SPACE;
			int type = static_cast<int>(KeyType::Key);
			bool capture = false;

			void Load(const InputKey& input)
			{
				device = static_cast<int>(input.GetDeviceType());
				key = input.GetVKey();
				const auto* gamepad = dynamic_cast<const GamePad*>(&input);
				type = gamepad ? static_cast<int>(gamepad->GetKeyType()) : 0;
				capture = false;
			}

			std::unique_ptr<InputKey> CreateKey() const
			{
				switch (static_cast<InputDevice>(device))
				{
				case InputDevice::Keybord: return std::make_unique<Keybord>(key);
				case InputDevice::Mouse: return std::make_unique<Mouse>(key);
				case InputDevice::GamePad: return std::make_unique<GamePad>(key, static_cast<KeyType>(type));
				default: return nullptr;
				}
			}

			bool Matches(const InputKey& input) const
			{
				if (input.GetDeviceType() != static_cast<InputDevice>(device) || input.GetVKey() != key)
					return false;
				const auto* gamepad = dynamic_cast<const GamePad*>(&input);
				return !gamepad || static_cast<int>(gamepad->GetKeyType()) == type;
			}
		};

		/**
		 * @brief 入力バインディングの編集UIを描画します。
		 * @param editor 入力バインディングの編集状態
		 */
		void DrawBindingEditor(BindingEditor& editor)
		{
			static const char* devices[] = { "Keyboard", "Mouse", "Gamepad" };
			ImGui::PushItemWidth(-1.0f);
			ImGui::TextUnformatted("Device");
			if (ImGui::Combo("##Device", &editor.device, devices, IM_ARRAYSIZE(devices)))
			{
				editor.key = editor.device == 0 ? VK_SPACE : editor.device == 1 ? VK_LBUTTON : XINPUT_GAMEPAD_A;
				editor.type = 0;
				editor.capture = false;
			}
			if (editor.device == static_cast<int>(InputDevice::Keybord))
			{
				ImGui::TextUnformatted("Key: ");
				ImGui::SameLine();
				ImGui::TextWrapped("%s", GetKeyboardKeyName(editor.key).c_str());
				if (editor.capture && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
					&& InputSystem::inputKeyDown != 0)
				{
					editor.key = static_cast<int>(InputSystem::inputKeyDown);
					editor.capture = false;
				}
				if (ImGui::Button(editor.capture ? "Cancel capture" : "Capture Key"))
					editor.capture = !editor.capture;
				if (editor.capture) ImGui::TextWrapped("Press a keyboard key...");
			}
			else
			{
				static const char* mouseNames[] = { "Left", "Right", "Middle", "X1", "X2" };
				static const int mouseKeys[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
				static const char* gamepadNames[] = { "A", "B", "X", "Y", "LB", "RB", "Back", "Start",
					"Left Stick Press", "Right Stick Press", "D-pad Up", "D-pad Down", "D-pad Left", "D-pad Right" };
				static const int gamepadKeys[] = { XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
					XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER, XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_START,
					XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB, XINPUT_GAMEPAD_DPAD_UP,
					XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT };
				const bool mouse = editor.device == static_cast<int>(InputDevice::Mouse);
				if (!mouse)
				{
					static const char* types[] = { "Button", "Left Trigger", "Right Trigger" };
					ImGui::TextUnformatted("Input Type");
					if (ImGui::Combo("##Type", &editor.type, types, IM_ARRAYSIZE(types)))
						editor.key = editor.type == 0 ? XINPUT_GAMEPAD_A : 0;
				}
				if (mouse || editor.type == 0)
				{
					const char* const* names = mouse ? mouseNames : gamepadNames;
					const int* keys = mouse ? mouseKeys : gamepadKeys;
					const int count = mouse ? IM_ARRAYSIZE(mouseKeys) : IM_ARRAYSIZE(gamepadKeys);
					std::string preview = "Button " + std::to_string(editor.key);
					for (int i = 0; i < count; ++i)
						if (keys[i] == editor.key) preview = names[i];
					ImGui::TextUnformatted("Button");
					if (ImGui::BeginCombo("##Button", preview.c_str()))
					{
						for (int i = 0; i < count; ++i)
						{
							const bool selected = editor.key == keys[i];
							if (ImGui::Selectable(names[i], selected)) editor.key = keys[i];
							if (selected) ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
				}
			}
			ImGui::PopItemWidth();
		}

		/**
		 * @brief 入力バインディングの重複チェックを行います。
		 * @param editor 入力バインディングの編集状態
		 * @param action アクション名
		 * @param ignoredIndex 無視するバインディングのインデックス（-1 で無視しない）
		 * @return エラーメッセージ（重複がなければ nullptr）
		 */
		const char* GetBindingError(const BindingEditor& editor, const std::string& action, int ignoredIndex = -1)
		{
			if (editor.device == static_cast<int>(InputDevice::Keybord) && (editor.key <= 0 || editor.key > 254))
				return "Enter a virtual key code between 1 and 254.";
			const auto& actions = InputSystem::GetInputKeys();
			const auto it = actions.find(action);
			if (it != actions.end())
			{
				for (size_t i = 0; i < it->second.size(); ++i)
					if (static_cast<int>(i) != ignoredIndex && editor.Matches(*it->second[i]))
						return "This binding is already assigned to the action.";
			}
			return nullptr;
		}

		/**
		 * @brief アクション名が空かどうかを判定します。
		 * @param name アクション名
		 * @return 空なら true
		 */
		bool IsActionNameEmpty(const std::string& name)
		{
			return name.find_first_not_of(" \t\r\n") == std::string::npos;
		}
	}
#endif

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
		ImGui::SetNextWindowSize(ImVec2(760.0f, 520.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Input Settings", &isOpen))
		{
			static std::string selectedAction;
			static int selectedBinding = -1;
			static std::string bindingAction;
			static std::string expandAction;
			static char newActionName[128] = "";
			static BindingEditor newBinding;
			static BindingEditor editedBinding;
			static std::string editingAction;
			static int editingBinding = -1;
			static std::vector<char> editedName(256, '\0');
			auto& actions = InputSystem::GetInputKeys();
			bool openAddPopup = false;
			bool changed = false;
			bool deleteRequested = false;
			std::string deleteAction;
			int deleteBinding = -1;

			if (ImGui::BeginTable("InputSettingsLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
			{
				ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthStretch, 1.0f);
				ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthStretch, 1.0f);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::BeginChild("ActionsPane", ImVec2(0.0f, 0.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(3.0f, 0.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.0f, 2.0f));
				ImGui::PushStyleColor(ImGuiCol_TableRowBg, IM_COL32(54, 54, 54, 255));
				ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, IM_COL32(57, 57, 57, 255));
				ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(44, 100, 134, 255));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				const float rowHeight = ImGui::GetFrameHeight();
				const float indent = ImGui::GetFontSize();
				if (ImGui::BeginTable("ActionList", 2,
					ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter
					| ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp,
					ImGui::GetContentRegionAvail()))
				{
					ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("Add", ImGuiTableColumnFlags_WidthFixed, rowHeight);
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers, rowHeight);
					ImGui::TableSetColumnIndex(0);
					ImGui::AlignTextToFramePadding();
					ImGui::TextUnformatted("Actions");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("+##AddAction", ImVec2(rowHeight, rowHeight)))
					{
						bindingAction.clear();
						newActionName[0] = '\0';
						openAddPopup = true;
					}
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Add action");

					for (const auto& [action, keys] : InputSystem::GetInputKeys())
					{
						ImGui::PushID(action.c_str());
						ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
						ImGui::TableSetColumnIndex(0);
						const ImVec2 actionPosition = ImGui::GetCursorScreenPos();
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6.0f);
						if (expandAction == action)
						{
							ImGui::SetNextItemOpen(true);
							expandAction.clear();
						}
						ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow
							| ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth
							| ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_FramePadding;
						if (selectedAction == action && selectedBinding == -1) flags |= ImGuiTreeNodeFlags_Selected;
						const bool expanded = ImGui::TreeNodeEx("##Action", flags, "%s", action.c_str());
						if (ImGui::IsItemClicked())
						{
							selectedAction = action;
							selectedBinding = -1;
						}
						if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
						{
							selectedAction = action;
							selectedBinding = -1;
						}
						if (ImGui::BeginPopupContextItem("ActionMenu"))
						{
							if (ImGui::MenuItem("Delete Action"))
							{
								deleteRequested = true;
								deleteAction = action;
							}
							ImGui::EndPopup();
						}
						DrawRowMarker(actionPosition, rowHeight, IM_COL32(151, 190, 132, 255));
						ImGui::TableSetColumnIndex(1);
						if (ImGui::Button("+##AddBinding", ImVec2(rowHeight, rowHeight)))
						{
							bindingAction = action;
							expandAction = action;
							openAddPopup = true;
						}
						if (ImGui::IsItemHovered()) ImGui::SetTooltip("Add binding to %s", action.c_str());
						if (expanded)
						{
							for (size_t index = 0; index < keys.size(); ++index)
							{
								ImGui::PushID(static_cast<int>(index));
								ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
								ImGui::TableSetColumnIndex(0);
								ImVec2 bindingPosition = ImGui::GetCursorScreenPos();
								bindingPosition.x += indent * 0.5f;
								ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent + 6.0f);
								const ImVec2 labelPosition = ImGui::GetCursorScreenPos();
								const std::string label = GetBindingLabel(*keys[index]);
								if (ImGui::Selectable("##Binding", selectedAction == action && selectedBinding == static_cast<int>(index),
									ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.0f, rowHeight)))
								{
									selectedAction = action;
									selectedBinding = static_cast<int>(index);
								}
								if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
								{
									selectedAction = action;
									selectedBinding = static_cast<int>(index);
								}
								const bool bindingHovered = ImGui::IsItemHovered();
								if (ImGui::BeginPopupContextItem("BindingMenu"))
								{
									if (ImGui::MenuItem("Delete Binding"))
									{
										deleteRequested = true;
										deleteAction = action;
										deleteBinding = static_cast<int>(index);
									}
									ImGui::EndPopup();
								}
								ImGui::GetWindowDrawList()->AddText(ImVec2(labelPosition.x, labelPosition.y + 2.0f),
									ImGui::GetColorU32(ImGuiCol_Text), label.c_str());
								if (bindingHovered) ImGui::SetTooltip("%s", label.c_str());
								DrawRowMarker(bindingPosition, rowHeight, IM_COL32(130, 188, 194, 255));
								ImGui::PopID();
							}
						}
						ImGui::PopID();
					}
					ImGui::EndTable();
				}
				ImGui::PopStyleColor(4);
				ImGui::PopStyleVar(3);
				ImGui::EndChild();

				// Apply removals after traversal so no map/vector iterators are invalidated while drawing.
				if (deleteRequested)
				{
					auto it = actions.find(deleteAction);
					if (it != actions.end())
					{
						if (deleteBinding < 0)
						{
							actions.erase(it);
							if (selectedAction == deleteAction) selectedAction.clear();
							changed = true;
						}
						else if (deleteBinding < static_cast<int>(it->second.size()))
						{
							it->second.erase(it->second.begin() + deleteBinding);
							if (selectedAction == deleteAction)
							{
								if (selectedBinding == deleteBinding) selectedBinding = -1;
								else if (selectedBinding > deleteBinding) --selectedBinding;
							}
							changed = true;
						}
					}
				}

				ImGui::TableSetColumnIndex(1);
				ImGui::BeginChild("PropertiesPane", ImVec2(0.0f, 0.0f));
				ImGui::TextUnformatted("Properties");
				ImGui::Separator();
				auto selected = actions.find(selectedAction);
				if (selected == actions.end())
				{
					selectedAction.clear();
					selectedBinding = -1;
					editingAction.clear();
					editingBinding = -1;
					editedBinding.capture = false;
					ImGui::TextWrapped("Select an action or a binding to edit its properties.");
				}
				else
				{
					if (selectedBinding >= static_cast<int>(selected->second.size())) selectedBinding = -1;
					if (editingAction != selectedAction || editingBinding != selectedBinding)
					{
						editingAction = selectedAction;
						editingBinding = selectedBinding;
						editedName.assign((std::max)(size_t(256), selectedAction.size() + 1), '\0');
						std::copy(selectedAction.begin(), selectedAction.end(), editedName.begin());
						editedBinding.capture = false;
						if (selectedBinding >= 0) editedBinding.Load(*selected->second[selectedBinding]);
					}
					if (selectedBinding < 0)
					{
						if (ImGui::CollapsingHeader("Action", ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::TextUnformatted("Name");
							ImGui::SetNextItemWidth(-1.0f);
							ImGui::InputText("##ActionName", editedName.data(), editedName.size());
							const std::string name = editedName.data();
							const bool invalidName = IsActionNameEmpty(name) || (name != selectedAction && actions.contains(name));
							if (invalidName) ImGui::TextWrapped("Enter a non-empty, unique action name.");
							ImGui::BeginDisabled(invalidName || name == selectedAction);
							if (ImGui::Button("Apply"))
							{
								auto node = actions.extract(selected);
								node.key() = name;
								actions.insert(std::move(node));
								selectedAction = name;
								expandAction = name;
								changed = true;
							}
							ImGui::EndDisabled();
							ImGui::SameLine();
							if (ImGui::Button("Revert")) editingAction.clear();
						}
					}
					else if (ImGui::CollapsingHeader("Binding", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::TextWrapped("Action: %s", selectedAction.c_str());
						DrawBindingEditor(editedBinding);
						const char* error = GetBindingError(editedBinding, selectedAction, selectedBinding);
						if (error) ImGui::TextWrapped("%s", error);
						ImGui::BeginDisabled(error != nullptr || editedBinding.Matches(*selected->second[selectedBinding]));
						if (ImGui::Button("Apply"))
						{
							selected->second[selectedBinding] = editedBinding.CreateKey();
							editedBinding.capture = false;
							changed = true;
						}
						ImGui::EndDisabled();
						ImGui::SameLine();
						if (ImGui::Button("Revert")) editedBinding.Load(*selected->second[selectedBinding]);
					}
				}
				ImGui::EndChild();
				ImGui::EndTable();
			}
			if (openAddPopup)
			{
				newBinding = BindingEditor{};
				ImGui::OpenPopup("AddActionPopup");
			}
			if (ImGui::IsPopupOpen("AddActionPopup"))
			{
				if (ImGui::BeginPopup("AddActionPopup"))
				{
					if (bindingAction.empty())
						ImGui::InputText("Action Name", newActionName, IM_ARRAYSIZE(newActionName));
					else
						ImGui::Text("Action: %s", bindingAction.c_str());
					DrawBindingEditor(newBinding);
					const std::string actionName = bindingAction.empty() ? newActionName : bindingAction;
					const bool invalidName = IsActionNameEmpty(actionName)
						|| (bindingAction.empty() && actions.contains(actionName));
					const char* error = GetBindingError(newBinding, actionName);
					if (invalidName) ImGui::TextWrapped("Enter a non-empty, unique action name.");
					if (error) ImGui::TextWrapped("%s", error);
					ImGui::BeginDisabled(invalidName || error != nullptr);
					if (ImGui::Button("Add"))
					{
						InputSystem::RegisterActionKey(actionName, newBinding.key,
							static_cast<InputDevice>(newBinding.device), static_cast<KeyType>(newBinding.type));
						selectedAction = actionName;
						selectedBinding = static_cast<int>(actions.at(actionName).size()) - 1;
						expandAction = actionName;
						changed = true;
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndDisabled();
					ImGui::SameLine();
					if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
				}
			}
			if (changed) InputSystem::SaveInputSettings();
		}
		ImGui::End();


#endif // USE_IMGUI
	}
}
