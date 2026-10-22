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
		if (!model) { ImGui::TextDisabled(reinterpret_cast<const char*>(u8"プレビューはありません。")); return; }

		ImGui::Text(reinterpret_cast<const char*>(u8"モデルプレビュー: %s"), model->GetPath().c_str());
		
		// プレビュー描画
		if (auto previewImage = static_cast<RenderTexture*>(context->GetSharedResource("PreRenderTexture")))
		{
			// 16:9 のアスペクト比でプレビューを描画するためのサイズを計算
			float width = 720.0f; // プレビューの幅を固定
			ImVec2 size = { width, width * 9.0f / 16.0f };
			ImGui::Image(previewImage->GetSRV(), size);
		}

		ImGui::Separator();
		ImGui::Text(reinterpret_cast<const char*>(u8"ノード: %zu 個"), model->nodes.size());
		ImGui::Text(reinterpret_cast<const char*>(u8"メッシュ: %zu 個"), model->meshes.size());
		ImGui::Text(reinterpret_cast<const char*>(u8"マテリアル: %zu 件"), model->materials.size());
	}
	bool ModelImportSettingsDrawer::DrawSettingsFields(nlohmann::json& editingSettings, bool& isDirty)
	{
		auto settings = editingSettings.is_null()
			? ModelImportSettings{}
			: editingSettings.get<ModelImportSettings>();
		bool changed = false;
		/*changed |= */ImGui::InputFloat(reinterpret_cast<const char*>(u8"拡大率"), &settings.scaleFactor);
		changed |= ImGui::IsItemDeactivatedAfterEdit(); // スケール係数の入力が完了したときに変更を検知する
		changed |= ImGui::Checkbox(reinterpret_cast<const char*>(u8"静的バッチ処理"), &settings.staticBatching);

		// assimp のインポートフラグの設定
		{
			ImGui::Text(reinterpret_cast<const char*>(u8"リアルタイムプリセット:"));
			const char* presetNames[] = { "Custom", "Fast", "Quality", "MaxQuality" };
			int currentPreset = static_cast<int>(settings.preset);
			if (ImGui::Combo(reinterpret_cast<const char*>(u8"プリセット"), &currentPreset, presetNames, IM_ARRAYSIZE(presetNames)))
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

			changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"左手座標系に変換"), &settings.importFlags, aiProcess_ConvertToLeftHanded);
			if (settings.preset == 0) // Custom の場合のみ詳細設定を表示
			{
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"接空間を計算"), &settings.importFlags, aiProcess_CalcTangentSpace);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"同一頂点を結合"), &settings.importFlags, aiProcess_JoinIdenticalVertices);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"左手座標系にする"), &settings.importFlags, aiProcess_MakeLeftHanded);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"三角形化"), &settings.importFlags, aiProcess_Triangulate);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"法線を生成"), &settings.importFlags, aiProcess_GenNormals);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"滑らかな法線を生成"), &settings.importFlags, aiProcess_GenSmoothNormals);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"大きなメッシュを分割"), &settings.importFlags, aiProcess_SplitLargeMeshes);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"頂点を事前変換"), &settings.importFlags, aiProcess_PreTransformVertices);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"ボーンウェイトを制限"), &settings.importFlags, aiProcess_LimitBoneWeights);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"データ構造を検証"), &settings.importFlags, aiProcess_ValidateDataStructure);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"キャッシュ局所性を改善"), &settings.importFlags, aiProcess_ImproveCacheLocality);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"不要なマテリアルを削除"), &settings.importFlags, aiProcess_RemoveRedundantMaterials);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"内向き法線を修正"), &settings.importFlags, aiProcess_FixInfacingNormals);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"プリミティブ種別で並べ替え"), &settings.importFlags, aiProcess_SortByPType);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"縮退ポリゴンを検出"), &settings.importFlags, aiProcess_FindDegenerates);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"無効なデータを検出"), &settings.importFlags, aiProcess_FindInvalidData);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"UV座標を生成"), &settings.importFlags, aiProcess_GenUVCoords);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"UV座標を変換"), &settings.importFlags, aiProcess_TransformUVCoords);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"重複を検出"), &settings.importFlags, aiProcess_FindInstances);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"メッシュを最適化"), &settings.importFlags, aiProcess_OptimizeMeshes);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"グラフを最適化"), &settings.importFlags, aiProcess_OptimizeGraph);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"UVを反転"), &settings.importFlags, aiProcess_FlipUVs);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"頂点順を反転"), &settings.importFlags, aiProcess_FlipWindingOrder);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"ボーン数で分割"), &settings.importFlags, aiProcess_SplitByBoneCount);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"ボーンを削除"), &settings.importFlags, aiProcess_Debone);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"全体の拡大率"), &settings.importFlags, aiProcess_GlobalScale);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"テクスチャを埋め込む"), &settings.importFlags, aiProcess_EmbedTextures);
				changed |= ImGui::CheckboxFlags(reinterpret_cast<const char*>(u8"法線を強制生成"), &settings.importFlags, aiProcess_ForceGenNormals);
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

		ImGui::Text(reinterpret_cast<const char*>(u8"アニメーション: %zu 件"), g_modelRenderer.m_asset ? g_modelRenderer.m_asset->animations.size() : 0);

		// アニメーションの設定を表示
		if (g_modelRenderer.m_asset && !g_modelRenderer.m_asset->animations.empty())
		{
			ImGui::Separator();
			ImGui::Text(reinterpret_cast<const char*>(u8"アニメーション設定:"));
			int animationCount = static_cast<int>(g_modelRenderer.m_asset->animations.size());
			const char* animationNames[100]{}; // 最大100個のアニメーションをサポート
			for (int i = 0; i < animationCount && i < 100; ++i)
			{
				animationNames[i] = reinterpret_cast<const char*>(g_modelRenderer.m_asset->animations[i].name.c_str());
			}
			// アニメーションのインデックスを選択するコンボボックス
			if (ImGui::Combo(reinterpret_cast<const char*>(u8"アニメーション"), &g_modelRenderer.animationIndex, animationNames, animationCount))
			{
				g_modelRenderer.time = 0.0f; // アニメーションを切り替えたら時間をリセット
			}
			const char* commandNames[] = { "Play", "Pause" };
			ImGui::Checkbox(g_updateAnimation ? "Pause" : "Play", &g_updateAnimation);

			ImGui::InputFloat(reinterpret_cast<const char*>(u8"時間"), &g_modelRenderer.time);
			ImGui::InputFloat(reinterpret_cast<const char*>(u8"時間比率"), &g_modelRenderer.timeRate);
			ImGui::InputFloat(reinterpret_cast<const char*>(u8"ブレンド時間"), &g_modelRenderer.animationBlendTime);
			ImGui::Checkbox(reinterpret_cast<const char*>(u8"ループ"), &g_modelRenderer.loop);
		}

		static bool showMaterialSettings = false;
		ImGui::Checkbox(reinterpret_cast<const char*>(u8"マテリアル設定を表示"), &showMaterialSettings);
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