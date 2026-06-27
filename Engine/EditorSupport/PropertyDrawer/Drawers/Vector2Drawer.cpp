#include "pch.h"
#include "Vector2Drawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"
#include <imgui_internal.h>


namespace CurryEngine
{
	void Vector2Drawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		static constexpr int componentCount = 2;
		static const char* labels[componentCount] = { "X", "Y" };

		Vector2 value = std::any_cast<Vector2>(prop.getter(context.Primary()));
		int mixedFlags = PropertyDrawHelper::MixedValueComponentFlag<Vector2>(context, prop, componentCount, [](const Vector2& a, const Vector2& b, int componentIndex) {
			return std::abs(a[componentIndex] - b[componentIndex]) < 1e-6f; // 浮動小数点数の比較は、絶対値の差が小さいかどうかで判定
			});

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
		bool edited = false;
		bool itemActivated = false;
		bool deactivatedAfterEdit = false;
		{
			static constexpr int componentCount = 2;
			static const char* labels[componentCount] = { "X", "Y" };

			ImGui::PushMultiItemsWidths(componentCount, ImGui::CalcItemWidth());
			for (int i = 0; i < componentCount; ++i)
			{
				ImGui::Text(labels[i]);
				ImGui::SameLine();
				ImGui::PushID(i);
				edited |= ImGui::DragFloat(("##" + std::string(labels[i])).c_str(), &value[i], vSpeed, vMin, vMax, mixedFlags & (1 << i) ? "---" : format);
				itemActivated |= ImGui::IsItemActivated();
				deactivatedAfterEdit |= ImGui::IsItemDeactivatedAfterEdit();
				ImGui::PopID();
				ImGui::PopItemWidth();
				if (i < componentCount - 1)
				{
					ImGui::SameLine();
				}
			}
		}
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
			},
			[itemActivated]() {
				return itemActivated;
			},
			[deactivatedAfterEdit]() {
				return deactivatedAfterEdit;
			}
		);
#endif // USE_IMGUI
	}
}