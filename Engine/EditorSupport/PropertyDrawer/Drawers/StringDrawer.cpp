#include "pch.h"
#include "StringDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"


namespace CurryEngine
{
	void StringDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		std::string value = std::any_cast<std::string>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<std::string>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);

		char buffer[256];
		strncpy_s(buffer, mixed ? "---" : value.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0'; // バッファの最後を null で終端
		bool edited = ImGui::InputText("##string", buffer, sizeof(buffer));
		if (edited)
		{
			// 値が変更されたときの処理。複数選択されている場合は、すべての対象に対して新しい値を適用します。
			PropertyDrawHelper::ApplyToAll<std::string>(context, prop, std::string(buffer));
		}

		// 値のコミット処理。ユーザーが編集を完了したときに、Undo/Redo コマンドを発行します。
		PropertyDrawHelper::CommitEdit<std::string>(prop, context, m_state, std::string(buffer),
			[](const std::string& v) {
				return v;
			},
			[](const std::string& a, const std::string& b) {
				return a == b;
			},
			[&]() {
				// 編集開始前の状態を保存する関数。ここでは、現在の std::string 値を m_state に保存しています。
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