#include "pch.h"
#include "EulerDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"


namespace CurryEngine
{
	void EulerDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		Quaternion value = std::any_cast<Quaternion>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<Quaternion>(context, prop);

		float vSpeed = 0.1f; // ドラッグの速度。必要に応じて属性から取得することもできます。
		float vMin = 0.0f;   // 最小値。必要に応じて属性から取得することもできます。
		float vMax = 0.0f;   // 最大値。必要に応じて属性から取得することもできます。
		const char* format = "%.3f"; // 表示フォーマット。必要に応じて属性から取得することもできます。

		// 属性から vSpeed、vMin、vMax、format を取得する。
		{
			const AttributeInfo* rangeAttr = prop.GetAttribute("Range");
			if (rangeAttr && rangeAttr->args.size() >= 2)
			{
				vMin = std::stof(rangeAttr->args[0]);
				vMax = std::stof(rangeAttr->args[1]);
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


		Vector3& euler = m_eulerState.Prev(prop.name);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		// ここで、ImGui を使用してプロパティの編集 UI を描画します。
		bool edited = ImGui::DragFloat3("##Euler", &euler.x, vSpeed, vMin, vMax, mixed ? "---" : format);
		if (!ImGui::IsItemActive())
		{
			m_eulerState.Prev(prop.name) = value.ToEuler();
		}
		if (edited)
		{
			// 値が変更されたときの処理。複数選択されている場合は、すべての対象に対して新しい値を適用します。
			Quaternion newQuat = Quaternion::FromEuler(euler);
			PropertyDrawHelper::ApplyToAll<Quaternion>(context, prop, newQuat);
		}

		// 値のコミット処理。ユーザーが編集を完了したときに、Undo/Redo コマンドを発行します。
		Quaternion currentValue = Quaternion::FromEuler(euler);
		PropertyDrawHelper::CommitEdit<Quaternion>(prop, context, m_prevQuaternionState, currentValue,
			[](const Quaternion& v) {
				Vector3 euler = v.ToEuler();
				return "(" + std::to_string(euler.x) + ", "
					+ std::to_string(euler.y) + ", "
					+ std::to_string(euler.z) + ")";
			},
			[](const Quaternion& a, const Quaternion& b) {
				return Quaternion::NearEqual(a, b);
			}
		);

#endif // USE_IMGUI
	}
}