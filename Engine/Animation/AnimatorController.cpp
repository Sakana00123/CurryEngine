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
		animationClips.clear();
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
		parameters.clear();
		for (const auto& paramJson : jsonData["parameters"])
		{
			AnimatorParameter param;
			param.name = paramJson["name"].get<std::string>();
			param.type = static_cast<AnimatorParameter::Type>(paramJson["type"].get<int>());
			param.defaultValue = paramJson["defaultValue"].get<float>();
#ifdef ENABLE_ANIMATOR_PARAMETER_BINDING
			// バインディング情報がある場合は読み込む
			if (paramJson.contains("binding"))
			{
				AnimatorParameterBinding binding;
				binding.sourceComponentId = ObjectId::FromString(paramJson["binding"]["sourceComponentId"].get<std::string>());
				binding.propertyName = paramJson["binding"]["propertyName"].get<std::string>();
				param.binding = binding;
			}
#endif // ENABLE_ANIMATOR_PARAMETER_BINDING
			parameters.push_back(param);
		}
	}
	if (jsonData.contains("states"))
	{
		states.clear();
		for (const auto& stateJson : jsonData["states"])
		{
			AnimatorState state;
			state.name = stateJson["name"].get<std::string>();
			state.clipId = stateJson["clipId"].get<CurryEngine::Resources::AssetId>();
			state.speed = stateJson["speed"].get<float>();
			state.loop = stateJson["loop"].get<bool>();
			state.rootMotion = stateJson.value<bool>("rootMotion", false);
			state.rootNodeIndex = stateJson.value<int>("rootNodeIndex", -1);
			state.rootMotionXZ = stateJson.value<bool>("rootMotionXZ", true);
			state.rootMotionY = stateJson.value<bool>("rootMotionY", true);
			auto& posArray = stateJson["editorPosition"];
			if (posArray.is_array() && posArray.size() == 2)
			{
				state.editorPosition.x = posArray[0].get<float>();
				state.editorPosition.y = posArray[1].get<float>();
			}

			state.blendType = static_cast<BlendTreeType>(stateJson.value<int>("blendType", 0));
			state.blendParamXIndex = stateJson.value<int>("blendParamXIndex", -1);
			state.blendParamYIndex = stateJson.value<int>("blendParamYIndex", -1);
			if (stateJson.contains("blendEntries"))
			{
				for (const auto& entryJson : stateJson["blendEntries"])
				{
					BlendTreeEntry entry;
					entry.clipId = entryJson["clipId"].get<CurryEngine::Resources::AssetId>();
					entry.threshold = entryJson["threshold"].get<float>();
					auto& entryPosArray = entryJson["position"];
					if (entryPosArray.is_array() && entryPosArray.size() == 2)
					{
						entry.position.x = entryPosArray[0].get<float>();
						entry.position.y = entryPosArray[1].get<float>();
					}
					state.blendEntries.push_back(entry);
				}
			}
			state.blendSmoothTime = stateJson.value<float>("blendSmoothTime", 0.0f);

			states.push_back(state);
		}
	}
	if (jsonData.contains("transitions"))
	{
		transitions.clear();
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
#ifdef ENABLE_ANIMATOR_PARAMETER_BINDING
		// バインディング情報がある場合は保存する
		if (param.binding.has_value())
		{
			nlohmann::json bindingJson;
			bindingJson["sourceComponentId"] = param.binding->sourceComponentId.ToString();
			bindingJson["propertyName"] = param.binding->propertyName;
			paramJson["binding"] = bindingJson;
		}
#endif // ENABLE_ANIMATOR_PARAMETER_BINDING
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
		stateJson["rootMotion"] = state.rootMotion;
		stateJson["rootNodeIndex"] = state.rootNodeIndex;
		stateJson["rootMotionXZ"] = state.rootMotionXZ;
		stateJson["rootMotionY"] = state.rootMotionY;
		stateJson["editorPosition"] = { state.editorPosition.x, state.editorPosition.y };

		stateJson["blendType"] = static_cast<int>(state.blendType);
		stateJson["blendParamXIndex"] = state.blendParamXIndex;
		stateJson["blendParamYIndex"] = state.blendParamYIndex;
		stateJson["blendEntries"] = nlohmann::json::array();
		for (const auto& entry : state.blendEntries)
		{
			nlohmann::json entryJson;
			entryJson["clipId"] = entry.clipId;
			entryJson["threshold"] = entry.threshold;
			entryJson["position"] = { entry.position.x, entry.position.y };
			stateJson["blendEntries"].push_back(entryJson);
		}
		stateJson["blendSmoothTime"] = state.blendSmoothTime;

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
	currentPose = initialPose;
	bindPose = std::move(initialPose);

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
		playing = { { 0.0f, stateIndex} };
		transitionElapsed = 0.0f;
		transitionDuration = 0.0f;
	}
	else
	{
		playing.push_back({ 0.0f, stateIndex });
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
		if (condition.parameterIndex < 0 || condition.parameterIndex >= controller.parameters.size())
		{
			continue; // 無効なパラメータインデックスはスキップ
		}
		auto& parameter = controller.parameters[condition.parameterIndex];

		float paramValue = GetParameterValue(controller, condition.parameterIndex);
		if (parameter.type == AnimatorParameter::Type::Trigger)
		{
			return paramValue > 0.9f; // Triggerは1.0fで表現されるため、0.9f以上ならトリガーが有効とみなす
		}
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

std::vector<RuntimeAnimatorController::BlendedClipWeight>
RuntimeAnimatorController::ComputeBlendWeights(const AnimatorController& controller, const PlayingState& state) const
{
	std::vector<BlendedClipWeight> result;
	if (state.stateIndex < 0 || state.stateIndex >= (int)controller.states.size()) return result;

	const AnimatorState& animState = controller.states[state.stateIndex];

	if (!animState.IsBlendTree())
	{
		if (animState.clipId.IsValid()) result.push_back({ animState.clipId, 1.0f });
		return result;
	}
	if (animState.blendEntries.empty()) return result;
	if (animState.blendEntries.size() == 1)
	{
		result.push_back({ animState.blendEntries[0].clipId, 1.0f });
		return result;
	}

	if (animState.blendType == BlendTreeType::Simple1D)
	{
		const float value = GetParameterValue(controller, animState.blendParamXIndex);
		const auto& entries = animState.blendEntries; // しきい値昇順ソート済み前提

		if (value <= entries.front().threshold) { result.push_back({ entries.front().clipId, 1.0f }); return result; }
		if (value >= entries.back().threshold) { result.push_back({ entries.back().clipId, 1.0f });  return result; }

		for (size_t i = 0; i + 1 < entries.size(); ++i)
		{
			if (value >= entries[i].threshold && value <= entries[i + 1].threshold)
			{
				const float span = entries[i + 1].threshold - entries[i].threshold;
				const float t = span > 0.0f ? (value - entries[i].threshold) / span : 0.0f;
				result.push_back({ entries[i].clipId, 1.0f - t });
				result.push_back({ entries[i + 1].clipId, t });
				break;
			}
		}
		return result;
	}

	if (animState.blendType == BlendTreeType::FreeformCartesian2D)
	{
		const Vector2 p{ GetParameterValue(controller, animState.blendParamXIndex),
						  GetParameterValue(controller, animState.blendParamYIndex) };
		const size_t n = animState.blendEntries.size();
		std::vector<float> w(n, 1.0f);

		// Gradient Band Interpolation（Juckett方式）
		for (size_t i = 0; i < n; ++i)
		{
			const Vector2& pi = animState.blendEntries[i].position;
			for (size_t j = 0; j < n; ++j)
			{
				if (i == j) continue;
				const Vector2& pj = animState.blendEntries[j].position;
				const Vector2 hij{ pj.x - pi.x, pj.y - pi.y };
				const float lenSq = hij.x * hij.x + hij.y * hij.y;
				if (lenSq < 1e-6f) continue;

				const Vector2 diff{ p.x - pi.x, p.y - pi.y };
				const float proj = (diff.x * hij.x + diff.y * hij.y) / lenSq;
				w[i] = std::min(w[i], std::clamp(1.0f - proj, 0.0f, 1.0f));
			}
		}

		float sum = 0.0f;
		for (float wi : w) sum += wi;
		if (sum <= 1e-6f) { result.push_back({ animState.blendEntries.front().clipId, 1.0f }); return result; }

		for (size_t i = 0; i < n; ++i)
			if (w[i] > 1e-6f) result.push_back({ animState.blendEntries[i].clipId, w[i] / sum });
		return result;
	}

	result.push_back({ animState.blendEntries.front().clipId, 1.0f });
	return result;
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

#ifdef ENABLE_ANIMATOR_PARAMETER_BINDING
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
#endif // ENABLE_ANIMATOR_PARAMETER_BINDING

	// 現在のステートを取得
	auto& currentState = controller.states[currentStateIndex];

	auto& frontPlaying = playing[INDEX_FRONT];
	frontPlaying.time += deltaTime * currentState.speed;

	auto frontTargetWeights = ComputeBlendWeights(controller, frontPlaying);
	auto frontWeights = SmoothBlendWeights(frontPlaying.blendWeights, frontTargetWeights, currentState.blendSmoothTime, deltaTime);
	const float frontAvgDuration = ResolveAverageDuration(controller, frontWeights);
	if (frontAvgDuration <= 0.0f) return;
	const float frontNormalizedTime = currentState.loop
		? fmod(frontPlaying.time / frontAvgDuration, 1.0f)
		: (std::min)(frontPlaying.time / frontAvgDuration, 1.0f);

	if (currentState.rootMotion)
	{
		for (const auto& bw : frontWeights)
		{
			auto it = controller.animationClips.find(bw.clipId);
			if (it == controller.animationClips.end() || !it->second) continue;
			const float duration = it->second->duration;

			XMFLOAT3 dT{}; XMFLOAT4 dR{ 0,0,0,1 };
			if (currentState.loop && frontNormalizedTime < rootMotionLastNormalizedTime)
			{
				// ループで先頭に戻った：終端までの分＋先頭からの分を合成
				XMFLOAT3 t1{}, t2{}; XMFLOAT4 r1{ 0,0,0,1 }, r2{ 0,0,0,1 };
				it->second->SampleRootMotion(rootMotionLastNormalizedTime * duration, duration, t1, r1, currentState.rootNodeIndex, currentState.rootMotionXZ, currentState.rootMotionY);
				it->second->SampleRootMotion(0.0f, frontNormalizedTime * duration, t2, r2, currentState.rootNodeIndex, currentState.rootMotionXZ, currentState.rootMotionY);
				dT = { t1.x + t2.x, t1.y + t2.y, t1.z + t2.z };
				XMStoreFloat4(&dR, XMQuaternionMultiply(XMLoadFloat4(&r1), XMLoadFloat4(&r2)));
			}
			else
			{
				it->second->SampleRootMotion(rootMotionLastNormalizedTime * duration, frontNormalizedTime * duration, dT, dR, currentState.rootNodeIndex, currentState.rootMotionXZ, currentState.rootMotionY);
			}
			rootMotionDeltaPosition.x += dT.x; rootMotionDeltaPosition.y += dT.y; rootMotionDeltaPosition.z += dT.z;
			XMStoreFloat4(&rootMotionDeltaRotation,
				XMQuaternionMultiply(XMLoadFloat4(&rootMotionDeltaRotation), XMLoadFloat4(&dR)));
			break; // 最初の有効クリップのみ（ブレンドツリー中の合成は今後の課題）
		}
	}
	rootMotionLastNormalizedTime = frontNormalizedTime;

	if (transitionDuration <= 0.0f)
	{
		for (const auto& transition : controller.transitions)
		{
			if (transition.fromStateIndex != -1 && transition.fromStateIndex != currentStateIndex) continue;
			if (transition.hasExitTime && frontNormalizedTime < transition.exitTime) continue;
			if (transition.conditions.size() > 0 && !AllConditionsMet(controller, transition.conditions)) continue;
			BeginTransition(transition, controller);
			ConsumeTrigger(controller, transition.conditions);
			break;
		}
	}

	float t = 0.0f;
	std::vector<BlendedClipWeight> nextWeights;
	float nextNormalizedTime = 0.0f;
	const bool transitioning = (playing.size() == TRANSITION_SIZE);

	if (transitioning)
	{
		auto& nextPlaying = playing[INDEX_NEXT];
		if (nextPlaying.stateIndex < 0 || nextPlaying.stateIndex >= (int)controller.states.size()) return;
		auto& nextState = controller.states[nextPlaying.stateIndex];
		nextPlaying.time += deltaTime * nextState.speed;
		transitionElapsed += deltaTime;
		t = transitionDuration > 0.0f ? std::clamp(transitionElapsed / transitionDuration, 0.0f, 1.0f) : 1.0f;

		auto nextTargetWeights = ComputeBlendWeights(controller, nextPlaying);
		nextWeights = SmoothBlendWeights(nextPlaying.blendWeights, nextTargetWeights, nextState.blendSmoothTime, deltaTime);
		const float nextAvgDuration = ResolveAverageDuration(controller, nextWeights);
		if (nextAvgDuration > 0.0f)
		{
			nextNormalizedTime = nextState.loop
				? fmod(nextPlaying.time / nextAvgDuration, 1.0f)
				: (std::min)(nextPlaying.time / nextAvgDuration, 1.0f);
		}
	}

	// フロントとネクストのウェイトを合成して最終的なポーズを計算
	CompositePose(controller, frontWeights, frontNormalizedTime, 1.0f - t, nextWeights, nextNormalizedTime, t);
	
	// ルートモーションで抜き出した軸は、スケルトンのローカル姿勢からは打ち消す
	if (currentState.rootMotion &&
		currentState.rootNodeIndex >= 0 &&
		currentState.rootNodeIndex < (int)currentPose.size() &&
		currentState.rootNodeIndex < (int)bindPose.size())
	{
		auto& rootPose = currentPose[currentState.rootNodeIndex];
		const auto& bindRoot = bindPose[currentState.rootNodeIndex];
		if (currentState.rootMotionXZ)
		{
			rootPose.translation.x = bindRoot.translation.x;
			rootPose.translation.z = bindRoot.translation.z;
		}
		if (currentState.rootMotionY)
		{
			rootPose.translation.y = bindRoot.translation.y;
		}
	}

	if (transitioning && transitionElapsed >= transitionDuration)
	{
		if (playing.size() > 1) playing.erase(playing.begin());
		transitionElapsed = 0.0f;
		transitionDuration = 0.0f;
		currentStateIndex = playing[INDEX_FRONT].stateIndex;
	}
}

void RuntimeAnimatorController::BeginTransition(const AnimatorTransition& transition, const AnimatorController& controller)
{
	// 遷移の開始処理
	if (transition.toStateIndex >= 0 && transition.toStateIndex < controller.states.size())
	{
		if (playing.size() == TRANSITION_SIZE)
		{
			// 既に遷移中の場合は、ネクストを入れ替える
			playing[INDEX_NEXT] = { 0.0f, transition.toStateIndex };
		}
		else
		{
			playing.push_back({ 0.0f, transition.toStateIndex });
		}
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

void RuntimeAnimatorController::ConsumeRootMotion(const AnimatorController& controller, XMFLOAT3& outDeltaPosition, XMFLOAT4& outDeltaRotation)
{
	outDeltaPosition = rootMotionDeltaPosition;
	outDeltaRotation = rootMotionDeltaRotation;
	rootMotionDeltaPosition = { 0.0f, 0.0f, 0.0f };
	rootMotionDeltaRotation = { 0.0f, 0.0f, 0.0f, 1.0f };
}

// weightsの平均クリップ長を求める（single-clipなら単にそのクリップの長さになる = 既存挙動と一致）
float RuntimeAnimatorController::ResolveAverageDuration(const AnimatorController& controller, const std::vector<BlendedClipWeight>& weights) const
{
	float avg = 0.0f;
	for (const auto& bw : weights)
	{
		auto it = controller.animationClips.find(bw.clipId);
		if (it != controller.animationClips.end() && it->second) avg += bw.weight * it->second->duration;
	}
	return avg;
}

// フロント/ネクストの全エントリを(1-t)/tでスケールしてまとめ、1本の累積合成ループでcurrentPoseに書き込む
void RuntimeAnimatorController::CompositePose(
	const AnimatorController& controller,
	const std::vector<BlendedClipWeight>& frontWeights, float frontTime, float frontScale,
	const std::vector<BlendedClipWeight>& nextWeights, float nextTime, float nextScale)
{
	struct FlatSample { CurryEngine::Resources::AssetId clipId; float time; float weight; };
	std::vector<FlatSample> flat;
	for (const auto& bw : frontWeights) if (bw.weight > 0.0f) flat.push_back({ bw.clipId, frontTime, bw.weight * frontScale });
	for (const auto& bw : nextWeights)  if (bw.weight > 0.0f && nextScale > 0.0f) flat.push_back({ bw.clipId, nextTime, bw.weight * nextScale });
	if (flat.empty()) return;

	float cumulative = 0.0f;
	for (auto& s : flat)
	{
		auto it = controller.animationClips.find(s.clipId);
		if (it == controller.animationClips.end() || !it->second) continue;
		const float sampleTime = s.time * it->second->duration;

		if (cumulative <= 0.0f) { it->second->Sample(sampleTime, currentPose, 1.0f); cumulative = s.weight; }
		else { cumulative += s.weight; it->second->Sample(sampleTime, currentPose, s.weight / cumulative); }
	}
}

std::vector<RuntimeAnimatorController::BlendedClipWeight> RuntimeAnimatorController::SmoothBlendWeights(
	std::unordered_map<CurryEngine::Resources::AssetId, float>& current,
	const std::vector<BlendedClipWeight>& target,
	float smoothTime, float deltaTime) const
{
	std::unordered_map<CurryEngine::Resources::AssetId, float> targetMap;
	for (const auto& t : target) targetMap[t.clipId] = t.weight;

	if (smoothTime <= 0.0f)
	{
		current = targetMap; // 従来通り即時切替
	}
	else
	{
		const float alpha = 1.0f - std::exp(-deltaTime / smoothTime);

		// 既存エントリを目標へ近づける(目標に無いものは0へ収束させて消す)
		for (auto it = current.begin(); it != current.end(); )
		{
			auto found = targetMap.find(it->first);
			const float targetWeight = (found != targetMap.end()) ? found->second : 0.0f;
			it->second += (targetWeight - it->second) * alpha;

			if (it->second < 1e-4f && found == targetMap.end())
				it = current.erase(it); // 完全にフェードアウトしたら破棄(マップ肥大化防止)
			else
				++it;
		}
		// 新しく現れたエントリは0から立ち上げる
		for (const auto& [clipId, weight] : targetMap)
		{
			if (current.find(clipId) == current.end())
				current[clipId] = weight * alpha;
		}
	}

	// 合計1.0になるよう正規化
	float sum = 0.0f;
	for (const auto& [clipId, weight] : current) sum += weight;

	std::vector<BlendedClipWeight> result;
	if (sum <= 1e-6f) return result;
	for (const auto& [clipId, weight] : current)
		if (weight > 1e-6f) result.push_back({ clipId, weight / sum });
	return result;
}
