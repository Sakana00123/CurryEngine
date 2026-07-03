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

		float vSpeed = 1.0f; // ドラッグの速度。必要に応じて属性から取得することもできます。
		int vMin = 0;   // 最小値。必要に応じて属性から取得することもできます。
		int vMax = 0;   // 最大値。必要に応じて属性から取得することもできます。
		const char* format = "%d"; // 表示フォーマット。必要に応じて属性から取得することもできます。

		// 属性から vSpeed、vMin、vMax、format を取得する。
		{
			const AttributeInfo* rangeAttr = prop.GetAttribute("Range");
			if (rangeAttr && rangeAttr->args.size() >= 2)
			{
				vMin = std::stoi(rangeAttr->args[0]);
				vMax = std::stoi(rangeAttr->args[1]);
			}
			const AttributeInfo* speedAttr = prop.GetAttribute("Speed");
			if (speedAttr && !speedAttr->args.empty())
			{
				vSpeed = std::stof(speedAttr->args[0]);
			}
			const AttributeInfo* formatAttr = prop.GetAttribute("Format");
			if (formatAttr && !formatAttr->args.empty())
			{
				format = formatAttr->args[0].c_str();
			}
		}

		PropertyDrawHelper::BeginPropertyLabel(prop);
		bool edited = ImGui::DragInt("##int", &value, vSpeed, vMin, vMax, mixed ? "---" : format);
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