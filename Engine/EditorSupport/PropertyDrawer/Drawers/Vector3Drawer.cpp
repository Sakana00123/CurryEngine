#include "pch.h"
#include "Vector3Drawer.h"
#include "Engine/Core/Math/Vector3.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"

namespace CurryEngine
{
	void Vector3Drawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
		// targets[0] を基準にしてプロパティ値を取得
		Vector3 value = std::any_cast<Vector3>(prop.getter(context.Primary()));
		// 複数選択されているオブジェクトのプロパティ値が混在しているかどうかを判定
		bool hasMixedValues = PropertyDrawHelper::HasMixedValues<Vector3>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		bool edited = ImGui::DragFloat3("##Vector3", &value.x, 1.0f, 0, 0, hasMixedValues ? "---" : "%.3f");
		if (edited)
		{
			PropertyDrawHelper::ApplyToAll<Vector3>(context, prop, value);
		}

		PropertyDrawHelper::CommitEdit<Vector3>(prop, context, m_state, value,
			[](const Vector3& v) {
				return "(" + std::to_string(v.x) + ", "
					+ std::to_string(v.y) + ", "
					+ std::to_string(v.z) + ")";
			});
	}
}
