#pragma once
#include "Engine/Core/Component.h"
#include "AnimatorController.h"

class Animator : public Component
{
	C_REFLECT(Animator)
public:
	Animator() = default;
	virtual ~Animator() = default;

	void Awake() override;

	void Update(float deltaTime) override;

#ifdef USE_IMGUI
	/** @brief インスペクタ用プロパティ表示。*/
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

	C_FUNCTION()
	void ResetController();

	C_FUNCTION()
	void SetFloat(const char* name, float value);
	C_FUNCTION()
	void SetInt(const char* name, int value);
	C_FUNCTION()
	void SetBool(const char* name, bool value);
	C_FUNCTION()
	void SetTrigger(const char* name);

	// シリアライズ
	json Serialize() const override;

	// デシリアライズ
	void Deserialize(const json& jsonData) override;

private:
	
	std::shared_ptr<AnimatorController> controller;
	std::shared_ptr<RuntimeAnimatorController> runtimeController;

	C_PROPERTY(CurryEngine::PropertyAttributes::CustomDrawer("AssetId"), CurryEngine::PropertyAttributes::AssetTypeExtension(".controller"), CurryEngine::PropertyAttributes::OnPropertyChanged("ResetController"))
	CurryEngine::Resources::AssetId controllerAssetId; // AnimatorController の AssetId

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GltfModelRenderer"))
	ObjectId targetModelRendererId; // アニメーションを適用する対象の GltfModelRenderer の ObjectId

};
