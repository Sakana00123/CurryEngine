#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class BeatScaler2DComponent : public Component
{
	C_REFLECT(BeatScaler2DComponent)
public:
	/** @brief スケールの強さ。*/
	C_PROPERTY()
	float scaleIntensity = 0.2f;
	/** @brief スケールの速度。*/
	C_PROPERTY()
	float scaleSpeed = 10.0f;
	/** @brief スケールの頻度（何ビートに一回スケーリングするか）。*/
	C_PROPERTY()
	int scaleFrequency = 1;

private:
	/** @brief 初期サイズ。*/
	Vector2 initialSize = { 0,0 };
	/** @brief 基本スケール。*/
	float baseScale = 1.0f;
	/** @brief 現在のスケール。*/
	float currentScale = 1.0f;
	/** @brief ターゲットスケール。*/
	float targetScale = 1.0f;
	/** @brief ビートカウント。*/
	int beatCount = 0;
public:
	/** @brief 開始処理。*/
	virtual void Start() override;
	/**
	 * @brief 更新処理。
	 * @param deltaTime 前フレームからの経過時間（秒）。
	 */
	virtual void Update(float deltaTime) override;
	/** @brief プロパティ描画。*/
	virtual void DrawProperty() override;
private:
	/** @brief ビート時の処理。*/
	void OnBeat();
};
