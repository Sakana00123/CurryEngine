#pragma once
#include "Engine/Core/Component.h"

class BeatScalerComponent : public Component
{
	C_REFLECT(BeatScalerComponent)
public:
	/** @brief ビートに合わせてスケーリングする強さ。*/
	C_PROPERTY()
	float scaleIntensity = 1.0f;
	/** @brief スケーリングの速さ。*/
	C_PROPERTY()
	float scaleSpeed = 5.0f;

	/** @brief 何ビートに一回スケーリングするか。*/
	C_PROPERTY()
	int scaleFrequency = 1;
public:
	BeatScalerComponent() = default;
	virtual ~BeatScalerComponent() = default;
	
	void Start() override;

	void Update(float deltaTime) override;

	void DrawProperty() override;

	void OnBeat();
private:
	float baseScale = 1.0f;
	float targetScale = 1.0f;
	float currentScale = 1.0f;
	int beatCount = 0; // ビートカウント
};