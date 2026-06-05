#include "pch.h"
#include "RhythmStateMachine.h"

void RhythmStateMachine::Reset(const std::string& initialState)
{
	currentStateName = initialState;
}

void RhythmStateMachine::Update()
{
	// 現在のステートを取得
	RhythmAnimState* currentState = GetState(currentStateName);
	if (!currentState)
	{
		return;
	}
	// 遷移条件をチェック
	for (const auto& transition : transitions)
	{
		if (transition.fromState == currentStateName)
		{
			// 条件を満たすかチェック（ここでは簡略化のため常にtrueとする）
			bool conditionsMet = true;
			if (conditionsMet)
			{
				// ステート遷移
				currentStateName = transition.toState;
				break;
			}
		}
	}
}

RhythmAnimState* RhythmStateMachine::GetState(const std::string& name)
{
	for (auto& state : states)
	{
		if (state.name == name)
		{
			return &state;
		}
	}
	return nullptr;
}