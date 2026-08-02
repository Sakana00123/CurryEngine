#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class TutorialSystem : public Component
{
	C_REFLECT(TutorialSystem)
public:
	TutorialSystem() = default;
	~TutorialSystem() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnDestroy() override; // コンポーネントが破棄されるときの処理をオーバーライドして実装します。

	static void SetTutorialMode(bool enabled); // チュートリアルモードを設定する関数

	static bool IsTutorialMode() { return isTutorialMode; } // チュートリアルモードが有効かどうかを取得する関数

	static TutorialSystem* GetInstance(); // TutorialSystem のインスタンスを取得する関数

	void AdvanceTutorialStep(); // チュートリアルのステップを進める関数

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
		ObjectId nameplateReference; // ネームプレートへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("RoundManager"))
	ObjectId roundManagerReference; // RoundManager コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("PhaseManager"))
	ObjectId phaseManagerReference; // PhaseManager コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId welcomeGuideReference; // ウェルカムガイドへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId shotGuideReference; // ショットガイドへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId oneRoundTwoShotsGuideReference; // 1ラウンド2ショットガイドへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId shopGuideReference; // ショップガイドへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId purchaseAllGuideReference; // 全部購入ガイドへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId enterPlacementGuideReference; // 配置に入るガイドへの参照
	
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId placementGuideReference; // 配置ガイドへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId nextRoundGuideReference; // 次のラウンドに進むガイドへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId obtrusiveGadgetGuideReference; // お邪魔ガジェットガイドへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId objectiveGuideReference; // 目的ガイドへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("SceneTransitionButton"))
	ObjectId sceneTransitionButtonReference; // シーン遷移ボタンへの参照


	C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly, CurryEngine::PropertyAttributes::NonSerialized)
	int currentStep = -1; // 現在のチュートリアルステップを追跡する変数

	enum TutorialStep
	{
		Welcome, // チュートリアル開始のウェルカムメッセージ
		Shot, // ショットの基本操作を説明するステップ
		WaitForFirstShotEnd, // 最初のショットを待つステップ
		OneRoundTwoShots, // 1ラウンドで2回ショットを打つことを説明するステップ
		WaitForShopEnter, // ショップに入ることを待つステップ
		Shop, // ショップの基本操作を説明するステップ
		PurchaseAll, // 全部購入させるステップ
		PlacementEnter, // 配置に入ることを説明するステップ
		WaitForPlacementEnter, // 配置に入ることを待つステップ
		Placement, // 配置の基本操作を説明するステップ
		WaitForPlacementEnd, // 配置を完了することを待つステップ
		NextRound, // 次のラウンドに進ませるステップ
		ObtrusiveGadget, // お邪魔ガジェットの説明をするステップ
		Objective, // ゲームの目的や勝利条件を説明するステップ
		Complete, // チュートリアル完了のメッセージ
	};

	// 現在のチュートリアルステップを取得する関数
	TutorialStep GetCurrentTutorialStep() const { return static_cast<TutorialStep>(currentStep); } 

private:
	static inline bool isTutorialMode; // チュートリアルモードが有効かどうかを示す静的変数

};
