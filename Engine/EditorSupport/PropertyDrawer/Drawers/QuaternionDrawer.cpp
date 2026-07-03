#include "pch.h"
#include "QuaternionDrawer.h"
//#include "Engine/Core/Math/Quaternion.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"
#include <imgui_internal.h>

namespace CurryEngine
{
	void QuaternionDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		static constexpr int componentCount = 4;
		static const char* labels[componentCount] = { "X", "Y", "Z", "W" };

		Quaternion value = std::any_cast<Quaternion>(prop.getter(context.Primary()));
		//bool mixed = PropertyDrawHelper::HasMixedValues<Quaternion>(context, prop);
		int mixedFlags = PropertyDrawHelper::MixedValueComponentFlag<Quaternion>(context, prop, componentCount, [](const Quaternion& a, const Quaternion& b, int componentIndex) {
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
		//bool edited = ImGui::DragFloat4("##Quaternion", &value.x, vSpeed, vMin, vMax, mixed ? "---" : format);
		bool edited = false;
		bool itemActivated = false;
		bool deactivatedAfterEdit = false;
		{
			ImGui::PushMultiItemsWidths(componentCount, ImGui::CalcItemWidth());
			for (int i = 0; i < componentCount; ++i)
			{
				ImGui::Text(labels[i]);
				ImGui::SameLine();
				ImGui::PushID(i);
				edited |= ImGui::DragFloat("##value", &value[i], vSpeed, vMin, vMax, mixedFlags & (1 << i) ? "---" : format);
				itemActivated |= ImGui::IsItemActivated();
				deactivatedAfterEdit |= ImGui::IsItemDeactivatedAfterEdit();
				ImGui::PopID();
				if (i < componentCount - 1)
					ImGui::SameLine();
				ImGui::PopItemWidth();
			}
		}
		if (edited)
		{
			// 値が変更されたときの処理。複数選択されている場合は、すべての対象に対して新しい値を適用します。
			PropertyDrawHelper::ApplyToAll<Quaternion>(context, prop, value);
		}

		// 値のコミット処理。ユーザーが編集を完了したときに、Undo/Redo コマンドを発行します
		PropertyDrawHelper::CommitEdit<Quaternion>(prop, context, m_state, value,
			[](const Quaternion& v) {
				return "(" + std::to_string(v.x) + ", "
					+ std::to_string(v.y) + ", "
					+ std::to_string(v.z) + ", "
					+ std::to_string(v.w) + ")";
			},
			[](const Quaternion& a, const Quaternion& b) {
				return Quaternion::NearEqual(a, b);
			},
			[itemActivated]() {
				// 前フレームの値を保存するかどうかをチェックする関数。
				return itemActivated;
			},
			[deactivatedAfterEdit]() {
				// コミットしてもいいかどうかをチェックする関数。
				return deactivatedAfterEdit;
			}
		);


#endif // USE_IMGUI
	}
}
