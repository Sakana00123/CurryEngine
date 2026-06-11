#include "pch.h"
#include "QuaternionDrawer.h"
//#include "Engine/Core/Math/Quaternion.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"

namespace CurryEngine
{
	void QuaternionDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#if 0
		// targets[0] を基準にしてプロパティ値を取得
		Quaternion value = std::any_cast<Quaternion>(prop.getter(context.Primary()));
		// 複数選択されているオブジェクトのプロパティ値が混在しているかどうかを判定
		bool hasMixedValues = PropertyDrawHelper::HasMixedValues<Quaternion>(context, prop);

		// ラベルの取得
		const char* label = prop.name.c_str();
		// ツールチップの取得
		const char* tooltip = nullptr;
		if (auto* tooltipAttr = prop.GetAttribute("Tooltip")) // Tooltip 属性があれば、引数からツールチップを取得
		{
			if (!tooltipAttr->args.empty())
			{
				tooltip = tooltipAttr->args[0].c_str();
			}
		}

		IMGUI_PROPERTY_EX(label, tooltip);
		bool edited = ImGui::DragFloat3("##Quaternion", &value.x, 1.0f, 0, 0, hasMixedValues ? "---" : "%.3f");
		if (edited)
		{
			PropertyDrawHelper::ApplyToAll<Quaternion>(context, prop, value);
		}

		if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
		{
			m_state.Prev(prop.name) = value;
		}

		if (ImGui::IsItemDeactivatedAfterEdit()) /* 編集終了後にコマンドを発行 */
		{
			Quaternion newValue = value; /* 現在の値を取得 */
			Quaternion prevValue = m_state.Prev(prop.name); /* 前回の値を取得 */
			if (newValue != prevValue)
			{
				//IMGUI_PROPERTY_COMMAND_Quaternion(label, newValue, prevValue);
			}
			m_state.Prev(prop.name) = newValue; /* 前回の値を更新 */
		}
#else
		Quaternion value = std::any_cast<Quaternion>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<Quaternion>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		ImGui::DragFloat3("##Quaternion", &value.x, 1.0f, 0, 0, mixed ? "---" : "%.3f");

		PropertyDrawHelper::CommitEdit<Quaternion>(prop, context, m_state, value,
			[](const Quaternion& v) {
				return "(" + std::to_string(v.x) + ", "
					+ std::to_string(v.y) + ", "
					+ std::to_string(v.z) + ", "
					+ std::to_string(v.w) + ")";
			});
#endif // 0

	}
}
