#pragma once

#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

/// <summary>
/// スマートボール風の動作をするブロック用コンポーネント
/// 左クリック長押しでz負の方向に下降し、離すと元の位置に戻る
/// 押下時間が短いほど戻る速度が遅い
/// </summary>
class SpringPlunger : public Component
{
	C_REFLECT(SpringPlunger)

private:
	// 状態管理
	C_PROPERTY()
		bool isPressed = false;

	C_PROPERTY()
		float pressedElapsed = 0.0f;

	C_PROPERTY()
		float maxDownDistance = 2.0f;	// 最大下降距離

	// 戻る速度設定
	C_PROPERTY()
		float minReturnSpeed = 1.0f;	// 最小押下時間での戻る速度
	C_PROPERTY()
		float maxReturnSpeed = 4.0f;	// 最大押下時間での戻る速度
	C_PROPERTY()
		float maxPressTime = 1.0f;		// 最大戻る速度に到達する押下時間
	C_PROPERTY()
		float returnSpeedCurveExponent = 0.5f;	// 戻る速度の曲線（小さいほど早期に上昇）

	// パワー設定
	C_PROPERTY()
		float minLaunchPower = 15.0f;	// 最小発射パワー
	C_PROPERTY()
		float maxLaunchPower = 50.0f;	// 最大発射パワー
	C_PROPERTY()
		float minPressTime = 0.3f;		// 発射に必要な最小押下時間

	// パワーカーブ調整
	C_PROPERTY()
		float powerCurveExponent = 3.0f;	// 小さいほど短い押下時間で高パワー

	// 内部状態
	Vector3 initialPosition = Vector3::Zero;
	Vector3 currentDownOffset = Vector3::Zero;	// 現在の下降オフセット

	// マウス判定用
	C_PROPERTY()
		bool canInteract = true;	// インタラクション有効

	// 横揺れ設定
	C_PROPERTY()
		float maxShakeAmplitude = 0.15f;   // 最大横揺れ幅（X軸）
	C_PROPERTY()
		float shakeFrequency = 12.0f;      // 揺れの速さ（Hz）
	C_PROPERTY()
		float shakeCurveExponent = 2.0f;   // 揺れ加速カーブ（大きいほど後半に集中）

	// 内部状態（横揺れ用）
	float shakeTime = 0.0f;               // サイン波の位相タイマー
	float currentShakeOffset = 0.0f;      // 現在のX揺れオフセット

public:
	SpringPlunger() = default;
	virtual ~SpringPlunger() override = default;

	void Initialize() override;
	void Update(float deltaTime) override;

private:
	/// <summary>
	/// 押下時間に応じてボールを発射する
	/// </summary>
	/// <param name="pressTime">ボタンが押されていた時間（秒）</param>
	void LaunchBall(float pressTime);

	/// <summary>
	/// 押下時間から戻る速度を計算する
	/// </summary>
	/// <param name="pressTime">ボタンが押されていた時間（秒）</param>
	/// <returns>押下時間に応じた戻る速度</returns>
	float GetReturnSpeed(float pressTime) const;
};