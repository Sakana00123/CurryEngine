#include "pch.h"
#include "EffekseerEffectComponent.h"
#include "Engine/Core/Transform.h"

REGISTER_COMPONENT(EffekseerEffectComponent, "Effects")


void EffekseerEffectComponent::Start()
{
	// エフェクトをロードする
	ReloadEffect();
}

void EffekseerEffectComponent::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void EffekseerEffectComponent::OnTransformChanged()
{
	// Transformの変更を検知してエフェクトの位置・回転・スケールを更新する
	if (effectHandle != -1)
	{
		Transform* transform = GetTransform();
		if (transform)
		{
			EffekseerEffectManager::SetPosition(effectHandle, transform->GetWorldPosition());
			EffekseerEffectManager::SetRotation(effectHandle, transform->GetWorldRotation());
			EffekseerEffectManager::SetScale(effectHandle, transform->GetWorldScale());
		}
	}
}

void EffekseerEffectComponent::ReloadEffect()
{
	// エフェクトをロードする
	effectHandle = EffekseerEffectManager::LoadRequest(effectAssetId);
}

void EffekseerEffectComponent::PlayEffect()
{
	// エフェクトを再生する
	if (effectHandle != -1)
	{
		EffekseerEffectManager::PlayRequestParameter param;
		param.handle = effectHandle;
		Transform* transform = GetTransform();
		if (transform)
		{
			param.position = transform->GetWorldPosition();
			param.rotation = transform->GetWorldRotation();
			param.scale = transform->GetWorldScale();
		}
		else
		{
			param.position = Vector3::Zero;
			param.rotation = Quaternion::Identity;
			param.scale = Vector3(1.0f, 1.0f, 1.0f);
		}
		EffekseerEffectManager::PlayRequest(param);
	}
	else
	{
		LOG_ERROR(u8"[EffekseerEffectComponent] エフェクトがロードされていません。AssetId: " + std::u8string(effectAssetId.ToString().begin(), effectAssetId.ToString().end()));
	}
}

void EffekseerEffectComponent::StopEffect()
{
	// エフェクトを停止する
	if (effectHandle != -1)
	{
		EffekseerEffectManager::StopRequest(effectHandle);
	}
	else
	{
		LOG_ERROR(u8"[EffekseerEffectComponent] エフェクトがロードされていません。AssetId: " + std::u8string(effectAssetId.ToString().begin(), effectAssetId.ToString().end()));
	}
}
