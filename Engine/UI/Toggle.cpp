#include "pch.h"
#include "Toggle.h"
#include "Engine/Scenes/Scene.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(Toggle, "UI", ComponentAttributes::DisallowMultiple, {});


void Toggle::Initialize()
{
	Selectable::Initialize();
	//if (checkMark) {
	//	checkMark->SetEnabled(isOn);
	//}
}

void Toggle::Begin(RenderContext* rtx)
{
	Selectable::Begin(rtx);
	//if (checkMark) {
	//	checkMark->SetEnabled(isOn);
	//}
	if (Image* image = GetScene()->FindComponentById<Image>(checkMarkReference))
	{
		image->SetEnabled(isOn);
	}
}

void Toggle::DrawProperty()
{
#ifdef USE_IMGUI
	Selectable::DrawProperty();

	IMGUI_PROPERTY_BEGIN();
	bool changed = false;

	bool isOn = this->isOn;
	IMGUI_PROPERTY("IsOn");
	changed |= ImGui::Checkbox("##IsOn", &isOn);
	if (changed) {
		bool newVal = isOn;
		bool oldVal = this->isOn;
		std::string newValStr = newVal ? "true" : "false";
		std::string oldValStr = oldVal ? "true" : "false";
		auto setter = [this](const bool& val) { SetIsOn(val); };
		IMGUI_PROPERTY_COMMAND_CUSTOM("IsOn", newVal, oldVal, newValStr, oldValStr, setter);
	}
	IMGUI_PROPERTY_END();
#endif // USE_IMGUI
}

json Toggle::Serialize() const
{
	json j = Selectable::Serialize();
	j["isOn"] = isOn;
	return j;
}

void Toggle::Deserialize(const json& j)
{
	Selectable::Deserialize(j);
	if (j.contains("isOn")) {
		isOn = j["isOn"].get<bool>();
	}
}

void Toggle::SetIsOn(bool isOn)
{
	if (this->isOn != isOn) {
		this->isOn = isOn;
		if (Image* image = GetScene()->FindComponentById<Image>(checkMarkReference))
		{
			image->SetEnabled(isOn);
		}
		Notify();
	}
}

bool Toggle::IsOn() const { return isOn; }

void Toggle::AddCallback(std::function<void(bool)> func)
{
	callbacks.emplace_back(func);
}

void Toggle::OnPointerClick(PointerEventData* eventData)
{
	if (IsInteractable()) {
		isOn = !isOn;
		Notify();
	}
}

void Toggle::OnSubmit(BaseEventData* eventData)
{
	if (IsInteractable()) {
		isOn = !isOn;
		Notify();
	}
}

void Toggle::Notify()
{
	for (auto& callback : callbacks) {
		callback(isOn);
	}
}