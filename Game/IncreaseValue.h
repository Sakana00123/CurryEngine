#pragma once
#include "Gadget.h"
#include "Engine/Physics/CollisionEvent.h"
#include <unordered_set>

class Ball;

class IncreaseValue : public Gadget
{
    C_REFLECT(IncreaseValue)
public:
    IncreaseValue() = default;
    ~IncreaseValue() = default;

    void Start() override;
    void Update(float deltaTime) override;
	void OnTriggerEnter(const TriggerInfo& info);
    //void OnTriggerStay(const TriggerInfo& info);
	void OnTriggerExit(const TriggerInfo& info);

    void OnActivate() override; // ガジェットがアクションを実行したときの処理
    void OnDeactivate() override; // ガジェットがアクションを実行したときの処理
    
	void OnPreviewEnter() override; // ピンに近づけたときの処理
	void OnPreviewExit() override; // ピンから離れたときの処理

    void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理

	void OnRoundEnd() override; // ラウンド終了時の処理

	void ClearBallSet() override; // 管理しているボールのセットをクリアするための関数。必要に応じてオーバーライドして実装します。

	void SetDisabled(bool disabled) override; // ガジェットが無効化されたときの処理。必要に応じてオーバーライドして実装します。

private:
    C_PROPERTY()
        float increaseSpeed = 1.0f;  // 1秒に増加する回数

    float increaseTimer = 0.0f;
    std::unordered_set<Ball*> ballsInRange;  // Stay中のボールを管理
};