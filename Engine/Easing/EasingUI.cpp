#include "pch.h"
#include "EasingUI.h"
#include "Engine/UI/RectTransform.h"
#include "Engine/Core/Time.h"
#ifdef USE_IMGUI
#include <imgui_curve.hpp>
#include <ImCurveEdit.h>
#endif // USE_IMGUI

REGISTER_COMPONENT(EasingAnchoredPosition, "Easing")

void EasingAnchoredPosition::Initialize()
{
	easeItems.resize(1);
}

void EasingAnchoredPosition::Update(float deltaTime)
{
	if (handler.GetSequenceCount() > 0)
	{
		float value = 0.0f;
		handler.Update(value, useUnscaledTime ? Time::UnscaledDeltaTime() : Time::DeltaTime());
		XMVECTOR From = XMLoadFloat2(&from);
		XMVECTOR To = XMLoadFloat2(&to);
		Vector2 result;
		XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), XMVectorLerp(From, To, value));
		GetOwner()->GetComponent<RectTransform>()->SetAnchoredPosition(result);
	}
}

#ifdef USE_IMGUI
void EasingAnchoredPosition::DrawProperty(const PropertyDrawContext& context)
{
	ImGui::Checkbox("useUnscaledTime", &useUnscaledTime);

	const char* typeNames[] = { "InQuad", "OutQuad", "InOutQuad", "InCubic", "OutCubic", "InOutCubic", "InQuart", "OutQuart", "InOutQuart", "InQuint", "OutQuint", "InOutQuint", "InSine", "OutSine",
		"InOutSine", "InExp", "OutExp", "InOutExp", "InCirc", "OutCirc", "InOutCirc", "InBounce", "OutBounce", "InOutBounce", "InBack", "OutBack", "InOutBack", "Linear", "None" };

	auto& [typeIndex, item] = easeItems.front();
	{
		item.easeData.startValue = 0.0f;
		item.easeData.endValue = 1.0f;
		ImGui::Combo("function", &typeIndex, typeNames, IM_ARRAYSIZE(typeNames));
		EasingHandler::ToEasingFunction(static_cast<EaseType>(typeIndex), item.function, item.backFunction);

		if (typeIndex != IM_ARRAYSIZE(typeNames) - 1)
		{
			if (ImGui::Button("SetCurrentValue##from")) {
				from = GetOwner()->GetComponent<RectTransform>()->GetAnchoredPosition();
			}
			ImGui::SameLine();
			ImGui::DragFloat2("From", &from.x, 0.1f);
			
			if (ImGui::Button("SetCurrentValue##to")) {
				to = GetOwner()->GetComponent<RectTransform>()->GetAnchoredPosition();
			}
			ImGui::SameLine();
			ImGui::DragFloat2("To", &to.x, 0.1f);

			if (item.backFunction)
			{
				ImGui::DragFloat("Back", &item.easeData.backValue, 0.1f);
			}
		}
		ImGui::DragFloat("Duration", &item.easeData.totalTime, 0.1f);


		static float values[128];

		if (item.function) {
			for (int j = 0; j < 128; j++) values[j] = item.function(
				j / 127.0f, 1.0f,
				item.easeData.endValue, item.easeData.startValue);
		}
		else if (item.backFunction)
		{
			for (int j = 0; j < 128; j++) values[j] = item.backFunction(
				j / 127.0f, 1.0f, item.easeData.backValue,
				item.easeData.endValue, item.easeData.startValue);
		}
		if (item.function || item.backFunction)
		{
			ImGui::PlotLines(typeNames[typeIndex], values, 128, 0, NULL, FLT_MAX, FLT_MAX, ImVec2(80, 80));
		}
	}

	if (ImGui::Button("Play"))
	{
		handler.Clear();
		handler.AddEasing(item);
	}

	
	// ImGui::Curve 用のコントロールポイント
	static int selectedPoint = -1;
	static ImVec2 points[4] = { ImVec2(0.0f, 0.0f), ImVec2(0.33f, 0.33f), ImVec2(0.66f, 0.66f), ImVec2(1.0f, 1.0f) };

	ImGui::Curve("Curve", ImVec2(200, 200), IM_ARRAYSIZE(points), points, &selectedPoint);

}
#endif // USE_IMGUI