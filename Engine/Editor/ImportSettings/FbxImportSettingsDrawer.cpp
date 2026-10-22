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
		if (!skinnedMesh) { ImGui::TextDisabled(reinterpret_cast<const char*>(u8"プレビューはありません。")); return; }
		ImGui::Text(reinterpret_cast<const char*>(u8"FBXプレビュー: %s"), skinnedMesh->GetPath().c_str());

		// プレビュー描画
		if (auto previewImage = static_cast<RenderTexture*>(context->GetSharedResource("PreRenderTexture")))
		{
			// 16:9 のアスペクト比でプレビューを描画するためのサイズを計算
			float width = 720.0f; // プレビューの幅を固定
			ImVec2 size = { width, width * 9.0f / 16.0f };
			ImGui::Image(previewImage->GetSRV(), size);
		}

		ImGui::Separator();
		ImGui::Text(reinterpret_cast<const char*>(u8"メッシュ: %zu 個"), skinnedMesh->meshes.size());
		ImGui::Text(reinterpret_cast<const char*>(u8"アニメーション: %zu 件"), skinnedMesh->animationClips.size());
		ImGui::Text(reinterpret_cast<const char*>(u8"マテリアル: %zu 件"), skinnedMesh->materials.size());

		int boneCount = 0;
		for (const auto& mesh : skinnedMesh->meshes)
		{
			boneCount += static_cast<int>(mesh.bindPose.bones.size());
		}
		ImGui::Text(reinterpret_cast<const char*>(u8"ボーン: %d 本"), boneCount);


	}
	bool FbxImportSettingsDrawer::DrawSettingsFields(nlohmann::json& settings, bool& isDirty)
	{
		FbxImportSettings importSettings = settings.get<FbxImportSettings>();
		bool changed = false;
		if (ImGui::CollapsingHeader(reinterpret_cast<const char*>(u8"FBXインポート設定")))
		{
			/*if (ImGui::Checkbox(reinterpret_cast<const char*>(u8"アニメーションを取り込む"), &importSettings.importAnimations))
			{
				isDirty = true;
				changed = true;
			}
			if (ImGui::Checkbox(reinterpret_cast<const char*>(u8"マテリアルを取り込む"), &importSettings.importMaterials))
			{
				isDirty = true;
				changed = true;
			}
			if (ImGui::Checkbox(reinterpret_cast<const char*>(u8"テクスチャを取り込む"), &importSettings.importTextures))
			{
				isDirty = true;
				changed = true;
			}
			if (ImGui::Checkbox(reinterpret_cast<const char*>(u8"カメラを取り込む"), &importSettings.importCameras))
			{
				isDirty = true;
				changed = true;
			}
			if (ImGui::Checkbox(reinterpret_cast<const char*>(u8"ライトを取り込む"), &importSettings.importLights))
			{
				isDirty = true;
				changed = true;
			}
			if (ImGui::Checkbox(reinterpret_cast<const char*>(u8"UVを反転"), &importSettings.flipUVs))
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
