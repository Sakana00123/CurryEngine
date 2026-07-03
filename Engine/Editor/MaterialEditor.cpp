#include "pch.h"
#include "MaterialEditor.h"
#include "Engine/Resources/AssetMaterial.h"
#include "Engine/Rendering/Material.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"

#include "Engine/Resources/AssetDatabase.h"


namespace CurryEngine::Editor
{
	MaterialEditor::MaterialEditor(const CurryEngine::Resources::AssetId& materialId)
	{
		// アセットデータベースからマテリアルをロード
		//m_material = CurryEngine::Resources::AssetDatabase::LoadAsset<CurryEngine::Resources::AssetMaterial>(materialId);
		m_material = ResourceManager::GetOrLoad<CurryEngine::Resources::AssetMaterial>(CurryEngine::Resources::AssetDatabase::FindMutable(materialId)->path.string());
		m_isOpen = true;
	}

#ifdef USE_IMGUI
	void MaterialEditor::DrawGUI(RenderContext* context)
	{
		if (!m_material) return;
		if (!m_isOpen) return;

		ImGui::Begin("Material Editor", &m_isOpen);

		// プレビュー
		/*ImGui::Text("Material Preview");
		{
			ImGui::Image((ImTextureID)context->GetSharedResource("MaterialPreviewSRV"), ImVec2(256, 256));
		}*/

		// マテリアルのプロパティを描画
		m_material->GetMaterial().DrawProperty();

		if (ImGui::Button("Save"))
		{
			Save();
		}

		ImGui::End();
	}
#endif // USE_IMGUI

	void MaterialEditor::Save()
	{
		if (!m_material) return;
		// マテリアルの変更を保存
		m_material->SaveToFile(m_material->GetPath());
	}
}