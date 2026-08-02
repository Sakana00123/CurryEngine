#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Easing/EasingHandler.h"

class BlendEasing : public Component
{
	C_REFLECT(BlendEasing)
public:
	BlendEasing() = default;
	~BlendEasing() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	//void DrawProperty() override; // エディタでプロパティを描画するためのオーバーライド関数

	// ブレンドを開始する関数。引数はブレンドの目標位置(0~1)。
	void StartBlend(float t = 1.0f); 

private:

	C_PROPERTY()
	Vector3 startPosition; // ブレンド開始位置

	C_PROPERTY()
	Vector3 targetPosition; // ブレンド目標位置

	C_PROPERTY()
	Vector3 startRotation; // ブレンド開始回転

	C_PROPERTY()
	Vector3 targetRotation; // ブレンド目標回転


	C_PROPERTY()
	float blendDuration = 1.0f; // ブレンドの持続時間

	C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly, CurryEngine::PropertyAttributes::NonSerialized)
	float elapsedTime = 0.0f; // 経過時間

	C_PROPERTY(CurryEngine::PropertyAttributes::HideInInspector)
	int easingType = 0; // イージングタイプの選択肢（例: 0=Linear, 1=EaseIn, 2=EaseOut, etc.）

	EasingHandler easingHandler; // イージングハンドラー
};
