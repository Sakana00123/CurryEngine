#include "pch.h"
#include "AnimatorController.h"
#include "Engine/Resources/AssetDatabase.h"
#include <fstream>
#include <Engine\Core\ObjectManager.h>
#include <any>

// アニメーションの遷移中に使用するインデックス
#define INDEX_FRONT 0
// 遷移先のアニメーションのインデックス
#define INDEX_NEXT 1
// ブレンドして遷移中である場合のサイズ
#define TRANSITION_SIZE 2


bool AnimatorController::LoadFromFile(const std::string& path)
{
	_path = path;
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
	if (jsonData.contains("name"))
	{
		name = jsonData["name"].get<std::string>();
	}
	if (jsonData.contains("animationClipIds"))
	{
		for (const auto& clipIdStr : jsonData["animationClipIds"])
		{
			CurryEngine::Resources::AssetId clipId(clipIdStr.get<std::string>());
			// アセットデータベースからアニメーションクリップをロード
			auto clip = CurryEngine::Resources::AssetDatabase::LoadAsset<AnimationClip>(clipId);
			if (!clip)
			{
				LOG_ERROR(u8"[AnimatorController] アニメーションクリップの読み込みに失敗しました: " + std::u8string(clipId.ToString().begin(), clipId.ToString().end()));
			}
			animationClips[clipId] = clip;
		}
	}
	if (jsonData.contains("parameters"))
	{
		for (const auto& paramJson : jsonData["parameters"])
		{
			AnimatorParameter param;
			param.name = paramJson["name"].get<std::string>();
			param.type = static_cast<AnimatorParameter::Type>(paramJson["type"].get<int>());
			param.defaultValue = paramJson["defaultValue"].get<float>();
			// バインディング情報がある場合は読み込む
			if (paramJson.contains("binding"))
			{
				AnimatorParameterBinding binding;
				binding.sourceComponentId = ObjectId::FromString(paramJson["binding"]["sourceComponentId"].get<std::string>());
				binding.propertyName = paramJson["binding"]["propertyName"].get<std::string>();
				param.binding = binding;
			}
			parameters.push_back(param);
		}
	}
	if (jsonData.contains("states"))
	{
		for (const auto& stateJson : jsonData["states"])
		{
			AnimatorState state;
			state.name = stateJson["name"].get<std::string>();
			state.clipId = stateJson["clipId"].get<CurryEngine::Resources::AssetId>();
			state.speed = stateJson["speed"].get<float>();
			state.loop = stateJson["loop"].get<bool>();
			auto posArray = stateJson["editorPosition"];
			if (posArray.is_array() && posArray.size() == 2)
			{
				state.editorPosition.x = posArray[0].get<float>();
				state.editorPosition.y = posArray[1].get<float>();
			}
			states.push_back(state);
		}
	}
	if (jsonData.contains("transitions"))
	{
		for (const auto& transitionJson : jsonData["transitions"])
		{
			AnimatorTransition transition;
			transition.fromStateIndex = transitionJson["fromStateIndex"].get<int>();
			transition.toStateIndex = transitionJson["toStateIndex"].get<int>();
			transition.blendDuration = transitionJson["blendDuration"].get<float>();
			transition.hasExitTime = transitionJson["hasExitTime"].get<bool>();
			transition.exitTime = transitionJson["exitTime"].get<float>();
			for (const auto& conditionJson : transitionJson["conditions"])
			{
				AnimatorCondition condition;
				condition.parameterIndex = conditionJson["parameterIndex"].get<int>();
				condition.comparison = static_cast<AnimatorCondition::Comparison>(conditionJson["comparison"].get<int>());
				condition.value = conditionJson["value"].get<float>();
				transition.conditions.push_back(condition);
			}
			transitions.push_back(transition);
		}
	}
	if (jsonData.contains("defaultStateIndex"))
	{
		defaultStateIndex = jsonData["defaultStateIndex"].get<int>();
	}
	return true;
}

bool AnimatorController::SaveToFile(const std::filesystem::path& path) const
{
	// アニメーションクリップのIDリストをJSON形式で保存する
	nlohmann::json jsonData;
	jsonData["name"] = name;
	jsonData["animationClipIds"] = nlohmann::json::array();
	for (const auto& [clipId, clip] : animationClips)
	{
		jsonData["animationClipIds"].push_back(clipId.ToString());
	}
	jsonData["parameters"] = nlohmann::json::array();
	for (const auto& param : parameters)
	{
		nlohmann::json paramJson;
		paramJson["name"] = param.name;
		paramJson["type"] = static_cast<int>(param.type);
		paramJson["defaultValue"] = param.defaultValue;
		// バインディング情報がある場合は保存する
		if (param.binding.has_value())
		{
			nlohmann::json bindingJson;
			bindingJson["sourceComponentId"] = param.binding->sourceComponentId.ToString();
			bindingJson["propertyName"] = param.binding->propertyName;
			paramJson["binding"] = bindingJson;
		}
		jsonData["parameters"].push_back(paramJson);
	}
	jsonData["states"] = nlohmann::json::array();
	for (const auto& state : states)
	{
		nlohmann::json stateJson;
		stateJson["name"] = state.name;
		stateJson["clipId"] = state.clipId;
		stateJson["speed"] = state.speed;
		stateJson["loop"] = state.loop;
		stateJson["editorPosition"] = { state.editorPosition.x, state.editorPosition.y };
		jsonData["states"].push_back(stateJson);
	}
	jsonData["transitions"] = nlohmann::json::array();
	for (const auto& transition : transitions)
	{
		nlohmann::json transitionJson;
		transitionJson["fromStateIndex"] = transition.fromStateIndex;
		transitionJson["toStateIndex"] = transition.toStateIndex;
		transitionJson["blendDuration"] = transition.blendDuration;
		transitionJson["hasExitTime"] = transition.hasExitTime;
		transitionJson["exitTime"] = transition.exitTime;
		transitionJson["conditions"] = nlohmann::json::array();
		for (const auto& condition : transition.conditions)
		{
			nlohmann::json conditionJson;
			conditionJson["parameterIndex"] = condition.parameterIndex;
			conditionJson["comparison"] = static_cast<int>(condition.comparison);
			conditionJson["value"] = condition.value;
			transitionJson["conditions"].push_back(conditionJson);
		}
		jsonData["transitions"].push_back(transitionJson);
	}
	jsonData["defaultStateIndex"] = defaultStateIndex;

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

void RuntimeAnimatorController::Initialize(const AnimatorController& controller, std::vector<NodePose> initialPose)
{
	// 初期ポーズの設定
	currentPose = std::move(initialPose);

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
	if (!state.clipId.IsValid())
	{
		std::string errorMsg = "[RuntimeAnimatorController] 無効なクリップインデックス: " + state.clipId.id;
		std::u8string u8ErrorMsg(errorMsg.begin(), errorMsg.end());
		LOG_ERROR(u8ErrorMsg);
		return;
	}
	if (blendDuration <= 0.0f || playing.empty())
	{
		playing = { {state.clipId, 0.0f} };
	}
	else
	{
		playing.push_back({ state.clipId, 0.0f, stateIndex });
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

bool RuntimeAnimatorController::AllConditionsMet(const AnimatorController& controller, const std::vector<AnimatorCondition>& conditions) const
{
	for (const auto& condition : conditions)
	{
		auto& parameter = controller.parameters[condition.parameterIndex];

		float paramValue = GetParameterValue(controller, condition.parameterIndex);
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

float RuntimeAnimatorController::GetParameterValue(const AnimatorController& controller, int parameterIndex) const
{
	const auto& param = controller.parameters[parameterIndex];
	auto it = parameterValues.find(param.name);
	return (it != parameterValues.end()) ? it->second : param.defaultValue; // 初期化前はdefaultValueにフォールバック
}

void RuntimeAnimatorController::Update(float deltaTime, const AnimatorController& controller)
{
	// アニメーションの更新処理
	if (playing.empty()) return;
	if (currentStateIndex < 0 || currentStateIndex >= controller.states.size()) return;

	// bindingsの更新
	for (const auto& param : controller.parameters)
	{
		if (parameterValues.find(param.name) == parameterValues.end())
		{
			parameterValues[param.name] = param.defaultValue;
		}

		// バインディングが設定されていない場合はスキップ
		if (!param.binding.has_value())
		{
			continue;
		}

		// バインディングが設定されている場合、外部の変数を参照し、値を反映する
		auto& binding = param.binding.value();
		auto component = ObjectManager::FindComponent(binding.sourceComponentId);
		if (!component)
		{
			LOG_WARNING("[RuntimeAnimatorController] バインディング先のコンポーネントが見つかりません: " + binding.sourceComponentId.ToString());
			continue;
		}
		// リフレクションを使ってプロパティの値を取得する
		auto* meta = component->GetClassMeta(); // クラスメタデータを取得
		if (!meta)
		{
			LOG_WARNING("[RuntimeAnimatorController] バインディング先のコンポーネントのクラスメタデータが見つかりません: " + binding.sourceComponentId.ToString());
			continue;
		}
		auto* prop = meta->FindProperty(binding.propertyName); // プロパティ情報を取得
		if (!prop)
		{
			LOG_WARNING("[RuntimeAnimatorController] バインディング先のプロパティが見つかりません: " + binding.propertyName);
			continue;
		}
		switch (param.type)
		{
			case AnimatorParameter::Type::Float:
			{
				// float型のプロパティを取得
				float value = std::any_cast<float>(prop->getter(component.get()));
				parameterValues[param.name] = value;
				break;
			}
			case AnimatorParameter::Type::Int:
			{
				// int型のプロパティを取得
				int value = std::any_cast<int>(prop->getter(component.get()));
				parameterValues[param.name] = static_cast<float>(value);
				break;
			}
			case AnimatorParameter::Type::Bool:
			{
				// bool型のプロパティを取得
				bool value = std::any_cast<bool>(prop->getter(component.get()));
				parameterValues[param.name] = value ? 1.0 : 0.0f;
				break;
			}
		default:
			break;
		}
	}

	// 現在のステートを取得
	auto& currentState = controller.states[currentStateIndex];

	auto& state = playing[INDEX_FRONT];
	state.time += deltaTime * currentState.speed; // 再生速度を考慮して時間を進める

	if (!state.clipId.IsValid()) return;

	auto& clip = controller.animationClips.at(state.clipId);
	if (!clip) return;
	
	float normalizedTime = currentState.loop ? fmod(state.time / clip->duration, 1.0f) : (std::min)(state.time / clip->duration, 1.0f);
	static int count = 0;
	if (count++ % 60 == 0) {
		LOG_INFO("[RuntimeAnimatorController] Update: currentStateIndex=" + std::to_string(currentStateIndex) + ", normalizedTime=" + std::to_string(normalizedTime));
	}

	// アニメーションをサンプリングして現在のポーズを更新(weightは1.0fで固定)
	clip->Sample(normalizedTime * clip->duration, currentPose, 1.0f);

	// 遷移中でなければ、AnyStateの遷移条件 -> 現在のStateの遷移条件の順で条件をチェック
	if (transitionDuration <= 0.0f)
	{
		for (const auto& transition : controller.transitions)
		{
			if (transition.fromStateIndex != -1 && transition.fromStateIndex != currentStateIndex) continue; // 遷移元が現在のステートでない場合はスキップ
			if (transition.hasExitTime && normalizedTime < transition.exitTime) continue; // ExitTimeが設定されていて、まだExitTimeに達していない場合はスキップ
			if (transition.conditions.size() > 0)
			{
				if (!AllConditionsMet(controller, transition.conditions)) continue; // 条件を満たしていない場合はスキップ
			}
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
		if (nextPlayingState.clipId.IsValid())
		{
			auto& nextClip = controller.animationClips.at(nextPlayingState.clipId);
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
		playing.push_back({ controller.states[transition.toStateIndex].clipId, 0.0f, transition.toStateIndex });
		transitionElapsed = 0.0f;
		transitionDuration = transition.blendDuration;
	}
}

void RuntimeAnimatorController::ConsumeTrigger(const AnimatorController& controller, const std::vector<AnimatorCondition>& conditions)
{
	// Triggerの消費処理
	for (const auto& c : conditions) {
		// parameterがTrigger型のものだけリセット。Float/Bool/Intは触らない
		std::string paramName = controller.parameters[c.parameterIndex].name;
		if (controller.GetParameterType(paramName) == AnimatorParameter::Type::Trigger) {
			parameterValues[paramName] = 0.0f;
		}
	}

}
