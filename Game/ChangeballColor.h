#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Physics/CollisionEvent.h"
#include "Engine/Physics/Rigidbody.h"
#include "Ball.h"
#include "Gadget.h"

class ChangeballColor : public Gadget
{
	C_REFLECT(ChangeballColor)
public:
	ChangeballColor() = default;
	~ChangeballColor() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnCollisionEnter(const CollisionInfo& info);

	void OnAction() override; // ラウンド終了時の処理

	void OnDeactivate() override; // ガジェットが非アクティブ化されたときの処理
	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理

	void OnPreviewEnter() override; // ピンに近づけたときの処理
	void OnPreviewExit() override; // ピンから離れたときの処理

private:

	Vector3 originalColor = { 1.0f, 1.0f, 1.0f }; // ボールの元の色を保存するための変数

	Vector3 newColor = { 1.0f, 0.0f, 0.0f }; // ボールの色を変更するための新しい色

	float effectTimer = 0.0f;
	float effectInterval = 0.05f; // 再生間隔（秒）

	std::weak_ptr<Rigidbody> cachedBallRigidbody; // Rigidbodyのキャッシュ
	Vector3 cachedCollisionNormal; // 衝突法線のキャッシュ
};