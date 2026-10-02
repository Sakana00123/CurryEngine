#pragma once
#include "RhythmAnimState.h"
#include "StateTransition.h"

// リズムステートマシンの定義
/** @brief RhythmStateMachine を表すクラスです。 */
class RhythmStateMachine
{
public:

	// ステートマシンのリセット
	/**
	 * @brief 状態を初期値に戻します。
	 */
	void Reset(const std::string& initialState);

	// 更新処理
	/**
	 * @brief 状態を更新します。
	 */
	void Update();

	// ステートを取得
	/**
	 * @brief GetState に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	RhythmAnimState* GetState(const std::string& name);

public:

	// ステートのリスト
	std::vector<RhythmAnimState> states;
	// ステート遷移のリスト
	std::vector<StateTransition> transitions;
	// 現在のステート名
	std::string currentStateName;
};