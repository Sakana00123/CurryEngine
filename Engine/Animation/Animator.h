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

	// シリアライズ
	json Serialize() const override;

	// デシリアライズ
	void Deserialize(const json& jsonData) override;

private:
	
	std::shared_ptr<AnimatorController> controller;

	RuntimeAnimatorController runtimeController;

	C_PROPERTY(CurryEngine::PropertyAttributes::CustomDrawer("AssetId"), CurryEngine::PropertyAttributes::AssetTypeExtension(".controller"), CurryEngine::PropertyAttributes::OnPropertyChanged("ResetController"))
	CurryEngine::Resources::AssetId controllerAssetId; // AnimatorController の AssetId

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GltfModelRenderer"))
	ObjectId targetModelRendererId; // アニメーションを適用する対象の GltfModelRenderer の ObjectId

};
