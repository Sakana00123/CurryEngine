#pragma once
#include "Selectable.h"
/** @brief Slider を表すクラスです。 */
class Slider : public Selectable, public IDragHandler, public IEndDragHandler
{
	C_REFLECT(Slider)
private:
	bool isDragging = false;
public:
	//Horizontal：水平、Vertical：垂直
	enum class Direction { LeftToRight, RightToLeft, TopToBottom, BottomToTop };
public:
	/**
	 * @brief Slider を構築します。
	 */
	Slider() = default;
	/**
	 * @brief Slider を破棄します。
	 */
	~Slider() override = default;

	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;

	/**
	 * @brief PointerDown イベントを処理します。
	 */
	void OnPointerDown(PointerEventData* eventData) override;

	/**
	 * @brief EndDrag イベントを処理します。
	 */
	void OnEndDrag(PointerEventData* eventData) override;

	/**
	 * @brief Drag イベントを処理します。
	 */
	void OnDrag(PointerEventData* eventData) override;

#ifdef USE_IMGUI
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

	/**
	 * @brief Value を設定します。
	 */
	void SetValue(float value);

	/**
	 * @brief GetValue に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	float GetValue() const;

	/**
	 * @brief IsDragging の条件を満たすか判定します。
	 * @return 処理結果を返します。
	 */
	bool IsDragging() const;

	/**
	 * @brief Direction を設定します。
	 */
	void SetDirection(const Direction& direction);

	template<class T>
	void AddValueChangeFunction(void (T::* func)(float), T* instance) {
		onValueChangedFunctions.emplace_back([=](float value) {(instance->*func)(value); });
	}

	void AddValueChangeFunction(void (*func)(float)) {
		onValueChangedFunctions.emplace_back(func);
	}

private:

	/**
	 * @brief 状態を更新します。
	 */
	void UpdateSliderValue(const XMFLOAT2& mousePos);

	/**
	 * @brief 状態を更新します。
	 */
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