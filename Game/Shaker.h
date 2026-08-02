#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Easing/EasingHandler.h"

class Shaker : public Component
{
	C_REFLECT(Shaker)
public:
	Shaker() = default;
	~Shaker() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	// シェイクを開始する関数。duration はシェイクの持続時間、magnitude はシェイクの強さを表します。
	void Shake(float duration, float magnitude);

	//void DrawProperty() override; // エディタでプロパティを描画するためのオーバーライド関数

private:

	float shakeDuration = 0.0f; // シェイクの残り時間
	float shakeMagnitude = 0.0f; // シェイクの強さ
	Vector3 originalPosition; // シェイク前のオブジェクトの位置を保存する変数
	EasingHandler easingHandler; // イージングハンドラーを使用してシェイクの減衰を制御

};
