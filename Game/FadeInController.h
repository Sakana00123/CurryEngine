#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Easing/EasingHandler.h"

class FadeInController : public Component
{
	C_REFLECT(FadeInController)
public:
	FadeInController() = default;
	~FadeInController() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;
	
	C_PROPERTY()
		float fadeDuration = 1.0f; // シーン遷移の時間（秒）

	C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly, CurryEngine::PropertyAttributes::NonSerialized)
		float fadeValue = 0.0f; // フェードの進行度（0.0f から 1.0f）

private:
	EasingHandler fadeEasing; // フェードのイージングハンドラー
	bool isFading = true; // フェード中かのフラグ
};