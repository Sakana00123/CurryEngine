#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Easing/EasingHandler.h"

class Rotater : public Component
{
	C_REFLECT(Rotater)
public:
	Rotater() = default;
	~Rotater() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

private:

	bool is2D = false; // 2D回転かどうかのフラグ

	C_PROPERTY()
	bool rotateX = false; // X軸回転の有無

	C_PROPERTY()
	bool rotateY = false; // Y軸回転の有無

	C_PROPERTY()
	bool rotateZ = false; // Z軸回転の有無

	C_PROPERTY()
	float rotationSpeed = 90.0f; // 回転速度（度/秒）

	// 緩急のついた回転をするかどうかのフラグ。true の場合、イージング関数を使用して回転速度を変化させます。
	C_PROPERTY()
		bool useEasing = false;

	float elapsedTime = 0.0f; // 回転開始からの経過時間を追跡する変数
	EasingHandler easingHandler; // イージングハンドラーを使用して回転の緩急を制御
};