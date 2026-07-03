#include "pch.h"
#include "ColorDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"


namespace CurryEngine
{
	void ColorDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		Color value = std::any_cast<Color>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<Color>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		bool edited = ImGui::ColorEdit4("##color", &value.r);
		if (edited)
		{
			// 値が変更されたときの処理。複数選択されている場合は、すべての対象に対して新しい値を適用します。
			PropertyDrawHelper::ApplyToAll<Color>(context, prop, value);
		}

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			// 値のコミット処理。ユーザーが編集を完了したときに、Undo/Redo コマンドを発行します。
			PropertyDrawHelper::CommitEdit<Color>(prop, context, m_state, value,
				[](const Color& c) {
					return "(" + std::to_string(c.r) + ", "
						+ std::to_string(c.g) + ", "
						+ std::to_string(c.b) + ", "
						+ std::to_string(c.a) + ")";
				}
			);
		}

#endif // USE_IMGUI
	}
}