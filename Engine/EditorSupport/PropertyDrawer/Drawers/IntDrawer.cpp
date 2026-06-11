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
		int value = std::any_cast<int>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<int>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		ImGui::DragInt("##int", &value, 1.0f, 0, 0, mixed ? "---" : "%d");

		PropertyDrawHelper::CommitEdit<int>(prop, context, m_state, value,
			[](const int& v) {
				return std::to_string(v);
			});
	}
}