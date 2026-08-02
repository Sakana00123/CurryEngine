#pragma once
#include "Gadget.h"
#include "Engine/Physics/CollisionEvent.h"

class Rigidbody;
class ParticleComponent;

struct ModifiedBall
{
	std::weak_ptr<Rigidbody> rb;
	float timer;
};

class GravityChange : public Gadget
{
	C_REFLECT(GravityChange)
public:
	GravityChange() = default;
	~GravityChange() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnTriggerEnter(const TriggerInfo& triggerInfo);

	void OnPreviewEnter() override; // ピンに近づけたときのプレビュー表示開始の処理
	void OnPreviewExit() override; // ピンから離れたときのプレビュー表示終了の処理
	
	void OnActivate() override; // ガジェットがアクティブ化されたときの処理
	void OnBreak() override; // ガジェットが壊れたときの処理
	void OnDeactivate() override; // ガジェットが非アクティブ化されたときの処理
	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理
	
	void ClearBallSet() override; // ガジェットが管理しているボールのセットをクリアするための関数

	void ResetGravity(); // 重力を元に戻す関数

	void SetDisabled(bool disabled) override; // ガジェットが無効化されたときの処理。必要に応じてオーバーライドして実装します。
private:

	C_PROPERTY(CurryEngine::PropertyAttributes::NonSerialized)
	float gravityChangeDuration = 0.3f; // 重力変化の持続時間

	float gravityChangeTimer = 0.0f; // 重力変化のタイマー
	Vector3 originalGravity; // 元の重力を保存する変数
	std::vector<ModifiedBall> modifiedBalls; // 重力が変更されたボールのリスト

	ParticleComponent* particleEffect = nullptr; // エフェクト用のパーティクルコンポーネント
};