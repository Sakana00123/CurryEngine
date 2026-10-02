#pragma once
#include "Engine/Core/Reflection/Meta.h"
#include "Engine/Resources/AnimationClip.h"
#include "Engine/Resources/AssetId.h"
#include <unordered_map>
#include <Engine\Resources\AnimationEvent.h>

#undef ENABLE_ANIMATOR_PARAMETER_BINDING

#ifdef ENABLE_ANIMATOR_PARAMETER_BINDING
/** @brief AnimatorParameterBinding を表す構造体です。 */
struct AnimatorParameterBinding
{
	ObjectId sourceComponentId; // ObjectReference属性で選択（targetModelRendererIdと同じパターン）
	std::string propertyName;   // リフレクション情報からコンボボックスで選択
};
#endif // ENABLE_ANIMATOR_PARAMETER_BINDING

/** @brief AnimatorParameter を表す構造体です。 */
struct AnimatorParameter
{
	/** @brief Type を表す列挙型です。 */
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
#ifdef ENABLE_ANIMATOR_PARAMETER_BINDING
	std::optional<AnimatorParameterBinding> binding; // パラメータのバインディング情報(未設定ならスクリプト側で制御)  
#endif // ENABLE_ANIMATOR_PARAMETER_BINDING

};

/** @brief AnimatorCondition を表す構造体です。 */
struct AnimatorCondition
{
	/** @brief Comparison を表す列挙型です。 */
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

/** @brief AnimatorTransition を表す構造体です。 */
struct AnimatorTransition
{
	int fromStateIndex = -1;
	int toStateIndex = -1;
	float blendDuration = 0.25f; // 遷移のブレンド時間
	std::vector<AnimatorCondition> conditions;
	bool hasExitTime = false;
	float exitTime = 1.0f; // 正規化時間(0.0f〜1.0f)での遷移開始タイミング
};

/** @brief BlendTreeType を表す列挙型です。 */
enum class BlendTreeType
{
	None,                // 通常の単一クリップステート
	Simple1D,            // 1軸のみでブレンド（移動速度など）
	FreeformCartesian2D, // 2軸を自由配置でブレンド（前後+左右ストレイフなど）
};

/** @brief BlendTreeEntry を表す構造体です。 */
struct BlendTreeEntry
{
	CurryEngine::Resources::AssetId clipId;
	float threshold = 0.0f; // Simple1D用
	Vector2 position;       // FreeformCartesian2D用
};

/** @brief AnimatorState を表す構造体です。 */
struct AnimatorState
{
	std::string name;
	CurryEngine::Resources::AssetId clipId; // アニメーションクリップのID
	CurryEngine::Resources::AssetId timelineId; // AnimationTimelineのID
	float speed = 1.0f; // 再生速度
	Vector2 editorPosition; // エディタ上での位置（ノードの配置用）
	bool loop = true; // ループ再生するかどうか
	bool rootMotion = false; // ルートモーションを使用するかどうか
	int rootNodeIndex = -1; // ルートモーションを適用するノードのインデックス（-1ならルートノード）

	BlendTreeType blendType = BlendTreeType::None;
	int blendParamXIndex = -1;    // Simple1Dはこれのみ使用 / FreeformCartesian2DはX軸
	int blendParamYIndex = -1;    // FreeformCartesian2Dの場合のみ使用
	std::vector<BlendTreeEntry> blendEntries; // blendType != None のときのみ使用
	float blendSmoothTime = 0.0f;

	bool IsBlendTree() const { return blendType != BlendTreeType::None; }
};

/** @brief AnimatorController を表すクラスです。 */
class AnimatorController : public Resource
{
public:
	std::string name;
	std::vector<AnimatorParameter> parameters; // アニメーションパラメータのリスト
	std::vector<AnimatorState> states; // アニメーションステートのリスト
	std::vector<AnimatorTransition> transitions; // アニメーション遷移のリスト
	CurryEngine::Resources::AssetId modelAssetId; // このAnimatorControllerが対象とするメッシュのアセットID(現状はエディタでの表示用のみ使用)
	int defaultStateIndex = 0; // デフォルトのアニメーションステートのインデックス

	std::unordered_map<CurryEngine::Resources::AssetId, std::shared_ptr<AnimationClip>> animationClips; // アニメーションクリップのリスト
	std::unordered_map<CurryEngine::Resources::AssetId, std::shared_ptr<CurryEngine::Resources::AnimationTimeline>> animationTimelines; // AnimationTimelineのリスト

	/**
	 * @brief LoadFromFile に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	bool LoadFromFile(const std::string& path) override;

	/**
	 * @brief SaveToFile の処理を行います。
	 */
	bool SaveToFile(const std::filesystem::path& path) const;

	// アニメーションパラメータの型を取得する
	/**
	 * @brief GetParameterType に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	AnimatorParameter::Type GetParameterType(const std::string& name) const;

	// アニメーションステートのインデックスを名前から取得する
	/**
	 * @brief GetStateIndexByName に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	int GetStateIndexByName(const std::string& name) const;
};

/** @brief RuntimeAnimatorController を表す構造体です。 */
struct RuntimeAnimatorController
{
	/** @brief BlendedClipWeight を表す構造体です。 */
	struct BlendedClipWeight
	{
		CurryEngine::Resources::AssetId clipId;
		float weight = 1.0f; // クリップの重み（0.0f〜1.0f）
	};
	/** @brief PlayingState を表す構造体です。 */
	struct PlayingState
	{
		float time = 0.0f;
		int stateIndex = -1; // AnimatorController::statesのインデックス
		std::unordered_map<CurryEngine::Resources::AssetId, float> blendWeights;
		float lastNormalizedTime = 0.0f;
	};
	std::unordered_map<std::string, float> parameterValues; // Trigger含め全部float運用が楽
	std::vector<PlayingState> playing; // 複数のアニメーションを同時に再生する場合の状態を保持
	int currentStateIndex = -1; // AnimatorController::statesのインデックス
	float transitionElapsed = 0.0f;
	float transitionDuration = 0.0f;
	std::vector<NodePose> currentPose;
	std::vector<NodePose> bindPose; // Initialize時の初期ポーズを保持（ルートモーション適用時の基準値）

	XMFLOAT3 rootMotionDeltaPosition{};   // このフレーム分の移動量（呼び出し側が毎フレームConsumeする）
	XMFLOAT4 rootMotionDeltaRotation{ 0,0,0,1 };
	float rootMotionLastNormalizedTime = 0.0f; // 直前フレームの正規化時間（ルートモーション差分計算用）
	ObjectId targetRendererId; // エディタ上で使用する、RuntimeAnimatorControllerが適用されるレンダラーのObjectId
	ObjectId targetAnimatorId;  // エディタ上で使用する、RuntimeAnimatorControllerが適用されるAnimatorのObjectId

	void Initialize(const AnimatorController& controller, std::vector<NodePose> initialPose = {});

	// アニメーションパラメータの初期値を同期する
	/**
	 * @brief SyncParameters の処理を行います。
	 */
	void SyncParameters(const AnimatorController& controller);

	// アニメーションの再生を開始する
	/**
	 * @brief Play の処理を行います。
	 */
	void Play(const AnimatorController& controller, int stateIndex, float blendDuration = 0.0f);

	/**
	 * @brief Float を設定します。
	 */
	void SetFloat(const std::string& name, float value);
	/**
	 * @brief Int を設定します。
	 */
	void SetInt(const std::string& name, int value);
	/**
	 * @brief Bool を設定します。
	 */
	void SetBool(const std::string& name, bool value);
	/**
	 * @brief Trigger を設定します。
	 */
	void SetTrigger(const std::string& name);

	// 現在のノードポーズを取得する
	/**
	 * @brief GetPose に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	const std::vector<NodePose>& GetPose() const;

	// 初期ポーズ（BindPose）を取得する
	/**
	 * @brief GetBindPose に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	//const std::vector<NodePose>& GetBindPose() const;

	// 条件をすべて満たしているかを判定する
	/**
	 * @brief AllConditionsMet の処理を行います。
	 */
	bool AllConditionsMet(const AnimatorController& controller, const std::vector<AnimatorCondition>& conditions) const;

	// ブレンドツリーの重みを計算する
	/**
	 * @brief ComputeBlendWeights の処理を行います。
	 */
	std::vector<BlendedClipWeight> ComputeBlendWeights(const AnimatorController& controller, const PlayingState& state) const;

	// アニメーションパラメータの値を取得する
	/**
	 * @brief GetParameterValue に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	float GetParameterValue(const AnimatorController& controller, int parameterIndex) const;

	/**
	 * @brief 状態を更新します。
	 */
	void Update(float deltaTime, const AnimatorController& controller, const XMFLOAT4X4& worldTransform);

	/**
	 * @brief BeginTransition の処理を行います。
	 */
	void BeginTransition(const AnimatorTransition& transition, const AnimatorController& controller);
	/**
	 * @brief ConsumeTrigger の処理を行います。
	 */
	void ConsumeTrigger(const AnimatorController& controller, const std::vector<AnimatorCondition>& conditions);
	// 蓄積したルートモーション差分を取得してリセットする
	/**
	 * @brief ConsumeRootMotion の処理を行います。
	 */
	void ConsumeRootMotion(const AnimatorController& controller, XMFLOAT3& outDeltaPosition, XMFLOAT4& outDeltaRotation);

	// 再生中のアニメーションの平均再生時間を計算する
	/**
	 * @brief ResolveAverageDuration の処理を行います。
	 */
	float ResolveAverageDuration(const AnimatorController& controller, const std::vector<BlendedClipWeight>& weights) const;
	// 再生中のアニメーションの平均ループフラグを計算する
	void CompositePose(const AnimatorController& controller, const std::vector<BlendedClipWeight>& frontWeights, float frontTime, float frontScale,
		const std::vector<BlendedClipWeight>& nextWeights, float nextTime, float nextScale);
	// ブレンドウェイトをスムーズに補間する
	std::vector<BlendedClipWeight> SmoothBlendWeights(
		std::unordered_map<CurryEngine::Resources::AssetId, float>& current,
		const std::vector<BlendedClipWeight>& target,
		float smoothTime, float deltaTime) const;


	std::vector<CurryEngine::Resources::FiredAnimationEvent> firedEvents; // このフレームで発火したイベント（Consume想定）

	// 発火イベントを取得してクリアする（ConsumeRootMotionと同じパターン）
	/**
	 * @brief ConsumeFiredEvents の処理を行います。
	 */
	std::vector<CurryEngine::Resources::FiredAnimationEvent> ConsumeFiredEvents();

private:
	// 指定ステートの正規化時間の進みからイベント発火を判定する
	/**
	 * @brief DispatchStateEvents の処理を行います。
	 */
	void DispatchStateEvents(const AnimatorController& controller, PlayingState& state, float normalizedTime);
};
