#include "pch.h"
#include "ModelImportSettingsDrawer.h"
#include <Engine\Resources\ImportSettings\ModelImportSettings.h>
#include <Engine\Resources\AssetModel.h>
#include <Engine\Rendering\Renderers\ModelRenderer.h>
#include <Engine\Rendering\Pipeline\Graphics.h>


namespace CurryEngine::Resources
{
	static ModelRenderer g_modelRenderer;
	static bool g_isDirty = true;

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
			float width = 160.0f; // プレビューの幅を固定
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
		return changed;
	}
	nlohmann::json ModelImportSettingsDrawer::GetDefaultSettings() const
	{
		// デフォルトコンストラクタの値がそのままデフォルト設定になるようにしているため、特に値を指定せずに返す
		return ModelImportSettings{};
	}
}