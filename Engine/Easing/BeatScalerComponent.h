#pragma once
#include "Engine/Core/Component.h"

/** @brief BeatScalerComponent を表すクラスです。 */
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
	/**
	 * @brief BeatScalerComponent を構築します。
	 */
	BeatScalerComponent() = default;
	/**
	 * @brief BeatScalerComponent を破棄します。
	 */
	virtual ~BeatScalerComponent() = default;
	
	/**
	 * @brief Start の処理を行います。
	 */
	void Start() override;

	/**
	 * @brief 状態を更新します。
	 */
	void Update(float deltaTime) override;

#ifdef USE_IMGUI
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

	/**
	 * @brief Beat イベントを処理します。
	 */
	void OnBeat();
private:
	float baseScale = 1.0f;
	float targetScale = 1.0f;
	float currentScale = 1.0f;
	int beatCount = 0; // ビートカウント
};