#include "pch.h"
#include "Animator.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Rendering/Renderers/GltfModelRenderer.h"

#include "Engine/Resources/AnimationClip.h"
#include "Engine/Resources/AssetDatabase.h"

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

		if (ImGui::Button(" + "))
		{
			controller->states.push_back(AnimatorState{ "NewState", CurryEngine::Resources::AssetId(), 1.0f, true, Vector2(0, 0)});
		}
		ImGui::SameLine();
		if (ImGui::Button(" - "))
		{
			if (!controller->states.empty())
			{
				controller->states.pop_back();
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
