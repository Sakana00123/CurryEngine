#pragma once
#include "RhythmAnimState.h"
#include "StateTransition.h"

// リズムステートマシンの定義
class RhythmStateMachine
{
public:

	// ステートマシンのリセット
	void Reset(const std::string& initialState);

	// 更新処理
	void Update();

	// ステートを取得
	RhythmAnimState* GetState(const std::string& name);

public:

	// ステートのリスト
	std::vector<RhythmAnimState> states;
	// ステート遷移のリスト
	std::vector<StateTransition> transitions;
	// 現在のステート名
	std::string currentStateName;
};