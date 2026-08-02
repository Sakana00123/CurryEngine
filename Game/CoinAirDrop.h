#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Physics/CollisionEvent.h"
#include "Gadget.h"
#include "Coin.h"

class CoinAirDrop : public Gadget
{
	C_REFLECT(CoinAirDrop)
public:
	CoinAirDrop() = default;
	~CoinAirDrop() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnCollisionEnter(const CollisionInfo& collisionInfo);

	void OnPreviewEnter() override; // ピンに近づけたときの処理
	void OnPreviewExit() override; // ピンから離れたときの処理


	void OnAction() override; // ラウンド終了時の処理
	void OnDeactivate() override; // ガジェットが非アクティブ化されたときの処理
	void OnActivate() override; // ガジェットがアクティブ化されたときの処理
	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理
private:

	// コインのプレハブパス
	C_PROPERTY()
		std::string coinPrefabPath = "./Assets/Prefabs/GadgetModels/Coin.prefab";

	C_PROPERTY()
		int coinCount = 3; // ドロップするコインの数

	int collisionCount = 5; // 衝突回数を追跡する変数

	int currentCollisionCount = 0; // 現在の衝突回数を追跡する変数

};