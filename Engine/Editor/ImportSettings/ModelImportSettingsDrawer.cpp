#include "pch.h"
#include "ModelImportSettingsDrawer.h"
#include <Engine\Resources\ImportSettings\ModelImportSettings.h>
#include <Engine\Resources\AssetModel.h>
#include <Engine\Rendering\Renderers\ModelRenderer.h>
#include <Engine\Rendering\Pipeline\Graphics.h>
#include <assimp\postprocess.h>


namespace CurryEngine::Resources
{
	static ModelRenderer g_modelRenderer;
	static bool g_updateAnimation = false;
	static bool g_isDirty = true;

	void ModelImportSettingsDrawer::Reset()
	{
		g_isDirty = true;
	}

	void ModelImportSettingsDrawer::Draw3DPreview(const std::shared_ptr<Resource>& previewResource, RenderContext* context)
	{
		auto model = std::dynamic_pointer_cast<AssetModel>(previewResource);
		if (g_isDirty)
		{
			g_modelRenderer = ModelRenderer(); // モデルが切り替わったときに古いモデルのリソースを解放するため、毎回新しいインスタンスを作る
			g_modelRenderer.SetModelAsset(model);
			g_isDirty = false;
		}
		// モデル描画
		if (model)
		{
			if (g_updateAnimation)
			{
				g_modelRenderer.Update(context->deltaTime);
			}
			g_modelRenderer.Draw(context);
		}
	}


	void ModelImportSettingsDrawer::DrawPreview(const std::shared_ptr<Resource>& previewResource, RenderContext* context)
	{
		auto model = std::dynamic_pointer_cast<AssetModel>(previewResource);
		if (!model) { ImGui::TextDisabled("No preview available."); return; }

		ImGui::Text("Model Preview: %s", model->GetPath().c_str());
		
		// プレビュー描画
		if (auto previewImage = static_cast<RenderTexture*>(context->GetSharedResource("PreRenderTexture")))
		{
			// 16:9 のアスペクト比でプレビューを描画するためのサイズを計算
			float width = 720.0f; // プレビューの幅を固定
			ImVec2 size = { width, width * 9.0f / 16.0f };
			ImGui::Image(previewImage->GetSRV(), size);
		}

		ImGui::Separator();
		ImGui::Text("Nodes: %zu", model->nodes.size());
		ImGui::Text("Meshes: %zu", model->meshes.size());
		ImGui::Text("Materials: %zu", model->materials.size());
	}
	bool ModelImportSettingsDrawer::DrawSettingsFields(nlohmann::json& editingSettings, bool& isDirty)
	{
		auto settings = editingSettings.is_null()
			? ModelImportSettings{}
			: editingSettings.get<ModelImportSettings>();
		bool changed = false;
		/*changed |= */ImGui::InputFloat("Scale Factor", &settings.scaleFactor);
		changed |= ImGui::IsItemDeactivatedAfterEdit(); // スケール係数の入力が完了したときに変更を検知する
		changed |= ImGui::Checkbox("Static Batching", &settings.staticBatching);

		// assimp のインポートフラグの設定
		{
			ImGui::Text("Realtime Presets:");
			const char* presetNames[] = { "Custom", "Fast", "Quality", "MaxQuality" };
			int currentPreset = static_cast<int>(settings.preset);
			if (ImGui::Combo("Preset", &currentPreset, presetNames, IM_ARRAYSIZE(presetNames)))
			{
				settings.preset = static_cast<char>(currentPreset);
				bool leftHanded = (settings.importFlags & aiProcess_ConvertToLeftHanded) != 0; // 左手座標系フラグを保持
				switch (settings.preset)
				{
				case 0: // Custom
					// カスタム設定の場合は何もしない
					break;
				case 1: // Fast
					settings.importFlags = aiProcessPreset_TargetRealtime_Fast;
					break;
				case 2: // Quality
					settings.importFlags = aiProcessPreset_TargetRealtime_Quality;
					break;
				case 3: // MaxQuality
					settings.importFlags = aiProcessPreset_TargetRealtime_MaxQuality;
					break;
				default:
					break;
				}
				if (settings.preset > 0 && leftHanded) settings.importFlags |= aiProcess_ConvertToLeftHanded; // 左手座標系フラグを再設定
				changed = true; // プリセットが変更された場合は変更フラグを立てる
			}

			changed |= ImGui::CheckboxFlags("Convert LeftHanded", &settings.importFlags, aiProcess_ConvertToLeftHanded);
			if (settings.preset == 0) // Custom の場合のみ詳細設定を表示
			{
				changed |= ImGui::CheckboxFlags("Calculate Tangent Space", &settings.importFlags, aiProcess_CalcTangentSpace);
				changed |= ImGui::CheckboxFlags("Join Identical Vertices", &settings.importFlags, aiProcess_JoinIdenticalVertices);
				changed |= ImGui::CheckboxFlags("Make Left Handed", &settings.importFlags, aiProcess_MakeLeftHanded);
				changed |= ImGui::CheckboxFlags("Triangulate", &settings.importFlags, aiProcess_Triangulate);
				changed |= ImGui::CheckboxFlags("Generate Normals", &settings.importFlags, aiProcess_GenNormals);
				changed |= ImGui::CheckboxFlags("Generate Smooth Normals", &settings.importFlags, aiProcess_GenSmoothNormals);
				changed |= ImGui::CheckboxFlags("Split Large Meshes", &settings.importFlags, aiProcess_SplitLargeMeshes);
				changed |= ImGui::CheckboxFlags("Pre-Transform Vertices", &settings.importFlags, aiProcess_PreTransformVertices);
				changed |= ImGui::CheckboxFlags("Limit Bone Weights", &settings.importFlags, aiProcess_LimitBoneWeights);
				changed |= ImGui::CheckboxFlags("Validate Data Structure", &settings.importFlags, aiProcess_ValidateDataStructure);
				changed |= ImGui::CheckboxFlags("Improve Cache Locality", &settings.importFlags, aiProcess_ImproveCacheLocality);
				changed |= ImGui::CheckboxFlags("Remove Redundant Materials", &settings.importFlags, aiProcess_RemoveRedundantMaterials);
				changed |= ImGui::CheckboxFlags("Fix Infacing Normals", &settings.importFlags, aiProcess_FixInfacingNormals);
				changed |= ImGui::CheckboxFlags("Sort By Primitive Type", &settings.importFlags, aiProcess_SortByPType);
				changed |= ImGui::CheckboxFlags("Find Degenerates", &settings.importFlags, aiProcess_FindDegenerates);
				changed |= ImGui::CheckboxFlags("Find Invalid Data", &settings.importFlags, aiProcess_FindInvalidData);
				changed |= ImGui::CheckboxFlags("Generate UV Coordinates", &settings.importFlags, aiProcess_GenUVCoords);
				changed |= ImGui::CheckboxFlags("Transform UV Coordinates", &settings.importFlags, aiProcess_TransformUVCoords);
				changed |= ImGui::CheckboxFlags("Find Instances", &settings.importFlags, aiProcess_FindInstances);
				changed |= ImGui::CheckboxFlags("Optimize Meshes", &settings.importFlags, aiProcess_OptimizeMeshes);
				changed |= ImGui::CheckboxFlags("Optimize Graph", &settings.importFlags, aiProcess_OptimizeGraph);
				changed |= ImGui::CheckboxFlags("Flip UVs", &settings.importFlags, aiProcess_FlipUVs);
				changed |= ImGui::CheckboxFlags("Flip Winding Order", &settings.importFlags, aiProcess_FlipWindingOrder);
				changed |= ImGui::CheckboxFlags("Split By Bone Count", &settings.importFlags, aiProcess_SplitByBoneCount);
				changed |= ImGui::CheckboxFlags("Debone", &settings.importFlags, aiProcess_Debone);
				changed |= ImGui::CheckboxFlags("Global Scale", &settings.importFlags, aiProcess_GlobalScale);
				changed |= ImGui::CheckboxFlags("Embed Textures", &settings.importFlags, aiProcess_EmbedTextures);
				changed |= ImGui::CheckboxFlags("Force Gen Normals", &settings.importFlags, aiProcess_ForceGenNormals);
			}
		}

		if (changed)
		{
			settings.scaleFactor = (std::max)(settings.scaleFactor, 0.0001f); // スケール係数が0以下にならないように制限

			editingSettings = settings;
			isDirty = true;
		}
		if (isDirty)
		{
			g_isDirty = true;
		}

		ImGui::Text("Animations: %zu", g_modelRenderer.m_asset ? g_modelRenderer.m_asset->animations.size() : 0);

		// アニメーションの設定を表示
		if (g_modelRenderer.m_asset && !g_modelRenderer.m_asset->animations.empty())
		{
			ImGui::Separator();
			ImGui::Text("Animation Settings:");
			int animationCount = static_cast<int>(g_modelRenderer.m_asset->animations.size());
			const char* animationNames[100]{}; // 最大100個のアニメーションをサポート
			for (int i = 0; i < animationCount && i < 100; ++i)
			{
				animationNames[i] = reinterpret_cast<const char*>(g_modelRenderer.m_asset->animations[i].name.c_str());
			}
			// アニメーションのインデックスを選択するコンボボックス
			if (ImGui::Combo("Animation", &g_modelRenderer.animationIndex, animationNames, animationCount))
			{
				g_modelRenderer.time = 0.0f; // アニメーションを切り替えたら時間をリセット
			}
			const char* commandNames[] = { "Play", "Pause" };
			ImGui::Checkbox(g_updateAnimation ? "Pause" : "Play", &g_updateAnimation);

			ImGui::InputFloat("Time", &g_modelRenderer.time);
			ImGui::InputFloat("Time Rate", &g_modelRenderer.timeRate);
			ImGui::InputFloat("Blend Time", &g_modelRenderer.animationBlendTime);
			ImGui::Checkbox("Loop", &g_modelRenderer.loop);
		}

		static bool showMaterialSettings = false;
		ImGui::Checkbox("Show Material Settings", &showMaterialSettings);
		if (g_modelRenderer.m_asset && showMaterialSettings)
		{
			ImGui::Separator();
			// マテリアルの設定を表示
			for (size_t i = 0; i < g_modelRenderer.m_asset->materials.size(); ++i)
			{
				auto& material = g_modelRenderer.m_asset->materials[i];
				if (ImGui::CollapsingHeader(("Material " + std::to_string(i)).c_str()))
				{
					material->DrawProperty();
				}
			}
		}


		return changed;
	}
	nlohmann::json ModelImportSettingsDrawer::GetDefaultSettings() const
	{
		// デフォルトコンストラクタの値がそのままデフォルト設定になるようにしているため、特に値を指定せずに返す
		return ModelImportSettings{};
	}
}