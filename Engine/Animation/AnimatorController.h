#pragma once
#include "Engine/Core/Reflection/Meta.h"
#include "Engine/Resources/AnimationClip.h"
#include "Engine/Resources/AssetId.h"
#include <unordered_map>

struct AnimatorParameterBinding
{
	ObjectId sourceComponentId; // ObjectReference属性で選択（targetModelRendererIdと同じパターン）
	std::string propertyName;   // リフレクション情報からコンボボックスで選択
};

struct AnimatorParameter
{
	enum class Type
	{
		Float,
		Int,
		Bool,
		Trigger
	};
	std::string name;
	Type type;
	float defaultValue;
	std::optional<AnimatorParameterBinding> binding; // パラメータのバインディング情報(未設定ならスクリプト側で制御)
};

struct AnimatorCondition
{
	enum class Comparison
	{
		Less,
		LessEqual,
		Greater,
		GreaterEqual,
		Equal,
		NotEqual
	};
	int parameterIndex = -1; // AnimatorParameterのインデックス
	Comparison comparison;
	float value;
};

struct AnimatorTransition
{
	int fromStateIndex = -1;
	int toStateIndex = -1;
	float blendDuration = 0.25f; // 遷移のブレンド時間
	std::vector<AnimatorCondition> conditions;
	bool hasExitTime = false;
	float exitTime = 1.0f; // 正規化時間(0.0f〜1.0f)での遷移開始タイミング
};

struct AnimatorState
{
	std::string name;
	CurryEngine::Resources::AssetId clipId; // アニメーションクリップのID
	float speed = 1.0f; // 再生速度
	bool loop = true; // ループ再生するかどうか
	Vector2 editorPosition; // エディタ上での位置（ノードの配置用）
};

class AnimatorController : public Resource
{
public:
	std::string name;
	std::vector<AnimatorParameter> parameters; // アニメーションパラメータのリスト
	std::vector<AnimatorState> states; // アニメーションステートのリスト
	std::vector<AnimatorTransition> transitions; // アニメーション遷移のリスト
	int defaultStateIndex = 0; // デフォルトのアニメーションステートのインデックス

	std::unordered_map< CurryEngine::Resources::AssetId, std::shared_ptr<AnimationClip>> animationClips; // アニメーションクリップのリスト

	bool LoadFromFile(const std::string& path) override;

	bool SaveToFile(const std::filesystem::path& path) const;

	// アニメーションパラメータの型を取得する
	AnimatorParameter::Type GetParameterType(const std::string& name) const;

};

struct RuntimeAnimatorController
{
	struct PlayingState
	{
		CurryEngine::Resources::AssetId clipId; // アニメーションクリップのID
		float time = 0.0f;
		int stateIndex = -1; // AnimatorController::statesのインデックス
	};
	std::unordered_map<std::string, float> parameterValues; // Trigger含め全部float運用が楽
	std::vector<PlayingState> playing; // 複数のアニメーションを同時に再生する場合の状態を保持
	int currentStateIndex = -1; // AnimatorController::statesのインデックス
	float transitionElapsed = 0.0f;
	float transitionDuration = 0.0f;
	std::vector<NodePose> currentPose;

	void Initialize(const AnimatorController& controller, std::vector<NodePose> initialPose = {});

	// アニメーションの再生を開始する
	void Play(const AnimatorController& controller, int stateIndex, float blendDuration = 0.0f);

	void SetFloat(const std::string& name, float value);
	void SetInt(const std::string& name, int value);
	void SetBool(const std::string& name, bool value);
	void SetTrigger(const std::string& name);

	const std::vector<NodePose>& GetPose() const;

	// 条件をすべて満たしているかを判定する
	bool AllConditionsMet(const AnimatorController& controller, const std::vector<AnimatorCondition>& conditions) const;

	// アニメーションパラメータの値を取得する
	float GetParameterValue(const AnimatorController& controller, int parameterIndex) const;

	void Update(float deltaTime, const AnimatorController& controller);

	void BeginTransition(const AnimatorTransition& transition, const AnimatorController& controller);
	void ConsumeTrigger(const AnimatorController& controller, const std::vector<AnimatorCondition>& conditions);
};
