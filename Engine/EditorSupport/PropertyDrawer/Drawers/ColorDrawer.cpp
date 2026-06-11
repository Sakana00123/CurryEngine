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
		Color value = std::any_cast<Color>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<Color>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		if (ImGui::ColorEdit4("##color", &value.r))
		{
			PropertyDrawHelper::ApplyToAll<Color>(context, prop, value);
		}

		PropertyDrawHelper::CommitEdit<Color>(prop, context, m_state, value,
			[](const Color& c) {
				return "(" + std::to_string(c.r) + ", "
					+ std::to_string(c.g) + ", "
					+ std::to_string(c.b) + ", "
					+ std::to_string(c.a) + ")";
			});
	}
}