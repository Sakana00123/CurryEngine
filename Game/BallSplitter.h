#pragma once
#include "Gadget.h"
#include "Engine/Physics/CollisionEvent.h"
#include "Engine/Physics/Rigidbody.h"

class BallSplitter : public Gadget
{
	C_REFLECT(BallSplitter)
public:
	BallSplitter() = default;
	~BallSplitter() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnCollisionEnter(const CollisionInfo& collisionInfo) ;


	void OnPreviewEnter() override;
	void OnPreviewExit() override;
	// ガジェットが特定のアクションを実行するためのイベント（例: プレイヤーがガジェットを使用したとき）
	void OnAction() override;

	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときのイベント
	void OnBreak() override; // ガジェットが壊れたときのイベント

private:

	// ボールのプレハブパス
	C_PROPERTY()
	std::string ballPrefabPath = "Assets/Prefabs/Ball.prefab";

	C_PROPERTY()
		float splitCooldownTime = 1.0f; // 分裂のクールダウン時間（秒）

	float splitCooldown = 0.0f; // 現在のクールダウンタイマー


	Vector3 spawnPosCache; // ボールのスポーン位置をキャッシュする変数
	float relativeScaleCache = 1.0f; // ボールの相対スケールをキャッシュする変数
	int valueCache = 1; // ボールの価値をキャッシュする変数
	std::weak_ptr<Rigidbody> ballRbCache; // Rigidbodyのキャッシュ
};