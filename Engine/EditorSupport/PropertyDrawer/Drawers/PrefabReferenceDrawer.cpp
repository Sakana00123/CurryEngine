#include "pch.h"
#include "PrefabReferenceDrawer.h"
#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/Resources/AssetDatabase.h"

namespace CurryEngine
{
	void PrefabReferenceDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		std::any valueAny = prop.getter(context.Primary());
		std::string value = valueAny.has_value() ? std::any_cast<std::string>(valueAny) : "";
		bool mixed = PropertyDrawHelper::HasMixedValues<std::string>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		bool edited = false;

		ImGui::SameLine();

		CurryEngine::Resources::AssetId assetId(value);
		auto* assetMeta = CurryEngine::Resources::AssetDatabase::Find(assetId);
		std::string displayName = assetMeta ? assetMeta->path.filename().string() : (mixed ? "---" : "None");
		ImGui::Text("%s", displayName.c_str());

		ImGui::SameLine();
		// ファイル選択ボタン
		if (ImGui::Button("..."))
		{
			ImGui::OpenPopup("SelectPrefabPopup");

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
		}

		if (ImGui::IsPopupOpen("SelectPrefabPopup"))
		{
			if (ImGui::BeginPopup("SelectPrefabPopup"))
			{
				for (auto& assetMeta : Resources::AssetDatabase::FindAllByType(AssetType::Prefab))
				{
					if (ImGui::Selectable(assetMeta.path.string().c_str()))
					{
						value = assetMeta.id.id;
						edited = true;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndPopup();
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
			//[]() {
			//	// 編集開始前の状態を保存する関数。ここでは、現在の std::string 値を m_state に保存しています。
			//	return activated;
			//},
			nullptr,
			[edited]() {
				// コミットしてもいいかどうかをチェックする関数。ここでは常に true を返していますが、必要に応じて条件を追加できます。
				return edited;
			}
		);
#endif // USE_IMGUI
	}
}
