#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class BallObserver : public Component
{
	C_REFLECT(BallObserver)
public:
	BallObserver() = default;
	~BallObserver() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	int GetBallCount() const { return ballCount; } // 衝突回数を取得する関数

	//void DrawProperty() override; // エディタでプロパティを描画するためのオーバーライド関数
private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Gate"))
	ObjectId gateReference; // Gate コンポーネントへの参照
	
	int ballCount = 0; // 衝突回数のカウンタ


};
