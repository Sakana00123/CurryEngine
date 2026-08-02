#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class RoundManager : public Component
{
	C_REFLECT(RoundManager)
public:
	RoundManager() = default;
	~RoundManager() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	// ラウンド数を取得する関数
	int GetCurrentRound() const { return currentRound; }

	// 最大ラウンド数を取得する関数
	int GetMaxRounds() const { return maxRounds; }

	// 次のラウンドに進む関数
	void NextRound();

	// ラウンド開始の関数
	void StartRound();

	// ラウンド終了の関数
	void EndRound();

	// 終了処理を行う関数(ゲームオーバーやクリアなどの処理をここに実装する)
	void EndGame();

	// ラウンドクリアして、ショップフェーズに移行する直前の処理を行う関数
	void OnRoundClear();

	// ラウンド完了時の処理を行う関数(ショット数が0になってて、ボールが無くなり、かつ目標金額を達成している場合に呼び出される)
	void OnRoundComplete();

	// ラウンド数をリセットする関数
	void ResetRounds() { currentRound = 1; }

	// ラウンド数を表示するテキストを更新する関数
	void UpdateRoundText();

	// ボールの個数を更新する関数
	void UpdateBallCountText();

	// ボールを再セットする関数
	void ResetBall();

	// 目標金額を表示するテキストを更新する関数
	void UpdateTargetValueText(int round);

	// 目標金額を計算する関数
	int CalculateTargetValue(int initialValue, float scalingFactor, int round);

	// 無限モードを開始する関数
	void StartEndlessMode();

private:

	// ここにコンポーネントのメンバ変数を定義します。必要に応じて C_PROPERTY() マクロを使用してシリアライズ可能なプロパティを定義できます。
	C_PROPERTY()
	int maxRounds = 4; // 最大ラウンド数

	C_PROPERTY()
	int currentRound = 1; // 現在のラウンド数

	
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId roundTextReference; // 例: text コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
		ObjectId canvasReference;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("ObtrusiveGadgetItemList"))
	ObjectId itemListReference; // 例: ObtrusiveGadgetItemList コンポーネントへの参照

	C_PROPERTY()
	std::string ballPrefabPath = "Assets/Prefabs/Ball.prefab"; // ボールのプレハブのファイルパス

	C_PROPERTY()
		Vector3 ballSpawnPosition = Vector3(0, 1, 0); // ボールのスポーン位置

	C_PROPERTY()
	int ballSpawnCount = 1; // スポーンするボールの個数

	C_PROPERTY()
	int initialBallCount = 2; // 初期のボールの個数

	/** @brief ボールの残りの個数 **/
	C_PROPERTY()
	int ballCount = 0;

	/** @brief ボールの個数を表示するテキストへの参照 **/
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId ballCountReference;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("PassiveSkillContainer"))
	ObjectId passiveSkillContainerReference; // パッシブスキルコンテナへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("RerollItem"))
	ObjectId rerollItemReference; // リロールアイテムへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("ComboText"))
	ObjectId comboTextReference; // コンボテキストへの参照

	/** @brief ボールがゾーンから出た後、初期位置に戻るまでの時間（秒） **/
	C_PROPERTY()
	float resetDelay = 2.0f;

	/* @brief 目標金額の初期値。ラウンドごとにこの値を基準にして目標金額が増加していくことを想定しています。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::ToolTip("Initial target value for goal achievement."))
	int initialTargetValue = 10; // 目標金額の初期値

	/* @brief 難易度上昇係数。ラウンドが進むごとに目標金額がどれだけ増加するかを決定します。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::ToolTip("Difficulty scaling factor for target value. The target value will increase by this factor each round."))
	float difficultyScalingFactor = 1.5f; // 難易度上昇係数

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId endlessModeButtonReference; // 無限モード開始ボタンへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("UIEasing"))
	ObjectId uiEasingReference; // UIイージングへの参照


	int initialBallSpawnCount = 0; // 初期のスポーンするボールの個数を保存する変数

	/** @brief ボールを初期位置に戻すまでの経過時間 **/
	float resetTimer = 0.0f;

	/** @brief リセット待機中か **/
	bool isWaitingForReset = false;

};