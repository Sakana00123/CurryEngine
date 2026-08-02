#pragma once
#include "Engine/UI/UIComponent.h"
#include "Engine/Easing/EasingHandler.h"

class UIEasing : public UIComponent
{
	C_REFLECT(UIEasing)
public:
	UIEasing() = default;
	~UIEasing() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;


	// イージングを開始する関数。引数はイージングの目標位置。
	void StartEasing(float t = 1.0f, std::function<void()> onComplete = nullptr);

private:

	C_PROPERTY()
	Vector2 startPosition; // イージング開始位置

	C_PROPERTY()
	Vector2 targetPosition; // イージング目標位置

	C_PROPERTY()
	float easingDuration = 1.0f; // イージングの持続時間

	C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly, CurryEngine::PropertyAttributes::NonSerialized)
	float elapsedTime = 0.0f; // 経過時間

	C_PROPERTY(CurryEngine::PropertyAttributes::HideInInspector)
	int easingType = 0; // イージングタイプの選択肢

	EasingHandler easingHandler; // イージングハンドラー

	float t = -1.0f; // イージングの進捗（0〜1）
	std::function<void()> onCompleteCallback; // イージング完了時のコールバック関数
};
