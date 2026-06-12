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
		Vector2 value = std::any_cast<Vector2>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<Vector2>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		bool edited = ImGui::DragFloat2(("##" + prop.name).c_str(), &value.x, 0.1f, 0.0f, 0.0f, mixed ? "---" : "%.3f");
		if (edited)
		{
			// 値が変更されたときの処理。複数選択されている場合は、すべての対象に対して新しい値を適用します。
			PropertyDrawHelper::ApplyToAll<Vector2>(context, prop, value);
		}

		// 値のコミット処理。ユーザーが編集を完了したときに、Undo/Redo コマンドを発行します。
		PropertyDrawHelper::CommitEdit<Vector2>(prop, context, m_state, value,
			[](const Vector2& v) {
				return "Vector2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
			});
	}
}