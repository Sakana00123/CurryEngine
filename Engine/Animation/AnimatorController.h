#pragma once
#include "Engine/Core/Reflection/Meta.h"
#include "Engine/Resources/AnimationClip.h"
#include "Engine/Resources/AssetId.h"

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
	std::string parameterName;
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
	int clipIndex; // アニメーションクリップのインデックス
	float speed = 1.0f; // 再生速度
	bool loop = true; // ループ再生するかどうか
	Vector2 editorPosition; // エディタ上での位置（ノードの配置用）
};

class AnimatorController : public Resource
{
public:
	struct PlayingState
	{
		int clipIndex = -1;
		float time = 0.0f;
	};
	std::string name;

	std::vector<PlayingState> playingStates; // 複数のアニメーションを同時に再生する場合の状態を保持

	float transitionElapsedTime = 0.0f; // 遷移の経過時間
	float transitionDuration = 0.0f; // アニメーションの遷移時間

	int currentClipIndex = 0;
	float currentTime = 0.0f;
	float weight = 1.0f; // アニメーションの重み（ブレンド用）

	std::vector<std::string> animationNames; // アニメーションクリップの名前リスト
	std::vector<CurryEngine::Resources::AssetId> animationClipIds; // アニメーションクリップのIDリスト
	std::vector<AnimatorParameter> parameters; // アニメーションパラメータのリスト
	std::vector<AnimatorState> states; // アニメーションステートのリスト
	std::vector<AnimatorTransition> transitions; // アニメーション遷移のリスト
	int defaultStateIndex = 0; // デフォルトのアニメーションステートのインデックス

	std::vector<std::shared_ptr<AnimationClip>> animationClips; // アニメーションクリップのリスト
	std::vector<NodePose> currentPose; // 現在のノードポーズのリスト


	bool LoadFromFile(const std::string& path) override;

	bool SaveToFile(const std::filesystem::path& path) const;

	// アニメーションパラメータの型を取得する
	AnimatorParameter::Type GetParameterType(const std::string& name) const;

};

struct RuntimeAnimatorController
{
	struct PlayingState
	{
		int clipIndex = -1;
		float time = 0.0f;
		int stateIndex = -1; // AnimatorController::statesのインデックス
	};
	std::unordered_map<std::string, float> parameterValues; // Trigger含め全部float運用が楽
	std::vector<PlayingState> playing; // 複数のアニメーションを同時に再生する場合の状態を保持
	int currentStateIndex = -1; // AnimatorController::statesのインデックス
	float transitionElapsed = 0.0f;
	float transitionDuration = 0.0f;
	std::vector<NodePose> currentPose;

	void Initialize(const AnimatorController& controller);

	// アニメーションの再生を開始する
	void Play(const AnimatorController& controller, int stateIndex, float blendDuration = 0.0f);

	void SetFloat(const std::string& name, float value);
	void SetInt(const std::string& name, int value);
	void SetBool(const std::string& name, bool value);
	void SetTrigger(const std::string& name);

	const std::vector<NodePose>& GetPose() const;

	void Update(float deltaTime, const AnimatorController& controller);

	void BeginTransition(const AnimatorTransition& transition, const AnimatorController& controller);
	void ConsumeTrigger(const AnimatorController& controller, const std::vector<AnimatorCondition>& conditions);
};
