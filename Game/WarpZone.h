#pragma once
#include "Gadget.h"
#include "Engine/Physics/CollisionEvent.h"
#include "Engine/Core/GameObject.h"
#include <memory>

class Rigidbody;

class WarpZone : public Gadget
{
	C_REFLECT(WarpZone)
public:
	WarpZone() = default;
	~WarpZone() = default;

public:

	void Start() override;
	void Update(float deltaTime) override;

	void OnTriggerEnter(const TriggerInfo& info); // トリガーに入ったときの処理
	void OnAction() override;

	void OnRoundEnd() override; // ラウンド終了時の処理

	void OnActivate() override; // ガジェットがアクションを実行したときの処理
	void OnDeactivate() override; // ガジェットが非アクティブ化されたときの処理

	void OnPreviewEnter() override {} // ピンに近づけたときの処理
	void OnPreviewExit() override {} // ピンから離れたときの処理

	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理

private:

	// ワープ先のオブジェクト名のプレフィックス（例: "WarpDest" と設定すると "WarpDest1"、"WarpDest2"... を検索する）
	C_PROPERTY()
		std::string warpDestinationBaseName = "WarpDest";

	// ワープ先の候補の最大数
	C_PROPERTY()
		int warpDestinationCount = 3;

	// ワープのクールダウン時間（秒）
	C_PROPERTY()
		float warpCooldownTime = 2.0f;

	C_PROPERTY()
		float warpEffectDuration = 1.0f; // ワープエフェクトの持続時間（秒）

	C_PROPERTY()
		float warpEffectWaitTime = 0.5f; // ワープエフェクト開始から実際のワープまでの待機時間（秒）

	float currentCooldown = 0.0f; // 現在のクールダウンタイマー

	// キャッシュ用変数
	std::weak_ptr<Rigidbody> cachedBallRigidbody;

	// 演出用
	bool isWarping = false;
	float effectTimer = 0.0f;
	Vector3 ballInitialScale;
	Vector3 warpDestPos;
	std::weak_ptr<Rigidbody> warpingBallRb;
	GameObject* warpDestObj = nullptr; // ワープ先オブジェクトへのポインタ
};