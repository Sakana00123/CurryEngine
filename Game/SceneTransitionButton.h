#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Easing/EasingHandler.h"

class SceneTransitionButton : public Component
{
	C_REFLECT(SceneTransitionButton)
public:
	SceneTransitionButton() = default;
	~SceneTransitionButton() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void StartSceneTransition(); // シーン遷移を開始する関数

	void TransitionScene(); // シーン遷移を実行する関数

public:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("RankingManager"))
	ObjectId rankingManagerReference; // ランキングマネージャーオブジェクトへの参照

	C_PROPERTY()
	std::string transitionSceneName; // 遷移先のシーン名

	C_PROPERTY()
	bool transitionTutorial = false; // チュートリアルに遷移するかどうかのフラグ

	C_PROPERTY()
	float fadeDuration = 1.0f; // シーン遷移の時間（秒）

	C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly, CurryEngine::PropertyAttributes::NonSerialized)
	float fadeValue = 0.0f; // フェードの進行度（0.0f から 1.0f）

private:
	EasingHandler fadeEasing; // フェードのイージングハンドラー
	bool isFading = false; // フェード中かのフラグ

};