#include "pch.h"
#include "Vector2Drawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"


namespace CurryEngine
{
	void Vector2Drawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		Vector2 value = std::any_cast<Vector2>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<Vector2>(context, prop);

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

		PropertyDrawHelper::BeginPropertyLabel(prop);
		bool edited = ImGui::DragFloat2(("##" + prop.name).c_str(), &value.x, vSpeed, vMin, vMax, mixed ? "---" : format);
		if (edited)
		{
			// 値が変更されたときの処理。複数選択されている場合は、すべての対象に対して新しい値を適用します。
			PropertyDrawHelper::ApplyToAll<Vector2>(context, prop, value);
		}

		// 値のコミット処理。ユーザーが編集を完了したときに、Undo/Redo コマンドを発行します。
		PropertyDrawHelper::CommitEdit<Vector2>(prop, context, m_state, value,
			[](const Vector2& v) {
				return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
			},
			[](const Vector2& a, const Vector2& b) {
				return std::abs(a.x - b.x) < 1e-6f && std::abs(a.y - b.y) < 1e-6f; // 浮動小数点数の比較は、絶対値の差が小さいかどうかで判定
			}
		);
#endif // USE_IMGUI
	}
}