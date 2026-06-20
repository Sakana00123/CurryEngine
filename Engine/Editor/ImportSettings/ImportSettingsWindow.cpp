#include "pch.h"
#include "ImportSettingsWindow.h"

#include "Engine/Resources/AssetDatabase.h"
#include "Engine/Resources/AssetMeta.h"
#include <Engine\Resources\AssetMetaSerializer.h>
#include "Engine/Resources/Resource.h"
#include "ImportSettingsDrawerRegistry.h"

#define U8(x) reinterpret_cast<const char*>(u8##x)

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
		if (ImGui::BeginPopupModal("Import Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
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

			ImGui::SameLine();

			// Resetボタン（デフォルト設定に戻す）
			if (ImGui::Button("Reset To Default"))
			{
				if (IImportSettingsDrawer* drawer = ImportSettingsDrawerRegistry::Find(meta->type))
				{
					_editingSettings = drawer->GetDefaultSettings();
					_isDirty = true; // デフォルト設定はまだ保存されていないので、変更フラグを立てる
					RequestPreviewUpdate(_targetId); // デフォルト設定に基づいてプレビューを更新
				}
			}

			// 閉じるボタン
			if (ImGui::Button("Close"))
			{
				if (_isDirty)
				{
					ShowCloseConfirmDialog(); // 変更がある場合は確認ダイアログを表示
				}
				else
				{
					ImGui::CloseCurrentPopup(); // 変更なしなら即閉じ
					CloseWindow();				// ウィンドウを閉じる
				}
			}

			// 確認ダイアログ（ネストされたPopupModal）
			if (_showCloseConfirm && !ImGui::IsPopupOpen("Unsaved Changes"))
			{
				ImGui::OpenPopup("Unsaved Changes");
			}

			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Unsaved Changes", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::TextUnformatted(U8("設定が変更されています。"));
				ImGui::TextUnformatted(U8("変更を保存せずに閉じますか？"));
				ImGui::Separator();

				if (ImGui::Button(U8("変更を保存して閉じる")))
				{
					OnApply(_targetId); // 変更を保存
					ImGui::CloseCurrentPopup(); // 確認ダイアログを閉じる
					ImGui::CloseCurrentPopup(); // インポート設定ウィンドウを閉じる
					CloseWindow(); // ウィンドウを閉じる
				}
				ImGui::SameLine();
				if (ImGui::Button(U8("変更を破棄する")))
				{
					ImGui::CloseCurrentPopup(); // 確認ダイアログを閉じる
					ImGui::CloseCurrentPopup(); // インポート設定ウィンドウを閉じる
					CloseWindow(); // ウィンドウを閉じる
				}
				ImGui::SameLine();
				if (ImGui::Button(U8("キャンセル")))
				{
					ImGui::CloseCurrentPopup(); // 確認ダイアログを閉じる
					CloseConfirmDialog(); // 確認ダイアログを閉じる
				}

				ImGui::EndPopup();
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
		
		if (_previewResource)
		{
			// プレビュー用のリソースがある場合は描画する
			if (IImportSettingsDrawer* drawer = ImportSettingsDrawerRegistry::Find(meta->type))
			{
				drawer->DrawPreview(_previewResource);
			}
			else
			{
				ImGui::TextDisabled("No preview drawer for this asset type.");
			}
		}
		else
		{
			ImGui::TextDisabled("Preview not available.");
		}

#endif // USE_IMGUI

	}

	void ImportSettingsWindow::DrawSettingsFields(const AssetId& id)
	{
#ifdef USE_IMGUI
		const AssetMeta* meta = AssetDatabase::Find(id);
		if (!meta) return;

		if (IImportSettingsDrawer* drawer = ImportSettingsDrawerRegistry::Find(meta->type))
		{
			// 設定フィールドを描画し、ユーザーが変更した場合は_editingSettingsを更新
			if (drawer->DrawSettingsFields(_editingSettings, _isDirty))
			{
				RequestPreviewUpdate(id); // 設定変更に応じてプレビューを更新
			}
		}
		else
		{
			ImGui::TextDisabled("No settings drawer for this asset type.");
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

	void ImportSettingsWindow::ShowCloseConfirmDialog()
	{
		_showCloseConfirm = true;
	}

	void ImportSettingsWindow::CloseConfirmDialog()
	{
		_showCloseConfirm = false;
	}

	void ImportSettingsWindow::CloseWindow()
	{
		_isOpen = false;
		_targetId = AssetId(); // ターゲットIDをリセット
		_isDirty = false; // 変更フラグをリセット
		_showCloseConfirm = false; // 確認ダイアログフラグをリセット
	}

}