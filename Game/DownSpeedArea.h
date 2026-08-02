#pragma once
#include "Gadget.h"
#include "Engine/Physics/CollisionEvent.h"
#include <unordered_set>

class DownSpeedArea : public Gadget
{
	C_REFLECT(DownSpeedArea)
public:
	DownSpeedArea() = default;
	~DownSpeedArea() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnTriggerEnter(const TriggerInfo& collisionInfo);
	void OnTriggerStay(const TriggerInfo& collisionInfo);

	void OnPreviewEnter() override; // ピンに近づけたときにプレビュー表示を開始するためのイベント。
	void OnPreviewExit() override; // ピンから離れたときにプレビュー表示を終了するためのイベント。

	void OnActivate() override; // ガジェットがアクティブ化されたときの処理
	void OnDeactivate() override; // ガジェットが非アクティブ化されたときの処理
	void OnBreak() override; // ガジェットが壊れたときの処理

	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理

	void ClearBallSet() override; // 管理しているボールのセットをクリアするための関数。必要に応じてオーバーライドして実装します。

private:



	C_PROPERTY()
		float enterDownSpeedRate = 0.5f; // トリガーに入ったときの減速率

	C_PROPERTY()
		float stayDownSpeedRate = 0.1f; // トリガー内

	C_PROPERTY()
		float extraGravityScale = 3.0f;      // Stay中に加える追加重力の倍率

	//// トリガーに入ったオブジェクトを管理するためのセット。これにより、同じオブジェクトが複数回減速されるのを防ぎます。
	//std::unordered_set<GameObject*> triggeredObjects;
};