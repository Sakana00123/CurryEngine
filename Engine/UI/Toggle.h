#pragma once
#include "Selectable.h"
#include "Text.h"

class Toggle : public Selectable, public IPointerClickHandler, public ISubmitHandler
{
	C_REFLECT(Toggle)
public:
	using Callback = std::function<void(bool)>;

	Toggle() = default;
	~Toggle() override = default;

	void Initialize() override;

	void Begin(RenderContext* rtx) override;

	void DrawProperty() override;

	// シリアライズ
	json Serialize() const override;

	// デシリアライズ
	void Deserialize(const json& j) override;

public:

	void SetIsOn(bool isOn);
	bool IsOn() const;

	void AddCallback(std::function<void(bool)> func);
protected:
	void OnPointerClick(PointerEventData* eventData) override;
	void OnSubmit(BaseEventData* eventData) override;
private:
	void Notify();
public:
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"))
	ObjectId checkMarkReference;

private:
	bool isOn = false;
	std::vector<Callback> callbacks;
};