#include "pch.h"
#include "EasingTransform.h"
REGISTER_COMPONENT(EasingScale, "Easing")

void EasingPosition::DrawProperty()
{
#ifdef USE_IMGUI

	ImGui::Checkbox("useUnscaledTime", &useUnscaledTime);

	const char* typeNames[] = { "InQuad", "OutQuad", "InOutQuad", "InCubic", "OutCubic", "InOutCubic", "InQuart", "OutQuart", "InOutQuart", "InQuint", "OutQuint", "InOutQuint", "InSine", "OutSine",
		"InOutSine", "InExp", "OutExp", "InOutExp", "InCirc", "OutCirc", "InOutCirc", "InBounce", "OutBounce", "InOutBounce", "InBack", "OutBack", "InOutBack", "Linear", "None" };

	for (int i = 0; i < easeItems.size(); i += 3)
	{
		ImGui::PushID(i);
		std::string label;
		if (i % 3 == 0) label = "X";
		else if (i % 3 == 1) label = "Y";
		else label = "Z";

		if (ImGui::TreeNodeEx(("ease" + std::to_string(i) + label).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Combo("function", &easeItems[i].first, typeNames, IM_ARRAYSIZE(typeNames));
			EasingHandler::ToEasingFunction(static_cast<EaseType>(easeItems[i].first), easeItems[i].second.function, easeItems[i].second.backFunction);

			if (easeItems[i].first != IM_ARRAYSIZE(typeNames) - 1)
			{
				ImGui::DragFloat3("start", &easeItems[i].second.easeData.startValue, 0.1f);
				ImGui::DragFloat3("end", &easeItems[i].second.easeData.endValue, 0.1f);
				if (easeItems[i].second.backFunction)
				{
					ImGui::DragFloat("back", &easeItems[i].second.easeData.backValue, 0.1f);
				}
			}
			ImGui::DragFloat("time", &easeItems[i].second.easeData.totalTime, 0.1f);


			//ImGui::SliderFloat("Progress", )


			static float values[128];

			if (easeItems[i].second.function) {
				for (int j = 0; j < 128; j++) values[j] = easeItems[i].second.function(
					i / 127.0f, 1.0f,
					easeItems[i].second.easeData.endValue, easeItems[i].second.easeData.startValue);
			}
			else if (easeItems[i].second.backFunction)
			{
				for (int j = 0; j < 128; j++) values[j] = easeItems[i].second.backFunction(
					i / 127.0f, 1.0f, easeItems[i].second.easeData.backValue,
					easeItems[i].second.easeData.endValue, easeItems[i].second.easeData.startValue);
			}
			if (easeItems[i].second.function || easeItems[i].second.backFunction)
			{
				ImGui::PlotLines("Curve", values, 128, 0, NULL, 0.0f, 1.0f, ImVec2(0, 80));
			}


			ImGui::TreePop();
		}

		ImGui::PopID();
	}
	float size = 20.0f;

	if (ImGui::Button("+", ImVec2(size, size)) && easeItems.size() < easeItems.max_size())
	{
		easeItems.resize(easeItems.size() + 3);
	}
	ImGui::SameLine();
	if (ImGui::Button("-", ImVec2(size, size)) && easeItems.size() > 0)
	{
		easeItems.resize((easeItems.size() - 3 >= 0) ? (easeItems.size() - 3) : 0);
	}

	if (ImGui::Button("Start") && easeItems.size() > 0)
	{
		EasingHandler handler;
		for (auto& easeItem : easeItems)
		{
			handler.AddEasing(easeItem.second);
		}
		//Propertyアクセス設定
		PropertyAccessor<float> accessor;
		accessor.getter = [&]()-> float {
			return GetOwner()->transform->position.x;
			};
		accessor.setter = [&](float value) {
			GetOwner()->transform->position.x = value;
			GetOwner()->transform->MarkNeedsUpdate();
			};

		StartHandler(handler, accessor);
	}
#endif // USE_IMGUI
}

void EasingRotation::DrawProperty()
{

}

void EasingScale::DrawProperty()
{
#ifdef USE_IMGUI

	ImGui::Checkbox("useUnscaledTime", &useUnscaledTime);

	const char* typeNames[] = { "InQuad", "OutQuad", "InOutQuad", "InCubic", "OutCubic", "InOutCubic", "InQuart", "OutQuart", "InOutQuart", "InQuint", "OutQuint", "InOutQuint", "InSine", "OutSine",
		"InOutSine", "InExp", "OutExp", "InOutExp", "InCirc", "OutCirc", "InOutCirc", "InBounce", "OutBounce", "InOutBounce", "InBack", "OutBack", "InOutBack", "Linear", "None" };

	for (int i = 0; i < easeItems.size(); i += 3)
	{
		ImGui::PushID(i);
		if (ImGui::TreeNodeEx(("ease" + std::to_string(i)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Combo("function", &easeItems[i].first, typeNames, IM_ARRAYSIZE(typeNames));
			EasingHandler::ToEasingFunction(static_cast<EaseType>(easeItems[i].first), easeItems[i].second.function, easeItems[i].second.backFunction);

			if (easeItems[i].first != IM_ARRAYSIZE(typeNames) - 1)
			{
				ImGui::DragFloat("start", &easeItems[i].second.easeData.startValue, 0.1f);
				ImGui::DragFloat("end", &easeItems[i].second.easeData.endValue, 0.1f);
				if (easeItems[i].second.backFunction)
				{
					ImGui::DragFloat("back", &easeItems[i].second.easeData.backValue, 0.1f);
				}
			}
			ImGui::DragFloat("time", &easeItems[i].second.easeData.totalTime, 0.1f);


			//ImGui::SliderFloat("Progress", )


			static float values[128];

			if (easeItems[i].second.function) {
				for (int j = 0; j < 128; j++) values[j] = easeItems[i].second.function(
					j / 127.0f, 1.0f,
					easeItems[i].second.easeData.endValue, easeItems[i].second.easeData.startValue);
			}
			else if (easeItems[i].second.backFunction)
			{
				for (int j = 0; j < 128; j++) values[j] = easeItems[i].second.backFunction(
					j / 127.0f, 1.0f, easeItems[i].second.easeData.backValue,
					easeItems[i].second.easeData.endValue, easeItems[i].second.easeData.startValue);
			}
			if (easeItems[i].second.function || easeItems[i].second.backFunction)
			{
				float minValue = min(easeItems[i].second.easeData.startValue, easeItems[i].second.easeData.endValue);
				float maxValue = max(easeItems[i].second.easeData.startValue, easeItems[i].second.easeData.endValue);
				ImGui::PlotLines("Curve", values, 128, 0, NULL, minValue, maxValue, ImVec2(0, 80));
			}


			ImGui::TreePop();
		}

		ImGui::PopID();
	}
	float size = 20.0f;

	if (ImGui::Button("+", ImVec2(size, size)) && easeItems.size() < easeItems.max_size())
	{
		easeItems.resize(easeItems.size() + 1);
	}
	ImGui::SameLine();
	if (ImGui::Button("-", ImVec2(size, size)) && easeItems.size() > 0)
	{
		easeItems.resize(easeItems.size() - 1);
	}

	if (ImGui::Button("Start") && easeItems.size() > 0)
	{
		EasingHandler handler;
		for (auto& easeItem : easeItems)
		{
			handler.AddEasing(easeItem.second);
		}
		//Propertyアクセス設定
		PropertyAccessor<float> accessor;
		accessor.getter = [&]()-> float {
			return GetOwner()->transform->scale.x;
			};
		accessor.setter = [&](float value) {
			GetOwner()->transform->SetScale(value);
			};

		StartHandler(handler, accessor);
	}
#endif // USE_IMGUI
}