#include "pch.h"
#include "ModelImportSettingsDrawer.h"
#include <Engine\Resources\ImportSettings\ModelImportSettings.h>
#include <Engine\Resources\ModelAsset.h>


namespace CurryEngine::Resources
{
	void ModelImportSettingsDrawer::DrawPreview(const std::shared_ptr<Resource>& previewResource)
	{
		auto model = std::dynamic_pointer_cast<ModelAsset>(previewResource);
		if (!model) { ImGui::TextDisabled("No preview available."); return; }
		// モデルのプレビュー描画は、ここでは簡略化してテキスト表示のみとする
		ImGui::Text("Model Preview: %s", model->GetPath().c_str());
		ImGui::Separator();
		ImGui::Text("Scenes: %zu", model->scenes.size());
		ImGui::Text("Nodes: %zu", model->nodes.size());
		ImGui::Text("Meshes: %zu", model->meshes.size());
	}
	bool ModelImportSettingsDrawer::DrawSettingsFields(nlohmann::json& editingSettings, bool& isDirty)
	{
		auto settings = editingSettings.is_null()
			? ModelImportSettings{}
			: editingSettings.get<ModelImportSettings>();
		bool changed = false;
		changed |= ImGui::InputFloat("Scale Factor", &settings.scaleFactor);
		changed |= ImGui::Checkbox("Static Batching", &settings.staticBatching);
		if (changed)
		{
			editingSettings = settings;
			isDirty = true;
		}
		return changed;
	}
	nlohmann::json ModelImportSettingsDrawer::GetDefaultSettings() const
	{
		// デフォルトコンストラクタの値がそのままデフォルト設定になるようにしているため、特に値を指定せずに返す
		return ModelImportSettings{};
	}
}