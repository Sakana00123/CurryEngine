#include "pch.h"
#include "FbxImportSettingsDrawer.h"
#include "Engine/Resources/ImportSettings/FbxImportSettings.h"
#include "Engine/Resources/SkinnedMesh.h"
#include "Engine/Rendering/Pipeline/Graphics.h"


namespace CurryEngine::Resources
{

	void FbxImportSettingsDrawer::Draw3DPreview(const std::shared_ptr<Resource>& previewResource, RenderContext* context)
	{
		auto skinnedMesh = std::dynamic_pointer_cast<SkinnedMesh>(previewResource);
		// SkinnedMeshの描画処理をここに実装する
		DirectX::XMFLOAT4X4 identityMatrix{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
		skinnedMesh->Render(context->immediateContext, identityMatrix, Color::White, nullptr);
	}
	void FbxImportSettingsDrawer::DrawPreview(const std::shared_ptr<Resource>& previewResource, RenderContext* context)
	{
		auto skinnedMesh = std::dynamic_pointer_cast<SkinnedMesh>(previewResource);
		if (!skinnedMesh) { ImGui::TextDisabled("No preview available."); return; }
		ImGui::Text("FBX Preview: %s", skinnedMesh->GetPath().c_str());

		// プレビュー描画
		if (auto previewImage = static_cast<RenderTexture*>(context->GetSharedResource("PreRenderTexture")))
		{
			// 16:9 のアスペクト比でプレビューを描画するためのサイズを計算
			float width = 720.0f; // プレビューの幅を固定
			ImVec2 size = { width, width * 9.0f / 16.0f };
			ImGui::Image(previewImage->GetSRV(), size);
		}

		ImGui::Separator();
		ImGui::Text("Meshes: %zu", skinnedMesh->meshes.size());
		ImGui::Text("Animations: %zu", skinnedMesh->animationClips.size());
		ImGui::Text("Materials: %zu", skinnedMesh->materials.size());

		int boneCount = 0;
		for (const auto& mesh : skinnedMesh->meshes)
		{
			boneCount += static_cast<int>(mesh.bindPose.bones.size());
		}
		ImGui::Text("Bones: %d", boneCount);


	}
	bool FbxImportSettingsDrawer::DrawSettingsFields(nlohmann::json& settings, bool& isDirty)
	{
		FbxImportSettings importSettings = settings.get<FbxImportSettings>();
		bool changed = false;
		if (ImGui::CollapsingHeader("FBX Import Settings"))
		{
			/*if (ImGui::Checkbox("Import Animations", &importSettings.importAnimations))
			{
				isDirty = true;
				changed = true;
			}
			if (ImGui::Checkbox("Import Materials", &importSettings.importMaterials))
			{
				isDirty = true;
				changed = true;
			}
			if (ImGui::Checkbox("Import Textures", &importSettings.importTextures))
			{
				isDirty = true;
				changed = true;
			}
			if (ImGui::Checkbox("Import Cameras", &importSettings.importCameras))
			{
				isDirty = true;
				changed = true;
			}
			if (ImGui::Checkbox("Import Lights", &importSettings.importLights))
			{
				isDirty = true;
				changed = true;
			}
			if (ImGui::Checkbox("Flip UVs", &importSettings.flipUVs))
			{
				isDirty = true;
				changed = true;
			}*/
		}

		if (changed)
		{
			settings = importSettings;
		}

		return changed;
	}
	nlohmann::json FbxImportSettingsDrawer::GetDefaultSettings() const
	{
		return FbxImportSettings{};
	}
}
