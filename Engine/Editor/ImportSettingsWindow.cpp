#include "pch.h"
#include "ImportSettingsWindow.h"

#include "Engine/Resources/AssetDatabase.h"
#include "Engine/Resources/AssetMeta.h"
#include <Engine\Resources\AssetMetaSerializer.h>
#include <Engine\Resources\ImportSettings\TextureImportSettings.h>
#include <Engine\Resources\Texture.h>
#include "Engine/Resources/Resource.h"

namespace CurryEngine::Resources
{
	void ImportSettingsWindow::Open(const AssetId& id)
	{
		OpenInternal(id, false);
	}
	void ImportSettingsWindow::OpenForNewAsset(const AssetId& id)
	{
		OpenInternal(id, true);
	}

	void ImportSettingsWindow::DrawGUI()
	{
#ifdef USE_IMGUI

		const AssetMeta* meta = AssetDatabase::Find(_targetId);
		if (!meta) { _isOpen = false; return; }

		// ポップアップがまだ開いていない場合は開く
		if (_isOpen && !ImGui::IsPopupOpen("Import Settings"))
		{
			ImGui::OpenPopup("Import Settings");
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)); // ポップアップの位置を画面中央に設定

		// ポップアップが開いているときだけ描画する
		if (ImGui::BeginPopupModal("Import Settings", &_isOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			// 上段: ファイル名とアセット種別
			ImGui::TextUnformatted(std::filesystem::path(meta->path).filename().string().c_str());
			ImGui::SameLine();
			ImGui::TextDisabled(AssetMetaSerializer::AssetTypeToString(meta->type).c_str());
			ImGui::Separator();

			// 中段: 左にプレビュー、右に設定フィールド（BeginTableで2カラム）
			if (ImGui::BeginTable("ImportSettingsLayout", 2, ImGuiTableFlags_Resizable))
			{
				ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthFixed, 200.0f);
				ImGui::TableSetupColumn("Settings");
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				DrawPreview(_targetId);

				ImGui::TableSetColumnIndex(1);
				DrawSettingsFields(_targetId); // ここで_editingSettingsを直接編集

				ImGui::EndTable();
			}

			ImGui::Separator();

			// 下段: Applyボタン（_isDirtyのときだけハイライト）
			ImGui::BeginDisabled(!_isDirty);
			if (ImGui::Button("Apply"))
			{
				OnApply(_targetId);
			}
			ImGui::EndDisabled();

			ImGui::SameLine();

			// Revertボタン（.metaの現在値に戻す）
			if (ImGui::Button("Revert"))
			{
				_editingSettings = meta->importSettings;
				_isDirty = false;
			}

			ImGui::EndPopup();
		}
#endif // USE_IMGUI

	}

	bool ImportSettingsWindow::IsOpen()
	{
#ifdef USE_IMGUI
		return ImGui::IsPopupOpen("Import Settings");
#endif // USE_IMGUI
		return _isOpen;
	}

	void ImportSettingsWindow::OpenInternal(const AssetId& id, bool isNewAsset)
	{
		auto meta = AssetDatabase::Find(id);
		if (!meta)
		{
			LOG_ERROR("Failed to open import settings: Asset not found with ID " + id.ToString());
			return;
		}
		// 設定を編集するために、現在のインポート設定をコピーして保持します。
		_targetId = id;
		_editingSettings = meta->importSettings;
		_isDirty = false;
		_isOpen = true;
		// プレビューの更新をリクエスト
		RequestPreviewUpdate(id);
	}

	void ImportSettingsWindow::DrawPreview(const AssetId& id)
	{
#ifdef USE_IMGUI
		const AssetMeta* meta = AssetDatabase::Find(id);
		if (!meta) return;
		switch (meta->type)
		{
		case AssetType::Texture:
		{
			ImGui::TextUnformatted("Texture Preview");

			if (auto thumbnail = std::dynamic_pointer_cast<AssetTexture>(_previewResource))
			{
				ImGui::Image(thumbnail->GetSRV(), ImVec2(128, 128));
			}

			break;
		}
		case AssetType::Model:
			ImGui::TextUnformatted("Model Preview");
			break;
		default:
			ImGui::TextUnformatted("No Preview Available");
			break;
		}
#endif // USE_IMGUI

	}

	void ImportSettingsWindow::DrawSettingsFields(const AssetId& id)
	{
#ifdef USE_IMGUI
		const AssetMeta* meta = AssetDatabase::Find(id);
		if (!meta) return;

		switch (meta->type)
		{
		case AssetType::Texture:
		{
			auto settings = _editingSettings.is_null()
				? TextureImportSettings{}
			: _editingSettings.get<TextureImportSettings>();

			bool changed = false;
			changed |= ImGui::Checkbox("Generate Mipmaps", &settings.generateMipmaps);

			// compressionはドロップダウンで選ばせる
			const char* compressionOptions[] = { "None", "BC1", "BC3", "BC7" };
			int currentIndex = 0;
			for (int i = 0; i < IM_ARRAYSIZE(compressionOptions); ++i)
				if (settings.compression == compressionOptions[i]) { currentIndex = i; break; }

			if (ImGui::Combo("Compression", &currentIndex, compressionOptions, IM_ARRAYSIZE(compressionOptions)))
			{
				settings.compression = compressionOptions[currentIndex];
				changed = true;
			}

			if (changed)
			{
				_editingSettings = settings; // 編集バッファに書き戻す
				_isDirty = true;
				// プレビュー更新（次フレームのDrawPreviewに反映される）
				RequestPreviewUpdate(id);
			}
			break;
		}
		// 将来: case AssetType::GltfModel: ...
		default:
			ImGui::TextDisabled("No import settings for this asset type.");
			break;
		}
#endif // USE_IMGUI
	}

	void ImportSettingsWindow::OnApply(const AssetId& id)
	{
		AssetMeta* meta = AssetDatabase::FindMutable(id); // 書き込み用のconst無し版
		if (!meta) return;

		// .metaに書き込み
		meta->importSettings = _editingSettings;
		AssetMetaSerializer::Save(*meta);

		// 再インポート（ResourceManagerのキャッシュを更新）
		AssetDatabase::LoadAsset<Resource>(id); // キャッシュ更新のためにロード

		_isDirty = false;
	}

	void ImportSettingsWindow::RequestPreviewUpdate(const AssetId& id)
	{
		// 現在は即時更新。将来的には非同期でのプレビュー生成を検討。
		UpdatePreview(id);
	}

	void ImportSettingsWindow::UpdatePreview(const AssetId& id)
	{
		const AssetMeta* meta = AssetDatabase::Find(id);
		if (!meta) return;

		// 編集バッファの設定で一時的なAssetMetaを作り、プレビュー用にインポートする
		AssetMeta previewMeta = *meta;
		previewMeta.importSettings = _editingSettings;

		if (IImporter* importer = ImporterRegistry::Find(previewMeta.type))
		{
			_previewResource = importer->Import(previewMeta);
		}
	}

}