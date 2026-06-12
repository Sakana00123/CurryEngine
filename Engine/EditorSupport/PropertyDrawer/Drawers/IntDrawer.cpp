#include "pch.h"
#include "IntDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"


namespace CurryEngine
{
	void IntDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		int value = std::any_cast<int>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<int>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		bool edited = ImGui::DragInt("##int", &value, 1.0f, 0, 0, mixed ? "---" : "%d");
		if (edited)
		{
			// 値が変更されたときの処理。複数選択されている場合は、すべての対象に対して新しい値を適用します。
			PropertyDrawHelper::ApplyToAll<int>(context, prop, value);
		}

		// 値のコミット処理。ユーザーが編集を完了したときに、Undo/Redo コマンドを発行します。
		PropertyDrawHelper::CommitEdit<int>(prop, context, m_state, value,
			[](const int& v) {
				return std::to_string(v);
			},
			[](const int& a, const int& b) {
				return a == b;
			},
			[&]() {
				// 編集開始前の状態を保存する関数。ここでは、現在の int 値を m_state に保存しています。
				return ImGui::IsItemActivated();
			},
			[&]() {
				// コミットしてもいいかどうかをチェックする関数。ここでは常に true を返していますが、必要に応じて条件を追加できます。
				return ImGui::IsItemDeactivatedAfterEdit();
			}
		);

#endif // USE_IMGUI
	}
}