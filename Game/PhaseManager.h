#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class PhaseManager : public Component
{
	C_REFLECT(PhaseManager)
public:
	PhaseManager() = default;
	~PhaseManager() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	//void DrawProperty() override; // エディタでプロパティを描画するためのオーバーライド関数

	//現在のフェーズを取得する関数
	int GetCurrentPhase() const { return currentPhase; }

	//フェーズを進める関数
	void AdvancePhase();

	void SetPhase(int phase); // フェーズを直接設定する関数

private:

	// 現在のフェーズを表す変数(0:プレイ中、1:ショップ、2:配置, 3:ゲームオーバーなどの追加フェーズがある場合はさらに増やす)
	C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly, CurryEngine::PropertyAttributes::NonSerialized)
	int currentPhase = -1;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("ResultManager"))
	ObjectId resultManagerReference; // 例: ResultManager コンポーネントへの参照

public:

	enum Phase
	{
		Playing = 0,
		Shop = 1,
		Placement = 2,
		Result = 3,
	};

};
