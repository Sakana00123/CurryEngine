#include "pch.h"
#include "AnimatorController.h"
#include "Engine/Resources/AssetDatabase.h"
#include <fstream>

// アニメーションの遷移中に使用するインデックス
#define INDEX_FRONT 0
// 遷移先のアニメーションのインデックス
#define INDEX_NEXT 1
// ブレンドして遷移中である場合のサイズ
#define TRANSITION_SIZE 2


bool AnimatorController::LoadFromFile(const std::string& path)
{
	// ファイルからアニメーションクリップのIDリストを読み込む
	// ここでは仮にJSON形式で保存されていると仮定する
	std::ifstream file(path);
	if (!file.is_open())
	{
		LOG_ERROR(u8"[AnimatorController] ファイルの読み込みに失敗しました: " + std::u8string(path.begin(), path.end()));
		return false;
	}
	nlohmann::json jsonData;
	file >> jsonData;
	if (jsonData.contains("animationClipIds"))
	{
		for (const auto& clipIdStr : jsonData["animationClipIds"])
		{
			CurryEngine::Resources::AssetId clipId(clipIdStr.get<std::string>());
			animationClipIds.push_back(clipId);
			// アセットデータベースからアニメーションクリップをロード
			auto clip = CurryEngine::Resources::AssetDatabase::LoadAsset<AnimationClip>(clipId);
			if (clip)
			{
				animationClips.push_back(clip);
			}
			else
			{
				LOG_ERROR(u8"[AnimatorController] アニメーションクリップの読み込みに失敗しました: " + std::u8string(clipId.ToString().begin(), clipId.ToString().end()));
			}
		}
	}
	return true;
}

bool AnimatorController::SaveToFile(const std::filesystem::path& path) const
{
	// アニメーションクリップのIDリストをJSON形式で保存する
	nlohmann::json jsonData;
	jsonData["animationClipIds"] = nlohmann::json::array();
	for (const auto& clipId : animationClipIds)
	{
		jsonData["animationClipIds"].push_back(clipId.ToString());
	}
	std::ofstream file(path);
	if (!file.is_open())
	{
		LOG_ERROR(u8"[AnimatorController] ファイルの保存に失敗しました: " + std::u8string(path.u8string().begin(), path.u8string().end()));
		return false;
	}
	file << jsonData.dump(4); // インデント付きで保存
	return true;
}

AnimatorParameter::Type AnimatorController::GetParameterType(const std::string& name) const
{
	for (const auto& param : parameters)
	{
		if (param.name == name)
		{
			return param.type;
		}
	}
	return AnimatorParameter::Type::Float; // デフォルトでFloatを返す（存在しない場合の扱い）
}

namespace
{
	bool AllConditionsMet(const std::vector<AnimatorCondition>& conditions, const std::unordered_map<std::string, float>& parameterValues)
	{
		for (const auto& condition : conditions)
		{
			auto it = parameterValues.find(condition.parameterName);
			if (it == parameterValues.end())
			{
				return false; // パラメータが見つからない場合は条件を満たさない
			}
			float paramValue = it->second;
			switch (condition.comparison)
			{
			case AnimatorCondition::Comparison::Equal:
				if (paramValue != condition.value) return false;
				break;
			case AnimatorCondition::Comparison::NotEqual:
				if (paramValue == condition.value) return false;
				break;
			case AnimatorCondition::Comparison::Greater:
				if (paramValue <= condition.value) return false;
				break;
			case AnimatorCondition::Comparison::Less:
				if (paramValue >= condition.value) return false;
				break;
			default:
				return false; // 未知の比較タイプ
			}
		}
		return true; // すべての条件を満たす
	}
}

void RuntimeAnimatorController::Initialize(const AnimatorController& controller)
{
	// パラメータの初期化
	for (const auto& param : controller.parameters)
	{
		parameterValues[param.name] = param.defaultValue;
	}

	// デフォルトステートの再生
	if (!controller.states.empty())
	{
		Play(controller, controller.defaultStateIndex, 0.0f);
	}
}

void RuntimeAnimatorController::Play(const AnimatorController& controller, int stateIndex, float blendDuration)
{
	if (stateIndex < 0 || stateIndex >= controller.states.size())
	{
		std::string errorMsg = "[RuntimeAnimatorController] 無効なステートインデックス: " + std::to_string(stateIndex);
		std::u8string u8ErrorMsg(errorMsg.begin(), errorMsg.end());
		LOG_ERROR(u8ErrorMsg);
		return;
	}
	const auto& state = controller.states[stateIndex];
	if (state.clipIndex < 0 || state.clipIndex >= controller.animationClips.size())
	{
		std::string errorMsg = "[RuntimeAnimatorController] 無効なクリップインデックス: " + std::to_string(state.clipIndex);
		std::u8string u8ErrorMsg(errorMsg.begin(), errorMsg.end());
		LOG_ERROR(u8ErrorMsg);
		return;
	}
	if (blendDuration <= 0.0f || playing.empty())
	{
		playing = { {state.clipIndex, 0.0f} };
	}
	else
	{
		playing.push_back({ state.clipIndex, 0.0f, stateIndex });
		transitionElapsed = 0.0f;
		transitionDuration = blendDuration;
	}
	currentStateIndex = stateIndex;
}

void RuntimeAnimatorController::SetFloat(const std::string& name, float value)
{
	parameterValues[name] = value;
}

void RuntimeAnimatorController::SetInt(const std::string& name, int value)
{
	parameterValues[name] = static_cast<float>(value);
}

void RuntimeAnimatorController::SetBool(const std::string& name, bool value)
{
	parameterValues[name] = value ? 1.0f : 0.0f;
}

void RuntimeAnimatorController::SetTrigger(const std::string& name)
{
	parameterValues[name] = 1.0f; // Triggerは1.0fで表現

}

const std::vector<NodePose>& RuntimeAnimatorController::GetPose() const
{
	// 現在のノードポーズを返す
	return currentPose;
}

void RuntimeAnimatorController::Update(float deltaTime, const AnimatorController& controller)
{
	// アニメーションの更新処理
	if (playing.empty()) return;
	if (currentStateIndex < 0 || currentStateIndex >= controller.states.size()) return;

	auto& currentState = controller.states[currentStateIndex];

	auto& state = playing[INDEX_FRONT];
	state.time += deltaTime * currentState.speed; // 再生速度を考慮して時間を進める

	if (state.clipIndex < 0 || state.clipIndex >= controller.animationClips.size()) return;

	auto& clip = controller.animationClips[state.clipIndex];
	if (!clip) return;
	
	float normalizedTime = currentState.loop ? fmod(state.time / clip->duration, 1.0f) : (std::min)(state.time / clip->duration, 1.0f);

	// アニメーションをサンプリングして現在のポーズを更新(weightは1.0fで固定)
	clip->Sample(normalizedTime * clip->duration, currentPose, 1.0f);

	// 遷移中でなければ、AnyStateの遷移条件 -> 現在のStateの遷移条件の順で条件をチェック
	if (transitionDuration <= 0.0f)
	{
		for (const auto& transition : controller.transitions)
		{
			if (transition.fromStateIndex != -1 && transition.fromStateIndex != currentStateIndex) continue; // 遷移元が現在のステートでない場合はスキップ
			if (transition.hasExitTime && normalizedTime < transition.exitTime) continue; // ExitTimeが設定されていて、まだExitTimeに達していない場合はスキップ
			if (!AllConditionsMet(transition.conditions, parameterValues)) continue; // 条件を満たしていない場合はスキップ
			// 遷移条件を満たした場合、遷移を開始
			BeginTransition(transition, controller);

			// Triggerを消費する
			ConsumeTrigger(controller, transition.conditions);

			break; // 最初に条件を満たした遷移だけを処理する
		}
	}
	// 遷移中の場合、遷移の経過時間を更新
	else if (playing.size() == TRANSITION_SIZE)
	{
		auto& nextPlayingState = playing[INDEX_NEXT];
		if (nextPlayingState.stateIndex < 0 || nextPlayingState.stateIndex >= controller.states.size()) return;
		auto& nextState = controller.states[nextPlayingState.stateIndex];
		nextPlayingState.time += deltaTime * nextState.speed; // 遷移先のステートの再生速度を考慮して時間を進める
		transitionElapsed += deltaTime;

		// 遷移の進行度を計算
		float t = std::clamp(transitionElapsed / transitionDuration, 0.0f, 1.0f);
		// 遷移中のアニメーションをサンプリングして現在のポーズを更新
		if (nextPlayingState.clipIndex >= 0 && nextPlayingState.clipIndex < controller.animationClips.size())
		{
			auto& nextClip = controller.animationClips[nextPlayingState.clipIndex];
			if (nextClip)
			{
				float nextNormalizedTime = nextState.loop ? fmod(nextPlayingState.time / nextClip->duration, 1.0f) : (std::min)(nextPlayingState.time / nextClip->duration, 1.0f);
				// 遷移先のアニメーションをサンプリングして現在のポーズを更新(weightはtでブレンド)
				nextClip->Sample(nextNormalizedTime * nextClip->duration, currentPose, t);
			}
		}

		if (transitionElapsed >= transitionDuration)
		{
			// 遷移が完了したら、最初のアニメーションを削除
			if (playing.size() > 1)
			{
				playing.erase(playing.begin());
			}
			transitionElapsed = 0.0f;
			transitionDuration = 0.0f;
			currentStateIndex = playing[INDEX_FRONT].stateIndex; // 遷移後のステートに更新
		}
	}
}

void RuntimeAnimatorController::BeginTransition(const AnimatorTransition& transition, const AnimatorController& controller)
{
	// 遷移の開始処理
	if (transition.toStateIndex >= 0 && transition.toStateIndex < controller.states.size())
	{
		playing.push_back({ controller.states[transition.toStateIndex].clipIndex, 0.0f, transition.toStateIndex });
		transitionElapsed = 0.0f;
		transitionDuration = transition.blendDuration;
	}
}

void RuntimeAnimatorController::ConsumeTrigger(const AnimatorController& controller, const std::vector<AnimatorCondition>& conditions)
{
	// Triggerの消費処理
	for (const auto& c : conditions) {
		// parameterがTrigger型のものだけリセット。Float/Bool/Intは触らない
		if (controller.GetParameterType(c.parameterName) == AnimatorParameter::Type::Trigger) {
			parameterValues[c.parameterName] = 0.0f;
		}
	}

}
