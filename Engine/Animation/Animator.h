#pragma once
#include "Engine/Core/Component.h"
#include "AnimatorController.h"

class Animator : public Component
{
	C_REFLECT(Animator)
public:
	Animator() = default;
	virtual ~Animator() = default;

	void Initialize() override;

	void Update(float deltaTime) override;

#ifdef USE_IMGUI
	/** @brief インスペクタ用プロパティ表示。*/
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

private:
	
	std::unique_ptr<AnimatorController> controller;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GltfModelRenderer"))
	ObjectId targetModelRendererId; // アニメーションを適用する対象の GltfModelRenderer の ObjectId

};
