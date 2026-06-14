#include "pch.h"
#include "AssetReferenceDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"
#include <Engine\Editor\Dialog.h>


namespace CurryEngine
{
	void AssetReferenceDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		std::string value = std::any_cast<std::string>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<std::string>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);

		char buffer[256];
		strncpy_s(buffer, mixed ? "---" : value.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0'; // バッファの最後を null で終端
		bool edited = ImGui::InputText("##string", buffer, sizeof(buffer));
		bool activated = ImGui::IsItemActivated();
		bool deactivatedAfterEdit = ImGui::IsItemDeactivatedAfterEdit();
		if (edited)
		{
			value = std::string(buffer);
		}

		ImGui::SameLine();
		// ファイル選択ボタン
		if (ImGui::Button("..."))
		{
			// ダイアログを開いてエフェクトデータを選択
			char filename[256]{};
			char filter[MAX_PATH] = "All Files (*.*)|*.*|"; // フィルタの例。必要に応じて変更してください。

			if (auto* dialogFilterAttr = prop.GetAttribute("DialogFilter"))
			{
				// TODO: うまく行ってないのでデバッグすること。args[0] にフィルタ文字列が入っている想定。
				if (!dialogFilterAttr->args.empty())
				{
					std::string raw = dialogFilterAttr->args[0];
					std::replace(raw.begin(), raw.end(), '|', '\0');
					raw += '\0'; // 末尾の追加 \0

					memcpy(filter, raw.data(), (std::min)(raw.size(), sizeof(filter) - 1));

					//strncpy_s(filter, dialogFilterAttr->args[0].data(), sizeof(filter));
					//filter[sizeof(filter) - 1] = '\0'; // バッファの最後を null で終端
				}
			}

			PropertyDrawHelper::CommitEdit<std::string>(prop, context, m_state, value,
				[](const std::string& v) {
					return v;
				},
				[](const std::string& a, const std::string& b) {
					return a == b;
				},
				[]() {
					// 編集開始前の状態を保存する関数。ここでは、現在の std::string 値を m_state に保存しています。
					return true;
				},
				[]() {
					// コミットしてもいいかどうかをチェックする関数。ここでは常に true を返していますが、必要に応じて条件を追加できます。
					return false;
				}
			);

			if (Dialog::OpenFileName(filename, sizeof(filename), filter) == DialogResult::OK)
			{
				value = filename;
				edited = true;
				activated = false; // ダイアログで選択したときは、InputText の編集状態を無効にする
				deactivatedAfterEdit = true; // ダイアログで選択した後は、即座にコミットするためにフラグを立てる
			}
		}


		if (edited)
		{
			// 値が変更されたときの処理。複数選択されている場合は、すべての対象に対して新しい値を適用します。
			PropertyDrawHelper::ApplyToAll<std::string>(context, prop, value);
		}

		// 値のコミット処理。ユーザーが編集を完了したときに、Undo/Redo コマンドを発行します。
		PropertyDrawHelper::CommitEdit<std::string>(prop, context, m_state, value,
			[](const std::string& v) {
				return v;
			},
			[](const std::string& a, const std::string& b) {
				return a == b;
			},
			[activated]() {
				// 編集開始前の状態を保存する関数。ここでは、現在の std::string 値を m_state に保存しています。
				return activated;
			},
			[deactivatedAfterEdit]() {
				// コミットしてもいいかどうかをチェックする関数。ここでは常に true を返していますが、必要に応じて条件を追加できます。
				return deactivatedAfterEdit;
			}
		);
#endif // USE_IMGUI

	}
}