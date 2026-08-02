#pragma once
#include "Gadget.h"
#include "Engine/Physics/CollisionEvent.h"

class Flipper : public Gadget
{
	C_REFLECT(Flipper)
public:
	/**@brief フリッパーの回転速度。*/
	C_PROPERTY()
	float flipForce = 500.0f;

	/**@brief フリッパーの回転方向。trueで右回転、falseで左回転。*/
	C_PROPERTY()
	bool isRightFlipper = true;

	/**@brief フリッパーの回転角度。*/
	C_PROPERTY()
	float flipAngle = 45.0f;

public:
	Flipper() = default;
	virtual ~Flipper() = default;
	void Start() override;
	void Update(float deltaTime) override;

	void OnRoundEnd() override; // ラウンド終了時の処理

	void OnCollisionEnter(const CollisionInfo& collision); // 衝突開始イベントの処理
	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理

private:
	float startAngle = 0.0f; // フリッパーの初期角度
	float currentFlipRate = 0.0f; // 現在のフリッパーの回転進捗(0.0fから1.0fの範囲)
	int flipDirection = 0; // フリッパーの回転方向（1: 回転中、-1: 反転中、0: 停止）

};