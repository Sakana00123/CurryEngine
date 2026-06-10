#pragma once
#include "Selectable.h"
class Slider : public Selectable, public IDragHandler, public IEndDragHandler
{
	C_REFLECT(Slider)
private:
	bool isDragging = false;
public:
	//Horizontal：水平、Vertical：垂直
	enum class Direction { LeftToRight, RightToLeft, TopToBottom, BottomToTop };
public:
	Slider() = default;
	~Slider() override = default;

	void Initialize() override;

	void OnPointerDown(PointerEventData* eventData) override;

	void OnEndDrag(PointerEventData* eventData) override;

	void OnDrag(PointerEventData* eventData) override;

#ifdef USE_IMGUI
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

	void SetValue(float value);

	float GetValue() const;

	bool IsDragging() const;

	void SetDirection(const Direction& direction);

	template<class T>
	void AddValueChangeFunction(void (T::* func)(float), T* instance) {
		onValueChangedFunctions.emplace_back([=](float value) {(instance->*func)(value); });
	}

	void AddValueChangeFunction(void (*func)(float)) {
		onValueChangedFunctions.emplace_back(func);
	}

private:

	void UpdateSliderValue(const XMFLOAT2& mousePos);

	void UpdateVisuals(float normalized);

public:
	float maxValue = 1.f;
	float minValue = 0.f;

	RectTransform* fillRect = nullptr;
	RectTransform* handleRect = nullptr;
private:
	float value = 0.f;
	float normalizedValue = 0.f;
	Direction direction = Direction::LeftToRight;

	//整数のみ使用できるようにするか
	bool wholeNumbers = false;
	std::vector<std::function<void(float)>> onValueChangedFunctions;

};