#include "pch.h"
#include "EasingComponent.h"
#include "Engine/Core/Time.h"
REGISTER_COMPONENT(EasingComponent, "Easing")

void EasingComponent::Update(float deltaTime)
{
	for (auto& [accessor, handler] : handlers)
	{
		if (!handler.IsCompleted())
		{
			float value = accessor.getter ? accessor.getter() : 0.0f;
			handler.Update(value, useUnscaledTime ? Time::UnscaledDeltaTime() : Time::DeltaTime());
			if (accessor.setter) {
				accessor.setter(value);
			}
		}
	}
	if (!handlers.empty())
	{
		handlers.erase(std::remove_if(handlers.begin(), handlers.end(),
			[&](const auto& handler) {
				return handler.second.IsCompleted();
			}),
			handlers.end());
	}
}

void EasingComponent::DrawProperty()
{
#ifdef USE_IMGUI

	ImGui::Checkbox("useUnscaledTime", &useUnscaledTime);

	static int valueType = 0;
	const char* valueTypes[] = { "positionX", "positionY","positionZ", "scaleX","scaleY","scaleZ", "rotationX", "rotationY", "rotationZ" };

	const char* typeNames[] = { "InQuad", "OutQuad", "InOutQuad", "InCubic", "OutCubic", "InOutCubic", "InQuart", "OutQuart", "InOutQuart", "InQuint", "OutQuint", "InOutQuint", "InSine", "OutSine",
		"InOutSine", "InExp", "OutExp", "InOutExp", "InCirc", "OutCirc", "InOutCirc", "InBounce", "OutBounce", "InOutBounce", "InBack", "OutBack", "InOutBack", "Linear", "None" };

	ImGui::Combo("valueType", &valueType, valueTypes, IM_ARRAYSIZE(valueTypes));

	for (int i = 0; i < easeItems.size(); i++)
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
		switch (valueType)
		{
		case 0:
		{
			accessor.getter = [&]()-> float {
				return GetOwner()->transform->position.x;
				};
			accessor.setter = [&](float value) {
				GetOwner()->transform->position.x = value;
				GetOwner()->transform->MarkNeedsUpdate();
				};
			break;
		}
		case 1:
		{
			accessor.getter = [&]()-> float {
				return GetOwner()->transform->position.y;
				};
			accessor.setter = [&](float value) {
				GetOwner()->transform->position.y = value;
				GetOwner()->transform->MarkNeedsUpdate();
				};
			break;
		}
		case 2:
		{
			accessor.getter = [&]()-> float {
				return GetOwner()->transform->position.z;
				};
			accessor.setter = [&](float value) {
				GetOwner()->transform->position.z = value;
				GetOwner()->transform->MarkNeedsUpdate();
				};
			break;
		}
		case 3:
		{
			accessor.getter = [&]()-> float {
				return GetOwner()->transform->scale.x;
				};
			accessor.setter = [&](float value) {
				GetOwner()->transform->scale.x = value;
				GetOwner()->transform->MarkNeedsUpdate();
				};
			break;
		}
		case 4:
		{
			accessor.getter = [&]()-> float {
				return GetOwner()->transform->scale.y;
				};
			accessor.setter = [&](float value) {
				GetOwner()->transform->scale.y = value;
				GetOwner()->transform->MarkNeedsUpdate();
				};
			break;
		}
		case 5:
		{
			accessor.getter = [&]()-> float {
				return GetOwner()->transform->scale.z;
				};
			accessor.setter = [&](float value) {
				GetOwner()->transform->scale.z = value;
				GetOwner()->transform->MarkNeedsUpdate();
				};
			break;
		}
		case 6:
		{
			accessor.getter = [&]()-> float {
				return GetOwner()->transform->GetEulerAngles().x;
				};
			accessor.setter = [&](float value) {
				Vector3 angles = GetOwner()->transform->GetEulerAngles();
				GetOwner()->transform->SetRotation({ value, angles.y, angles.z });
				GetOwner()->transform->MarkNeedsUpdate();
				};
			break;
		}
		case 7:
		{
			accessor.getter = [&]()-> float {
				return GetOwner()->transform->GetEulerAngles().y;
				};
			accessor.setter = [&](float value) {
				Vector3 angles = GetOwner()->transform->GetEulerAngles();
				GetOwner()->transform->SetRotation({ angles.x, value, angles.z });
				GetOwner()->transform->MarkNeedsUpdate();
				};
			break;
		}
		case 8:
		{
			accessor.getter = [&]()-> float {
				return GetOwner()->transform->GetEulerAngles().z;
				};
			accessor.setter = [&](float value) {
				Vector3 angles = GetOwner()->transform->GetEulerAngles();
				GetOwner()->transform->SetRotation({ angles.x, angles.y, value });
				GetOwner()->transform->MarkNeedsUpdate();
				};
			break;
		}
		default:
			break;
		}

		StartHandler(handler, accessor);
	}
#endif // USE_IMGUI
}