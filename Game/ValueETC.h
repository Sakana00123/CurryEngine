#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Physics/CollisionEvent.h"
#include <set>
#include "Gadget.h"

class Ball;

class ValueETC : public Gadget
{
    C_REFLECT(ValueETC)
public:
    ValueETC() = default;
    ~ValueETC() = default;

    void Start() override;
    void Update(float deltaTime) override;
    void OnTriggerEnter(const TriggerInfo& info);
    void OnTriggerExit(const TriggerInfo& info);

	void OnActivate() override; // ガジェットがアクションを実行したときの処理
	void OnDeactivate() override; // ガジェットがアクションを実行したときの処理
    
	void OnPreviewEnter() override; // ピンに近づけたときのプレビュー表示開始
	void OnPreviewExit() override; // ピンから離れたときのプレビュー表示終了
	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理

	void OnRoundEnd() override; // ラウンドの終了時の処理

private:
    int decreaseAmount = 1;

    std::set<Ball*> triggeredBalls;  // すでに減らしたボールを管理
};