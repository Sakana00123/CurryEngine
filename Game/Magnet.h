#pragma once
#include "Gadget.h"
#include "Engine/Physics/CollisionEvent.h"
#include <unordered_set>

class Ball;

class Magnet : public Gadget
{
    C_REFLECT(Magnet)
public:
    Magnet() = default;
    ~Magnet() = default;

    void Start() override;
    void Update(float deltaTime) override;
    void OnTriggerStay(const TriggerInfo& info);
    
	void OnRoundEnd() override; // ラウンド終了時の処理

	void OnActivate() override; // ガジェットがアクションを実行したときの処理
	void OnDeactivate() override; // ガジェットがアクションを実行したときの処理

	void OnPreviewEnter() override; // ピンに近づけたときのプレビュー表示開始の処理
	void OnPreviewExit() override; // ピンから離れたときのプレビュー表示終了の処理


	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理

	void ClearBallSet() override; // 管理しているボールのセットをクリアするための関数。必要に応じてオーバーライドして実装します。

	void SetDisabled(bool disabled) override; // ガジェットが無効化されたときの処理。必要に応じてオーバーライドして実装します。

private:
    C_PROPERTY()
        float strength = 10.0f;// 磁力の強さ

    C_PROPERTY()
		float magnetTime = 3.0f; // 磁力が持続する時間（秒）

    C_PROPERTY()
		float cooldownTime = 2.0f; // 磁力のクールダウン時間（秒）

    C_PROPERTY()
	float currentMagnetTime = 0.0f; // 現在の磁力持続時間

	C_PROPERTY()
	float currentCooldown = 0.0f; // 現在のクールダウン時間

	bool isActive = true; // 磁力が現在発動中かどうか

	float audioCooldown = 0.0f; // 音のクールダウンタイマー
	//std::unordered_set<GameObject*> affectedObjects; // 磁力の影響を受けているオブジェクトのセット
};