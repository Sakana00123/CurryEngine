#include "pch.h"
#include "UIComponent.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(UIComponent, "UI", ComponentAttributes::HideInAddComponentMenu, {})

RectTransform* UIComponent::GetRectTransform()
{
	if (!rect) {
		if (auto* rectTransform = GetOwner()->GetComponent<RectTransform>()) {
			SetRectTransform(rectTransform);
		}
	}
	return rect;
}

void UIComponent::Awake()
{
	if (auto* rect = gameObject->GetComponent<RectTransform>()) {
		gameObject->transform = rect;
		SetRectTransform(rect);
	}
	else {
		gameObject->RemoveComponent<Transform>();
		rect = gameObject->AddComponent<RectTransform>();
		gameObject->transform = rect;
		SetRectTransform(rect);
	}
}

void UIComponent::SetInputEnabled(bool enabled)
{
	isInputEnabled = enabled;
}