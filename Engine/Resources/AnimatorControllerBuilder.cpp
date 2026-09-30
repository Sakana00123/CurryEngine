#include "pch.h"
#include "AnimatorControllerBuilder.h"
#include "AssetDatabase.h"

namespace CurryEngine::Resources
{
	std::shared_ptr<AnimatorController> AnimatorControllerBuilder::BuildFromModelAsset(const ModelAsset& modelAsset, const AnimatorControllerBuildOptions& options)
	{
		auto controller = std::make_shared<AnimatorController>();
		std::filesystem::path modelPath(modelAsset.GetPath());
		controller->name = modelPath.stem().string();
		auto modelMeta = CurryEngine::Resources::AssetDatabase::GetOrImport(modelPath);
		if (modelMeta)
		{
			controller->modelAssetId = modelMeta->id;
		}

		int rootNodeIndex = -1;
		// ルートノードを探す(親を持たないノードかつ、名前が"root"を含み、skinとmeshが-1のノード)
		for (int i = 0; i < modelAsset.nodes.size(); ++i)
		{
			auto& node = modelAsset.nodes[i];
			std::string nodeNameLower = node.name;
			std::transform(nodeNameLower.begin(), nodeNameLower.end(), nodeNameLower.begin(), [](unsigned char c) { return std::tolower(c); });
			if (node.skin == -1 && node.mesh == -1 && (nodeNameLower.find("root") != std::string::npos || nodeNameLower.find("armature") != std::string::npos))
			{
				rootNodeIndex = i;
				break;
			}
		}

		if (options.generateStateForEachAnimationClip)
		{
			int i = 0;
			while (true)
			{
				std::filesystem::path animationPath(modelPath);
				animationPath.replace_filename(animationPath.stem().string() + "_" + std::to_string(i) + ".anim");
				if (!std::filesystem::exists(animationPath))
				{
					break;
				}
				auto animationMeta = CurryEngine::Resources::AssetDatabase::GetOrImport(animationPath);
				std::string animationName = controller->name + "_" + std::to_string(controller->states.size());
				if (animationMeta)
				{
					if (auto animationClip = CurryEngine::Resources::AssetDatabase::LoadAsset<AnimationClip>(animationMeta->id))
					{
						animationName = animationClip->name;
						controller->animationClips[animationMeta->id] = animationClip;
					}
				}

				AnimatorState state;
				state.name = animationName;
				state.clipId = animationMeta ? animationMeta->id : CurryEngine::Resources::AssetId(); // AnimationClipのIDを設定

				// AnimationTimelineの自動リンクが有効な場合、名前規則に従ってAnimationTimelineを探してリンクする
				if (options.autoLinkAnimationTimelines)
				{
					// 名前規則に従ってAnimationTimelineが存在する場合はそのIDを設定する
					std::filesystem::path timelinePath = animationPath;
					timelinePath.replace_filename(timelinePath.stem().string() + ".timeline");
					if (std::filesystem::exists(timelinePath))
					{
						if (auto timelineMeta = CurryEngine::Resources::AssetDatabase::GetOrImport(timelinePath))
						{
							state.timelineId = timelineMeta->id;
						}
					}
				}
				state.speed = 1.0f;
				state.loop = false;
				state.rootMotion = false;
				state.rootNodeIndex = rootNodeIndex;
				state.editorPosition = { 0.0f, i * 50.0f };
				controller->states.push_back(state);
				i++;
			}
		}
		return controller;
	}
}
