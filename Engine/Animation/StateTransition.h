#pragma once

// ステート遷移条件の定義
enum class ConditionType : uint8_t
{
	None,       // 条件なし
	Trigger,    // トリガー
	Bool,		// ブール
	Int,	    // 整数
	Float       // 浮動小数点数
};

// ステート遷移条件
struct StateTransitionCondition
{
	ConditionType type = ConditionType::None;
	std::string parameterName;
	// 各種条件値
	bool boolValue = false;
	int intValue = 0;
	float floatValue = 0.0f;
	// 比較演算子（Int, Float用）
	enum class ComparisonOperator : uint8_t
	{
		Equal,
		NotEqual,
		Less,
		LessEqual,
		Greater,
		GreaterEqual
	};
	ComparisonOperator comparisonOperator = ComparisonOperator::Equal;
};

// ステート遷移の定義
struct StateTransition
{
	std::string fromState; // 遷移元ステート名
	std::string toState;   // 遷移先ステート名
	std::vector<StateTransitionCondition> conditions; // 遷移条件リスト
	float exitTime = 0.0f; // ステートの終了時間（0.0〜1.0、0.0の場合は無効）
	bool hasExitTime = false; // 終了時間を使用するか
	bool interruptible = true; // 割り込み可能か
};