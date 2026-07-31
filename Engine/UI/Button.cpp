#include "pch.h"
#include "Button.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Scenes/Scene.h"

REGISTER_COMPONENT(Button, "UI")

void Button::Initialize()
{
	//image = gameObject->GetComponent<Image>();
}

void Button::Update(float deltaTime)
{
	// customClickFunc が設定されている場合、呼び出して true を返した場合にクリック処理を実行
    if (customClickFunc)
    {
        if (customClickFunc())
        {
            OnClick();
        }
	}
}

void Button::UpdateInfo(EventInfo& info)
{
	if (!info.IsValid())
		return;

	Object* target = nullptr;
	if (info.objReference.IsValid())
	{
		if (info.className == "GameObject")
		{
			target = ObjectManager::Find(info.objReference);
		}
		else
		{
			target = GetScene()->FindComponentById<Component>(info.objReference);
		}
	}

	// ターゲットと関数が有効か確認
	if (target)
	{
		if (const auto* method = target->GetClassMeta()->FindMethod(info.funcName))
		{
			for (auto& [typeStr, nameStr] : method->parameters)
			{
				info.value.first = typeStr;
				if (typeStr == "int") {
					info.value.second = 0;
				}
				else if (typeStr == "float") {
					info.value.second = 0.f;
				}
				else if (typeStr == "std::string") {
					info.value.second = std::string();
				}
				else if (typeStr == "bool") {
					info.value.second = false;
				}
				else {
					if (typeStr != "void")
					{
						LOG_WARNING("Unsupported parameter type for Button event: " + typeStr);
					}
					info.value.second.reset();
				}
			}
		}
	}
}

void Button::OnClick()
{
	// クリックが有効か確認
	bool canClick = true;
	if (clickFlagFunc)
    {
		canClick = clickFlagFunc();
	}
	if (canClick)
    {
        // 登録された関数を順に呼び出す
        for (auto& func : onClickFunctions) {
            if (func) {
                func();
            }
        }
        // インスペクタで設定されたイベントを順に処理
        for (auto& info : eventInfo) {
			Object* target = nullptr;
            if (info.objReference.IsValid())
            {
				if (info.className == "GameObject")
				{
					target = ObjectManager::Find(info.objReference);
				}
				else
				{
					target = GetScene()->FindComponentById<Component>(info.objReference);
				}
            }
			if (target)
			{
                if (const auto* method = target->GetClassMeta()->FindMethod(info.funcName))
                {
					std::vector<std::any> args = {};
					if (info.value.first != "void")
					{
						args.push_back(info.value.second);
					}
					method->InvokeVoid(target, args);
                }
			}
        }
    }
}

#ifdef USE_IMGUI
void Button::DrawProperty(const PropertyDrawContext& context)
{
    Selectable::DrawProperty(context);
    ImGui::Checkbox("pressing", &isPressed);
#if 1

    ImGui::NewLine();

    // イベント
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::Selectable("OnClick()", true);
        ImGui::PopStyleColor(3);
        //ImVec2 size = ImVec2(250.f, droppedObjects.empty() ? 30.f : 30.f * droppedObjects.size());
        if (ImGui::BeginChild("OnClickEvent", ImVec2(250, 150), true)) {

            for (size_t i = 0; i < eventInfo.size(); i++) {
				auto& info = eventInfo[i];
                ImGui::PushID(static_cast<int>(i));
                // 要素の間に区切り線をつける
                if (i > 0) {
                    ImGui::Separator();
                }
                // ドロップ先
				std::string label;
				Object* target = nullptr;
				GameObject* targetObj = nullptr;
				if (info.objReference.IsValid())
				{
					if (info.className == "GameObject")
                    {
						target = targetObj = ObjectManager::Find(info.objReference);
                    }
					else
                    {
						if (auto comp = GetScene()->FindComponentById<Component>(info.objReference))
						{
							targetObj = comp->GetOwner();
							target = comp;
						}
					}
				}
                if (target)
                {
					label = targetObj->GetName() + " (" + target->GetTypeName() + ")";
                }
                else
                {
                    label = "None";
				}

				ImGui::Button(label.c_str());
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
						if (payload->DataSize == sizeof(ObjectId)) // ペイロードのサイズが ObjectId と同じであることを確認
						{
							info.objReference = *reinterpret_cast<const ObjectId*>(payload->Data); // ペイロードから ObjectId を取得
							info.className = "GameObject"; // クラス名を GameObject に設定
							info.funcName.clear();	// クラスが変わったので関数名をリセット
							info.value.second.reset(); // 引数もリセット
						}
                    }
                    ImGui::EndDragDropTarget();
                }

                //ImGui::SameLine();

				auto createFunctionList = [this](Object* target) -> std::vector<std::string>
				{
					std::vector<std::string> list;
					if (target)
					{
						for (auto& method : target->GetClassMeta()->methods)
						{
							list.push_back(method.name);
						}
					}
					return list;
				};



				// クラスの関数リスト
				std::vector<Object*> classes;
				if (targetObj)
				{
					classes.push_back(targetObj);

					for (const auto& component : targetObj->GetAllComponents())
					{
						if (component)
						{
							classes.push_back(component.get());
						}
					}
				}

				// 関数のコンボボックス
				if (ImGui::BeginCombo("##func", info.funcName.empty() ? "No Function" : info.funcName.c_str()))
				{
					if (ImGui::Selectable("No Function", info.funcName.empty()))
					{
						info.funcName.clear();
						info.value.first.clear();
						info.value.second.reset();

						UpdateInfo(info);

						ImGui::SetItemDefaultFocus();
					}

					ImGui::Separator();

					// ゲームオブジェクトとコンポーネントのクラス選択と関数リストの更新
					{
						for (int j = 0; j < classes.size(); j++)
						{
							auto* pClass = classes[j];
							if (!pClass) continue;
							if (ImGui::BeginMenu(pClass->GetTypeName().c_str()))
							{
								// 関数リスト
								std::vector<std::string> itemStrs = createFunctionList(pClass);

								for (int k = 0; k < itemStrs.size(); k++)
								{
									auto& funcName = itemStrs[k];
									bool isSelected = (info.className == pClass->GetTypeName()) && (info.funcName == funcName);
									//bool isSelected = false;
									bool enabled = true;
									if (ImGui::MenuItem(funcName.c_str(), NULL, isSelected, enabled))
									{
										info.objReference = pClass->GetId();
										info.className = pClass->GetTypeName();
										info.funcName = funcName;
										UpdateInfo(info);
									}

									if (isSelected) {
										ImGui::SetItemDefaultFocus();
									}
								}
								ImGui::EndMenu();
							}
						}
					}
                    ImGui::EndCombo();
                }
				// 引数の表示 (std::anyから型情報を元に適切なUIを表示する)
				if (info.IsValid())
				{
					{
						auto& [typeStr, valueAny] = info.value;
						if (typeStr == "int")
						{
							int val = valueAny.has_value() ? std::any_cast<int>(valueAny) : 0;
							if (ImGui::InputInt("##arg", &val))
							{
								valueAny = val;
							}
						}
						else if (typeStr == "float")
						{
							float val = valueAny.has_value() ? std::any_cast<float>(valueAny) : 0.f;
							if (ImGui::InputFloat("##arg", &val))
							{
								valueAny = val;
							}
						}
						else if (typeStr == "std::string" || typeStr == "string")
						{
							std::string val = valueAny.has_value() ? std::any_cast<std::string>(valueAny) : "";
							char buffer[256];
							strncpy_s(buffer, val.c_str(), sizeof(buffer));
							if (ImGui::InputText("##arg", buffer, sizeof(buffer)))
							{
								valueAny = std::string(buffer);
							}
						}
						else if (typeStr == "bool")
						{
							bool val = valueAny.has_value() ? std::any_cast<bool>(valueAny) : false;
							if (ImGui::Checkbox("##arg", &val))
							{
								valueAny = val;
							}
						}
                        else
                        {
                            ImGui::Text("Unsupported type");
						}
					}
				}

                ImGui::PopID();
            }

            if (eventInfo.empty()) {
                ImGui::Text("List is Empty");
            }
        }
        ImGui::EndChild();


        if (ImGui::Button("+", ImVec2(30, 30)) && eventInfo.size() < eventInfo.max_size()) {
            eventInfo.resize(eventInfo.size() + 1);
        }
        ImGui::SameLine();
        if (ImGui::Button("-", ImVec2(30, 30)) && eventInfo.size() > 0) {
            eventInfo.resize(eventInfo.size() - 1);
        }
    }
#endif // 0


}
#endif // USE_IMGUI

json Button::Serialize() const
{
	json j = Selectable::Serialize();

	json eventInfoJson = json::array();
	for (const auto& info : eventInfo)
	{
		json infoJson;
		infoJson["objReference"] = info.objReference.Value();
		infoJson["className"] = info.className;
		infoJson["funcName"] = info.funcName;
		if (info.value.second.has_value())
		{
			const auto& [typeStr, valueAny] = info.value;
			infoJson["value"] = { {"type", typeStr} };
			if (typeStr == "int") {
				infoJson["value"]["data"] = std::any_cast<int>(valueAny);
			}
			else if (typeStr == "float") {
				infoJson["value"]["data"] = std::any_cast<float>(valueAny);
			}
			else if (typeStr == "std::string" || typeStr == "string") {
				infoJson["value"]["data"] = std::any_cast<std::string>(valueAny);
			}
			else if (typeStr == "bool") {
				infoJson["value"]["data"] = std::any_cast<bool>(valueAny);
			}
			else {
				infoJson["value"]["data"] = nullptr; // サポートされていない型は null として保存
			}
		}
		else
		{
			infoJson["value"] = nullptr; // 値がない場合は null として保存
		}
		eventInfoJson.push_back(infoJson);
	}
	j["eventInfo"] = eventInfoJson;
	return j;
}

void Button::Deserialize(const json& j)
{
	Selectable::Deserialize(j);
	if (j.contains("eventInfo"))
	{
		json eventInfoJson = j["eventInfo"];
		eventInfo.clear();

		for (const auto& infoJson : eventInfoJson)
		{
			EventInfo info;
			info.objReference = ObjectId::FromValue(infoJson["objReference"].get<uint64_t>());
			info.className = infoJson["className"].get<std::string>();
			info.funcName = infoJson["funcName"].get<std::string>();
			if (infoJson.contains("value") && !infoJson["value"].is_null())
			{
				std::string typeStr = infoJson["value"]["type"].get<std::string>();
				if (typeStr == "int") {
					info.value = { typeStr, infoJson["value"]["data"].get<int>() };
				}
				else if (typeStr == "float") {
					info.value = { typeStr, infoJson["value"]["data"].get<float>() };
				}
				else if (typeStr == "std::string" || typeStr == "string") {
					info.value = { typeStr, infoJson["value"]["data"].get<std::string>() };
				}
				else if (typeStr == "bool") {
					info.value = { typeStr, infoJson["value"]["data"].get<bool>() };
				}
				else {
					info.value = { typeStr, std::any() }; // サポートされていない型は空の std::any として保存
				}
			}
			else
			{
				info.value = { "", std::any() }; // 値がない場合は空の std::any として保存
			}
			eventInfo.push_back(info);
		}
	}
}