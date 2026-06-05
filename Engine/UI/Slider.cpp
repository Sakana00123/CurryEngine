#include "pch.h"
#include "Slider.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(Slider, "UI", ComponentAttributes::DisallowMultiple, {});

void Slider::Initialize()
{
	Selectable::Initialize();
	SetDirection(direction);
	SetValue(value);
	//rect->size = { direction == Direction::Horizontal ? 300.f : 50.f, direction == Direction::Horizontal ? 50.f : 300.f };
}

void Slider::OnPointerDown(PointerEventData* eventData)
{
	isDragging = true;
	UpdateSliderValue(eventData->position);
	Selectable::OnPointerDown(eventData);
}

void Slider::OnEndDrag(PointerEventData* eventData)
{
	isDragging = false;
	Selectable::OnPointerUp(eventData);
}

void Slider::OnDrag(PointerEventData* eventData)
{
	UpdateSliderValue(eventData->position);
}

void Slider::DrawProperty()
{
#ifdef USE_IMGUI
	Selectable::DrawProperty();

	static const char* items[] = { "LeftToRight", "RightToLeft", "TopToBottom", "BottomToTop" };
	if (ImGui::BeginCombo("Direction", items[static_cast<int>(direction)])) {
		for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
			bool isSelected = (static_cast<int>(direction) == i);
			if (ImGui::Selectable(items[i], isSelected)) {
				//SetDirection(static_cast<Direction>(i));
				direction = static_cast<Direction>(i);
			}

			//同じアイテムが選ばれていたらフォーカスを与える
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::DragFloat("min", &minValue);
	ImGui::DragFloat("max", &maxValue);
	ImGui::DragFloat("Value", &this->value);
	ImGui::DragFloat("NormalizedValue", &this->normalizedValue);
	ImGui::Checkbox("WholeNumbers", &wholeNumbers);
#endif // USE_IMGUI
}

void Slider::SetValue(float value)
{
	float oldValue = this->value;
	float newValue = std::clamp(value, minValue, maxValue);
	this->value = wholeNumbers ? roundf(newValue) : newValue;
	float normalized = (this->value - minValue) / (maxValue - minValue);
	UpdateVisuals(normalized);

	if (oldValue != this->value) {
		for (auto& func : onValueChangedFunctions) {
			func(this->value);
		}
	}
}

float Slider::GetValue() const
{
	return this->value;
}

bool Slider::IsDragging() const
{
	return isDragging;
}

void Slider::SetDirection(const Direction& direction)
{
	RectTransform* rect = GetRectTransform();
	this->direction = direction;
	switch (direction)
	{
	case Direction::LeftToRight:
		if (fillRect) {
			fillRect->size = { rect->size.x, rect->size.y };
		}
		if (handleRect) {
			handleRect->anchorMin = rect->anchorMin;
			handleRect->anchorMax = rect->anchorMax;
		}
		break;
	case Direction::RightToLeft:
		if (fillRect) {
			fillRect->size = { rect->size.x, rect->size.y };
		}
		if (handleRect) {
			handleRect->anchorMin = rect->anchorMax;
			handleRect->anchorMax = rect->anchorMin;
		}
		break;
	case Direction::TopToBottom:
		if (fillRect) {
			fillRect->size = { rect->size.y, rect->size.x };
		}
		if (handleRect) {
			handleRect->anchorMin = { rect->anchorMin.y, rect->anchorMin.x };
			handleRect->anchorMax = { rect->anchorMax.y, rect->anchorMax.x };
		}
		break;
	case Direction::BottomToTop:
		if (fillRect) {
			fillRect->size = { rect->size.y, rect->size.x };
		}
		if (handleRect) {
			handleRect->anchorMin = { rect->anchorMax.y, rect->anchorMax.x };
			handleRect->anchorMax = { rect->anchorMin.y, rect->anchorMin.x };
		}
		break;
	}
	UpdateVisuals(normalizedValue);
}

void Slider::UpdateSliderValue(const XMFLOAT2& mousePos)
{
	RectTransform* rect = GetRectTransform();
	if (!rect) return;
	if (!handleRect) return;
	const XMFLOAT2 trackCenter = handleRect->GetParent() ?
		handleRect->GetParent()->GetWorldPosition() :
		rect->GetWorldPosition();
	const XMFLOAT2 trackSize = handleRect->GetParent() ?
		handleRect->GetParent()->GetWorldSize() :
		rect->GetWorldSize();
	const XMFLOAT2 trackPos = trackCenter;
	float normalized = 0.0f;
	switch (direction) {
	case Slider::Direction::LeftToRight:
		normalized = (mousePos.x - trackPos.x) / trackSize.x;
		break;
	case Slider::Direction::RightToLeft:
		normalized = 1.0f - (mousePos.x - trackPos.x) / trackSize.x;
		break;
	case Slider::Direction::BottomToTop:
		normalized = (mousePos.y - trackPos.y) / trackSize.y;
		break;
	case Slider::Direction::TopToBottom:
		normalized = 1.0f - (mousePos.y - trackPos.y) / trackSize.y;
		break;
	}
	normalized = std::clamp(normalized, 0.0f, 1.0f);
	SetValue(minValue + (maxValue - minValue) * normalized);
}

void Slider::UpdateVisuals(float normalized)
{
	normalizedValue = normalized;
	// Handle の Anchor を更新
	if (handleRect && handleRect->GetParent()) {
		Vector2 anchor = { 0.5f,0.5f };
		switch (direction) {
		case Slider::Direction::LeftToRight:
			anchor.x = normalized;
			break;
		case Slider::Direction::RightToLeft:
			anchor.x = 1.f - normalized;
			break;
		case Slider::Direction::BottomToTop:
			anchor.y = normalized;
			break;
		case Slider::Direction::TopToBottom:
			anchor.y = 1.f - normalized;
			break;
		}
		handleRect->SetAnchorMin(anchor);
		handleRect->SetAnchorMax(anchor);
	}
	// Fill の Anchor を更新
	RectTransform* fillContainerRect = fillRect ?
		dynamic_cast<RectTransform*>(fillRect->gameObject->parent->transform) :
		nullptr;
	if (fillContainerRect) {
		Vector2 anchorMin = { 0,0 };
		Vector2 anchorMax = { 1,1 };
		Vector2 containerAnchor = {};
		switch (direction) {
		case Slider::Direction::LeftToRight:
			anchorMin.x = 0.f;
			anchorMax.x = normalized;
			containerAnchor = { 0,0.5f };
			break;
		case Slider::Direction::RightToLeft:
			anchorMin.x = 1.f - normalized;
			anchorMax.x = 1.f;
			containerAnchor = { 0, 0.5f };
			break;
		case Slider::Direction::BottomToTop:
			anchorMin.y = 0.f;
			anchorMax.y = normalized;
			break;
		case Slider::Direction::TopToBottom:
			anchorMin.y = 1.f - normalized;
			anchorMax.y = 1.f;
			break;
		}
		
		fillRect->SetAnchorMin(anchorMin);
		fillRect->SetAnchorMax(anchorMax);
	}
}