#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Effects/EffekseerEffectManager.h"

class EffekseerEffectComponent : public Component
{
	C_REFLECT(EffekseerEffectComponent)
public:
	EffekseerEffectComponent() = default;
	~EffekseerEffectComponent() = default;

public:

	void Start() override;
	void Update(float deltaTime) override;

	// Transformの変更を検知してエフェクトの位置・回転・スケールを更新する
	void OnTransformChanged() override;

	// EffekseerエフェクトのアセットIDを更新し、エフェクトをロードする
	C_FUNCTION()
	void ReloadEffect();

	// Effekseerエフェクトを再生する
	C_FUNCTION()
	void PlayEffect();

	// Effekseerエフェクトを停止する
	C_FUNCTION()
	void StopEffect();

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::CustomDrawer("AssetId"), CurryEngine::PropertyAttributes::AssetTypeExtension(".efk"), CurryEngine::PropertyAttributes::OnPropertyChanged("ReloadEffect"))
	CurryEngine::Resources::AssetId effectAssetId; // EffekseerエフェクトのアセットID

	EffekseerEffectManager::UserHandle effectHandle = -1; // Effekseerエフェクトのユーザーハンドル
};
