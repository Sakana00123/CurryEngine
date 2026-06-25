#include "pch.h"
#include "ImportSettingsWindow.h"

#include "Engine/Resources/AssetDatabase.h"
#include "Engine/Resources/AssetMeta.h"
#include <Engine\Resources\AssetMetaSerializer.h>
#include "Engine/Resources/Resource.h"
#include "ImportSettingsDrawerRegistry.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <Engine\Scenes\Scene.h>
#include <Engine\Scenes\SceneManager.h>

namespace
{
	static std::unordered_map<ImGuiID, float> g_map;
	float& GetAnim(ImGuiID id)
	{
		return g_map[id];
	}
}

namespace ImGui
{
	void LoadingBar(const char* label, float fraction, const ImVec2& size_arg) {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		ImDrawList* draw = window->DrawList;
		const ImVec2 pos = window->DC.CursorPos;

		// サイズの決定（デフォルトは横幅一杯、高さ12px）
		float default_width = ImGui::GetContentRegionAvail().x;
		float default_height = 12.0f;
		ImVec2 size = ImGui::CalcItemSize(size_arg, default_width, default_height);

		// テキスト（ラベル）がある場合は、テキストの高さ分だけ配置領域（bb）を広げる
		bool has_text = (label && label[0] != '\0' && !ImGui::FindRenderedTextEnd(label));
		float text_height = has_text ? (ImGui::CalcTextSize(label).y + 4.0f) : 0.0f;

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + text_height + size.y));

		// ImGuiに領域を登録してカーソルを進める
		ImGui::ItemSize(bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, id)) return;

		// 1. ラベルテキストがある場合は描画
		if (has_text) {
			draw->AddText(pos, ImGui::GetColorU32(ImGuiCol_Text), label);
		}

		// ローディングバー本体の描画基準座標
		ImVec2 bar_min(bb.Min.x, bb.Min.y + text_height);
		ImVec2 bar_max(bb.Max.x, bb.Max.y);

#if 1
		// 2. 滑らかな進捗アニメーション (Lerp)
		float& anim_fraction = GetAnim(id);
		anim_fraction = ImLerp(anim_fraction, ImClamp(fraction, 0.0f, 1.0f), g.IO.DeltaTime * 14.0f);
#else
		float anim_fraction = ImClamp(fraction, 0.0f, 1.0f);
#endif // 0


		// 3. 背景（トラック）の描画
		ImU32 col_bg = ImGui::GetColorU32(ImGuiCol_FrameBg);
		float rounding = size.y * 0.5f; // 完全な丸角（カプセル型）にする
		draw->AddRectFilled(bar_min, bar_max, col_bg, rounding);

		// 4. 進捗バー（アクティブ部分）と動く斜線パターンの描画
		if (anim_fraction > 0.001f) {
			float progress_w = size.x * anim_fraction;
			ImVec2 progress_max(bar_min.x + progress_w, bar_max.y);
			ImU32 col_accent = ImGui::GetColorU32(ImGuiCol_SliderGrab); // テーマのメイン色

			// 進捗バーのベース（土台）を丸角で描画
			draw->AddRectFilled(bar_min, progress_max, col_accent, rounding);

			// ★ 修正点: 存在しない PathClip の代わりに標準の四角形クリッピングを適用
			// これで左右の限界を安全に制限し、ループ内の計算をシンプルにします
			draw->PushClipRect(bar_min, progress_max, true);

			// アニメーション用の時間オフセット
			float time = (float)g.Time;
			float speed = 40.0f;
			float stripe_spacing = 16.0f; // 斜線の間隔
			float stripe_width = 8.0f;    // 斜線の太さ

			float offset = ImFmod(time * speed, stripe_spacing);
			ImU32 col_stripe = IM_COL32(255, 255, 255, 35); // 白の不透明度を少し調整（約14%）

			// 斜線を描画
			float start_x = bar_min.x - size.y;
			float end_x = progress_max.x + size.y;

			for (float x = start_x + offset; x < end_x; x += stripe_spacing) {
				ImVec2 p_tl(x, bar_min.y);
				ImVec2 p_tr(x + stripe_width, bar_min.y);
				ImVec2 p_br(x + stripe_width - size.y, bar_max.y);
				ImVec2 p_bl(x - size.y, bar_max.y);

				draw->AddQuadFilled(p_tl, p_tr, p_br, p_bl, col_stripe);
			}

			// 四角形クリッピングを解除
			draw->PopClipRect();

			// ★ 修正点（カプセル型マスク処理）: 
			// 四角形クリッピングだけだと「両端の丸角部分」から斜線がハミ出て不自然になるため、
			// バーの一番外側の丸角を補正します。
			// 進捗バーが完全に満タン（1.0）でない場合、右端の丸みを綺麗にマスクするために
			// トラックの背景色と同じ色で外側を再レンダリングするか、
			// アンチエイリアスが崩れないように内側にだけ斜線を収める処理を自動で行います。
			if (anim_fraction < 0.99f && progress_w > rounding) {
				// 進捗中の右端部分の丸みを綺麗に補正するため、
				// アクセントカラーの「右側半分だけの丸角」を上から再描画して斜線のはみ出しを綺麗に上書き隠蔽します。
				draw->PushClipRect(ImVec2(progress_max.x - rounding, bar_min.y), progress_max, true);
				draw->AddRectFilled(bar_min, progress_max, col_accent, rounding);
				draw->PopClipRect();
			}

			// 左端の丸みからはみ出た斜線を隠すマスク
			if (progress_w > rounding) {
				draw->PushClipRect(bar_min, ImVec2(bar_min.x + rounding, bar_max.y), true);
				draw->AddRectFilled(bar_min, progress_max, col_accent, rounding);
				draw->PopClipRect();
			}
		}
	}
}


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

	void ImportSettingsWindow::Render3DPreview(RenderContext* context)
	{
		const AssetMeta* meta = AssetDatabase::Find(_targetId);
		if (meta && _isOpen && _previewResource)
		{
			if (IImportSettingsDrawer* drawer = ImportSettingsDrawerRegistry::Find(meta->type))
			{
				drawer->Draw3DPreview(_previewResource, context);
			}
		}
	}

	void ImportSettingsWindow::DrawGUI(RenderContext* context)
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
				DrawPreview(_targetId, context);

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
				RequestPreviewUpdate(_targetId); // Revert後の設定に基づいてプレビューを更新
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

			// 読み込み中のローディング表示
			if (_isPreviewUpdating && !ImGui::IsPopupOpen("##Loading Preview"))
			{
				_previewProgress = 0.0f; // プログレスバーをリセット
				g_map.clear(); // アニメーションの状態をリセット
				ImGui::OpenPopup("##Loading Preview");
			}
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("##Loading Preview", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				std::string statusText = _isPreviewLoadCancelled ? U8("プレビューの読み込みをキャンセルしています...") : std::string(U8("プレビューを読み込んでいます...")) + std::to_string(static_cast<int>(_previewProgress * 100)) + "%";
				ImGui::TextUnformatted(statusText.c_str());

				ImGui::LoadingBar("Loading Preview", _previewProgress, ImVec2(300, 20));

				ImGui::BeginDisabled(_isPreviewLoadCancelled);
				if (ImGui::Button(U8("キャンセル")))
				{
					// プレビューの更新をキャンセルするためのフラグを立てる
					_isPreviewLoadCancelled = true;
				}
				ImGui::EndDisabled();

				ImGui::EndPopup();
			}
			if (!_isPreviewUpdating && ImGui::IsPopupOpen("##Loading Preview"))
			{
				_isPreviewLoadCancelled = false; // キャンセルフラグをリセット
				ImGui::CloseCurrentPopup();
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

		// プレビューカメラの初期化
		if (Scene* currentScene = SceneManager::GetCurrentScene())
		{
			currentScene->GetPreviewEditorCamera()->Initialize();
			currentScene->GetPreviewEditorCamera()->SetPosition(Vector3(0, 0, 0)); // 適切な初期位置に設定
		}
	}

	void ImportSettingsWindow::DrawPreview(const AssetId& id, RenderContext* context)
	{
#ifdef USE_IMGUI
		const AssetMeta* meta = AssetDatabase::Find(id);
		if (!meta) return;
		
		if (_previewResource)
		{
			// プレビュー用のリソースがある場合は描画する
			if (IImportSettingsDrawer* drawer = ImportSettingsDrawerRegistry::Find(meta->type))
			{
				drawer->DrawPreview(_previewResource, context);
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
		_previewThread = std::thread([&]() {
			std::lock_guard<std::mutex> lock(_previewMutex);
			UpdatePreview(id);
			_isPreviewUpdating = false;
			});
		_previewThread.detach();
		_isPreviewUpdating = true;
		//UpdatePreview(id);
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
			_previewResource = nullptr; // 既存のプレビューリソースを解放
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
		const AssetMeta* meta = AssetDatabase::Find(_targetId);
		if (meta)
		{
			// ウィンドウを閉じるときに、描画クラスの状態をリセットする。
			if (IImportSettingsDrawer* drawer = ImportSettingsDrawerRegistry::Find(meta->type))
			{
				drawer->Reset(); // 描画クラスの状態をリセットして、プレビューリソースのクリーンアップなどを行う
			}
		}
		_previewResource = nullptr; // プレビューリソースを解放
		_isOpen = false;
		_targetId = AssetId(); // ターゲットIDをリセット
		_isDirty = false; // 変更フラグをリセット
		_showCloseConfirm = false; // 確認ダイアログフラグをリセット
	}

}