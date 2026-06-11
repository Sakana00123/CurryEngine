#include "pch.h"
#include "FloatDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"

namespace CurryEngine
{
	void FloatDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
        float value = std::any_cast<float>(prop.getter(context.Primary()));
        bool mixed = PropertyDrawHelper::HasMixedValues<float>(context, prop);

        PropertyDrawHelper::BeginPropertyLabel(prop);
        ImGui::DragFloat("##float", &value, 1.0f, 0, 0, mixed ? "---" : "%.3f");

        PropertyDrawHelper::CommitEdit<float>(prop, context, m_state, value,
            [](const float& v) {
				return std::to_string(v);
            });
	}
}
