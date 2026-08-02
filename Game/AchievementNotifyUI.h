#pragma once
#include "Engine/Core/Component.h"
#include "Engine/UI/Text.h"
#include "Engine/UI/Image.h"

class AchievementNotifyUI : public Component {
	C_REFLECT(AchievementNotifyUI)
public:
	void Start() override;
	void Update(float deltaTime) override;

	void Show(const std::wstring& name, const std::wstring& description);
	bool IsShowing() const { return isShowing; }

private:
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"))
		ObjectId backgroundRef;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId nameTextRef;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId descriptionTextRef;

	float timer = 0.0f;
	const float duration = 4.0f;
	bool isShowing = false;

	void ApplyAlpha(float alpha);
};