#include "pch.h"
#include "Animator.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Rendering/Renderers/GltfModelRenderer.h"

#include "Engine/Resources/AnimationClip.h"
#include "Engine/Resources/AssetDatabase.h"
#include <Engine\Editor\AnimatorControllerEditorWindow.h>

REGISTER_COMPONENT(Animator, "Animation")

void Animator::Awake()
{
	// コントローラーのリセット
	ResetController();
}

void Animator::Update(float deltaTime)
{
	// コントローラーが存在する場合、アニメーションの更新処理を行う
	if (controller)
	{
		// コントローラーの更新処理を呼び出す
		if (runtimeController.GetPose().empty())
		{
			if (auto renderer = GetScene()->FindComponentById<GltfModelRenderer>(targetModelRendererId))
			{
				runtimeController.Initialize(*controller, renderer->GetBindPose());
			}
			else
			{
				LOG_WARNING(u8"[Animator] GltfModelRenderer が見つかりません。アニメーションを再生できません。");
			}
		}
		runtimeController.Update(deltaTime, *controller);

		// GltfModelRenderer にアニメーションを適用する
		if (targetModelRendererId.IsValid())
		{
			if (auto renderer = GetScene()->FindComponentById<GltfModelRenderer>(targetModelRendererId))
			{
				renderer->ApplyPose(runtimeController.GetPose());
			}
		}

	}
}

#ifdef USE_IMGUI
void Animator::DrawProperty(const PropertyDrawContext& context)
{
    Component::DrawProperty(context); // 基底クラスの描画を呼び出す
    if (context.targets.size() != 1)
    {
        ImGui::Text("Multiple selection is not supported for Animator.");
        return;
	}

    if (controller)
    {
		for (const auto& [clipId, clip] : controller->animationClips)
		{
			ImGui::Text("Clip: %s", clip->name.c_str());
		}
		for (int i = 0; i < controller->states.size(); ++i)
		{
			const auto& state = controller->states[i];
			if (ImGui::Button(("Play State: " + state.name).c_str()))
			{
				if (runtimeController.GetPose().empty())
				{
					if (auto renderer = GetScene()->FindComponentById<GltfModelRenderer>(targetModelRendererId))
					{
						runtimeController.Initialize(*controller, renderer->GetBindPose());
					}
					else
					{
						LOG_WARNING(u8"[Animator] GltfModelRenderer が見つかりません。アニメーションを再生できません。");
					}
				}
				runtimeController.Play(*controller, i);
			}
		}

		ImGui::Separator();

		// controller->statesのエディタ

		for (int i = 0; i < controller->states.size(); ++i)
		{
			ImGui::PushID(i);
			const auto& state = controller->states[i];
			// ステートの名前を編集可能にする
			char buffer[128];
			strncpy_s(buffer, state.name.c_str(), sizeof(buffer));
			if (ImGui::InputText(("State Name##" + std::to_string(i)).c_str(), buffer, sizeof(buffer)))
			{
				controller->states[i].name = buffer;
			}

			std::string clipName = "None";
			if (state.clipId.IsValid())
			{
				auto it = controller->animationClips.find(state.clipId);
				if (it != controller->animationClips.end() && it->second)
				{
					clipName = it->second->name;
				}
			}
			ImGui::Text("Clip: %s", clipName.c_str());
			ImGui::SameLine();
			// アニメーションクリップを選択するためのコンボボックスを表示
			static std::unordered_map<CurryEngine::Resources::AssetId, std::shared_ptr<AnimationClip>> clipMap;
			if (ImGui::Button("..."))
			{
				ImGui::OpenPopup("SelectAnimationClipPopup");
				clipMap.clear();
				std::vector<CurryEngine::Resources::AssetMeta> metas = CurryEngine::Resources::AssetDatabase::FindAllByType(AssetType::Animation);
				for (const auto& meta : metas)
				{
					auto clip = CurryEngine::Resources::AssetDatabase::LoadAsset<AnimationClip>(meta.id);
					clipMap[meta.id] = clip;
				}
			}
			if (ImGui::BeginPopup("SelectAnimationClipPopup"))
			{
				for (const auto& [clipId, clip] : clipMap)
				{
					std::string clipLabel = clip->name + "##" + std::to_string(i);
					if (ImGui::Selectable(clipLabel.c_str()))
					{
						controller->animationClips[clipId] = clip;
						controller->states[i].clipId = clipId;
					}
				}
				ImGui::EndPopup();
			}

			// ステートのクリップインデックスを編集可能にする
			//ImGui::InputInt(("Clip Index##" + std::to_string(i)).c_str(), &controller->states[i].clipIndex);
			// ステートの再生
			ImGui::InputFloat(("Speed##" + std::to_string(i)).c_str(), &controller->states[i].speed);
			ImGui::Checkbox(("Loop##" + std::to_string(i)).c_str(),&controller->states[i].loop);

			ImGui::Separator();
			ImGui::PopID();
		}

		if (ImGui::Button(" + ##state"))
		{
			controller->states.push_back(AnimatorState{ "NewState", CurryEngine::Resources::AssetId(), 1.0f, true, Vector2(0, 0)});
		}
		ImGui::SameLine();
		if (ImGui::Button(" - ##state"))
		{
			if (!controller->states.empty())
			{
				controller->states.pop_back();
			}
		}

		ImGui::SeparatorText("Parameters");
		for (int i = 0; i < controller->parameters.size(); ++i)
		{
			ImGui::PushID(i);
			auto& parameter = controller->parameters[i];
			bool isChanged = false;
			float parameterValue = runtimeController.GetParameterValue(*controller.get(), i);
			// パラメータの名前を編集可能にする
			char buffer[128];
			strncpy_s(buffer, parameter.name.c_str(), sizeof(buffer));
			if (ImGui::InputText(("Parameter Name##" + std::to_string(i)).c_str(), buffer, sizeof(buffer)))
			{
				parameter.name = buffer;
			}

			// パラメータの型を選択するためのコンボボックスを表示
			const char* parameterTypes[] = { "Float", "Int", "Bool", "Trigger" };
			int currentTypeIndex = static_cast<int>(parameter.type);
			if (ImGui::BeginCombo(("Type##" + std::to_string(i)).c_str(), parameterTypes[currentTypeIndex]))
			{
				for (int j = 0; j < IM_ARRAYSIZE(parameterTypes); ++j)
				{
					bool isSelected = (currentTypeIndex == j);
					if (ImGui::Selectable(parameterTypes[j], isSelected))
					{
						parameter.type = static_cast<AnimatorParameter::Type>(j);
						isChanged |= true;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			// パラメータのデフォルト値を編集可能にする
			switch (parameter.type)
			{
				case AnimatorParameter::Type::Float:
				{
					isChanged |= ImGui::InputFloat(("Default Value##" + std::to_string(i)).c_str(), &parameterValue);
					break;
				}
				case AnimatorParameter::Type::Int:
				{
					int intValue = static_cast<int>(parameterValue);
					if (ImGui::InputInt(("Default Value##" + std::to_string(i)).c_str(), &intValue))
					{
						parameterValue = static_cast<float>(intValue);
						isChanged |= true;
					}
					break;
				}
				case AnimatorParameter::Type::Bool:
				{
					bool boolValue = (parameterValue != 0.0f);
					if (ImGui::Checkbox(("Default Value##" + std::to_string(i)).c_str(), &boolValue))
					{
						parameterValue = boolValue ? 1.0f : 0.0f;
						isChanged |= true;
					}
					break;
				}
				case AnimatorParameter::Type::Trigger:
				{
					if (ImGui::RadioButton("##Trigger", parameterValue))
					{
						// Triggerはボタンを押すと1.0fに設定される
						runtimeController.SetTrigger(parameter.name);
					}
					break;
				}
			default:
				break;
			}
			if (isChanged)
			{
				runtimeController.SetFloat(parameter.name, parameterValue);
			}
			if (parameter.type != AnimatorParameter::Type::Trigger)
			{
				ImGui::SameLine();
				// バインディングの表示
				std::string bindingText = "None";
				if (parameter.binding.has_value())
				{
					if (auto comp = ObjectManager::FindComponent(parameter.binding->sourceComponentId))
					{
						bindingText = "(" + comp->GetOwner()->GetName() + ")" + comp->GetTypeName() + "." + parameter.binding->propertyName;
					}
				}
				ImGui::Text("Binding: %s", bindingText.c_str());

				ImGui::SameLine();
				// バインディング設定
				if (ImGui::Button(("...##" + std::to_string(i)).c_str()))
				{
					ImGui::OpenPopup("BindPopup");
				}
				if (ImGui::BeginPopup("BindPopup"))
				{
					// 検索ボックス
					static char searchBuffer[128] = "";
					ImGui::InputText("##Search", searchBuffer, sizeof(searchBuffer));
					
					auto scene = GetScene();
					if (scene)
					{
						auto& componentCacheMap = scene->GetObjectManager()->GetComponentCacheMap();
						// 検索結果を表示
						for (const auto& [compId, weakComp] : componentCacheMap)
						{
							auto comp = weakComp.lock();
							if (!comp) continue;
							auto classMeta = comp->GetClassMeta();
							if (!classMeta) continue;
							// プロパティを表示
							for (const auto& property : classMeta->properties)
							{
								const std::string& propertyName = property.name;
								std::string propertyFullName = "(" + comp->GetOwner()->GetName() + ")" + comp->GetTypeName() + "." + propertyName;
								// 検索フィルタに一致するプロパティのみ表示
								if (searchBuffer[0] != '\0' && propertyFullName.find(searchBuffer) == std::string::npos)
								{
									continue;
								}

								// プロパティの型がパラメータの型と一致する場合のみ表示
								bool isTypeMatch = false;
								switch (parameter.type)
								{
								case AnimatorParameter::Type::Float:
									isTypeMatch = (property.type == "float");
									break;
								case AnimatorParameter::Type::Int:
									isTypeMatch = (property.type == "int");
									break;
								case AnimatorParameter::Type::Bool:
									isTypeMatch = (property.type == "bool");
									break;
								default:
									break;
								}
								// 型が一致しない場合はスキップ
								if (!isTypeMatch) continue;

								// プロパティを選択可能にする
								if (ImGui::Selectable(propertyFullName.c_str()))
								{
									parameter.binding = AnimatorParameterBinding{ compId, propertyName };
									ImGui::CloseCurrentPopup();
								}
							}
						}
					}
					ImGui::EndPopup();
				}
			}

			ImGui::Separator();

			ImGui::PopID();
		}
		if (ImGui::Button(" + ##parameter"))
		{
			controller->parameters.push_back(AnimatorParameter{ "NewParameter", AnimatorParameter::Type::Float, 0.0f });
		}
		ImGui::SameLine();
		if (ImGui::Button(" - ##parameter"))
		{
			if (!controller->parameters.empty())
			{
				controller->parameters.pop_back();
			}
		}

		ImGui::SeparatorText("Transitions");
		for (int i = 0; i < controller->transitions.size(); ++i)
		{
			const auto& transition = controller->transitions[i];
			const auto& parameters = controller->parameters;
			ImGui::PushID(i);

			// 遷移元のステートを選択するためのコンボボックスを表示
			ImGui::Text("From State:");
			ImGui::SameLine();
			std::string fromStateName = (transition.fromStateIndex >= 0 && transition.fromStateIndex < controller->states.size()) ? controller->states[transition.fromStateIndex].name : "None";
			if (ImGui::BeginCombo(("##FromState" + std::to_string(i)).c_str(), fromStateName.c_str()))
			{
				for (int j = 0; j < controller->states.size(); ++j)
				{
					bool isSelected = (transition.fromStateIndex == j);
					if (ImGui::Selectable(controller->states[j].name.c_str(), isSelected))
					{
						controller->transitions[i].fromStateIndex = j;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			// 遷移先のステートを選択するためのコンボボックスを表示
			ImGui::Text("To State:");
			ImGui::SameLine();
			std::string toStateName = (transition.toStateIndex >= 0 && transition.toStateIndex < controller->states.size()) ? controller->states[transition.toStateIndex].name : "None";
			if (ImGui::BeginCombo(("##ToState" + std::to_string(i)).c_str(), toStateName.c_str()))
			{
				for (int j = 0; j < controller->states.size(); ++j)
				{
					bool isSelected = (transition.toStateIndex == j);
					if (ImGui::Selectable(controller->states[j].name.c_str(), isSelected))
					{
						controller->transitions[i].toStateIndex = j;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			// 遷移のブレンド時間の設定
			ImGui::InputFloat(("Blend Duration##" + std::to_string(i)).c_str(), &controller->transitions[i].blendDuration);

			ImGui::Separator();

			// 遷移条件の設定
			ImGui::Text("Conditions:");
			for (int j = 0; j < transition.conditions.size(); ++j)
			{
				const auto& condition = transition.conditions[j];
				ImGui::PushID(j);
				ImGui::Text("Parameter:");
				ImGui::SameLine();
				std::string parameterName = "None";
				if (condition.parameterIndex >= 0 && condition.parameterIndex < parameters.size())
				{
					parameterName = parameters[condition.parameterIndex].name;
				}
				if (ImGui::BeginCombo(("##Parameter" + std::to_string(i) + "_" + std::to_string(j)).c_str(), parameterName.c_str()))
				{
					for (int k = 0; k < parameters.size(); ++k)
					{
						const auto& param = parameters[k];
						bool isSelected = (parameterName == param.name);
						if (ImGui::Selectable(param.name.c_str(), isSelected))
						{
							controller->transitions[i].conditions[j].parameterIndex = k;
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				// 遷移条件の比較演算子と値の設定
				bool isParameterTrigger = false;
				if (condition.parameterIndex >= 0 && condition.parameterIndex < parameters.size())
				{
					isParameterTrigger = (parameters[condition.parameterIndex].type == AnimatorParameter::Type::Trigger);
				}
				if (!isParameterTrigger)
				{
					// 比較演算子の設定
					const char* comparisonTypes[] = { "<", "<=", ">", ">=", "==", "!=" };
					int currentComparisonIndex = static_cast<int>(condition.comparison);
					if (ImGui::BeginCombo(("##Comparison" + std::to_string(i) + "_" + std::to_string(j)).c_str(), comparisonTypes[currentComparisonIndex]))
					{
						for (int k = 0; k < IM_ARRAYSIZE(comparisonTypes); ++k)
						{
							bool isSelected = (currentComparisonIndex == k);
							if (ImGui::Selectable(comparisonTypes[k], isSelected))
							{
								controller->transitions[i].conditions[j].comparison = static_cast<AnimatorCondition::Comparison>(k);
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
					ImGui::InputFloat(("Value##" + std::to_string(i) + "_" + std::to_string(j)).c_str(), &controller->transitions[i].conditions[j].value);
				}
				ImGui::PopID();
			}
			// 遷移条件の追加
			if (ImGui::Button((" + ##condition" + std::to_string(i)).c_str()))
			{
				controller->transitions[i].conditions.push_back(AnimatorCondition{ -1, AnimatorCondition::Comparison::Equal, 0.0f});
			}
			ImGui::SameLine();
			// 遷移条件の削除
			if (ImGui::Button((" - ##condition" + std::to_string(i)).c_str()))
			{
				if (!controller->transitions[i].conditions.empty())
				{
					controller->transitions[i].conditions.pop_back();
				}
			}
			ImGui::Separator();

			// Exit Timeの有効化/無効化の設定
			ImGui::Checkbox(("Has Exit Time##" + std::to_string(i)).c_str(), &controller->transitions[i].hasExitTime);

			// Exit Timeの設定
			if (controller->transitions[i].hasExitTime)
			{
				ImGui::SliderFloat(("Exit Time##" + std::to_string(i)).c_str(), &controller->transitions[i].exitTime, 0.0f, 1.0f);
			}

			ImGui::Separator();

			ImGui::PopID();
		}
		ImGui::Separator();

		if (ImGui::Button(" + ##transition"))
		{
			controller->transitions.push_back(AnimatorTransition{ -1, -1, 0.25f, {}, false, 0.0f });
		}
		ImGui::SameLine();
		if (ImGui::Button(" - ##transition"))
		{
			if (!controller->transitions.empty())
			{
				controller->transitions.pop_back();
			}
		}
	}
}
#endif // USE_IMGUI

void Animator::ResetController()
{
	controller = CurryEngine::Resources::AssetDatabase::LoadAsset<AnimatorController>(controllerAssetId);
	if (!controller) 
	{
		runtimeController = RuntimeAnimatorController();
		return;
	}
	// コントローラーの初期化
	if (auto renderer = GetScene()->FindComponentById<GltfModelRenderer>(targetModelRendererId))
	{
		runtimeController.Initialize(*controller, renderer->GetBindPose());
	}
}

json Animator::Serialize() const
{
	json j = Component::Serialize();
	// controllerをファイルに保存
	if (controller)
	{
		controller->SaveToFile(controller->GetPath());
	}
	// controllerのAssetIdを保存
	j["controllerAssetId"] = controllerAssetId.ToString();

	return j;
}

void Animator::Deserialize(const json& j)
{
	Component::Deserialize(j);
	// controllerのAssetIdを読み込む
	if (j.contains("controllerAssetId"))
	{
		controllerAssetId = j["controllerAssetId"].get<CurryEngine::Resources::AssetId>();
	}
	else
	{
		controllerAssetId = CurryEngine::Resources::AssetId();
	}
	// controllerをロード
	ResetController();
}
