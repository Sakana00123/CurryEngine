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
		// QuaternionDrawer::Draw の先頭に一時的に追加
		{
			Quaternion value = std::any_cast<Quaternion>(prop.getter(context.Primary()));
			Vector3 euler = value.ToEuler();
			Quaternion restored = Quaternion::FromEuler(euler);
			/*LOG_INFO(std::format("original: ({}, {}, {}, {})", value.x, value.y, value.z, value.w));
			LOG_INFO(std::format("restored: ({}, {}, {}, {})", restored.x, restored.y, restored.z, restored.w));*/
			if (value != restored)
			{
				LOG_WARNING(std::format("QuaternionDrawer: Quaternion value cannot be accurately represented as Euler angles. Original: ({}, {}, {}, {}), Restored: ({}, {}, {}, {})",
					value.x, value.y, value.z, value.w,
					restored.x, restored.y, restored.z, restored.w));
			}
		}

		Quaternion value = std::any_cast<Quaternion>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<Quaternion>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		bool edited = ImGui::DragFloat4("##Quaternion", &value.x, 1.0f, 0, 0, mixed ? "---" : "%.3f");
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
			});
	}
}
