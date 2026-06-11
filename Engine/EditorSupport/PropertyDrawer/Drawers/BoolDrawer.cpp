#include "pch.h"
#include "BoolDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"


namespace CurryEngine
{
	void BoolDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
		bool value = std::any_cast<bool>(prop.getter(context.Primary()));
		int v = value ? 1 : 0;
		bool mixed = PropertyDrawHelper::HasMixedValues<bool>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		bool edited = false;
		if (mixed)
		{
			// 値が混在している場合は、フラグを -1 にして表示をMixedにする。そうでない場合は、値に応じてフラグを 1 または 0 にする。
			if (ImGui::CheckboxFlags("##bool", &v, -1))
			{
				value = !value;
				edited = true;
			}
		}
		else
		{
			edited = ImGui::Checkbox("##bool", &value);
		}

		if (edited)
		{
			PropertyDrawHelper::ApplyToAll<bool>(context, prop, value);

			PropertyDrawHelper::CommitEdit<bool>(prop, context, m_state, value,
				[](const bool& v) {
					return v ? "True" : "False";
				});
		}
	}
}