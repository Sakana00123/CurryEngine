#include "pch.h"
#include "StringDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"


namespace CurryEngine
{
	void StringDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
		std::string value = std::any_cast<std::string>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<std::string>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		
		char buffer[256];
		if (!ImGui::IsItemEdited())
		{
			strncpy_s(buffer, mixed ? "---" : value.c_str(), sizeof(buffer));
			buffer[sizeof(buffer) - 1] = '\0'; // バッファの最後を null で終端
		}
		
		bool edited = ImGui::InputText("##string", buffer, sizeof(buffer));

		PropertyDrawHelper::CommitEdit<std::string>(prop, context, m_state, std::string(buffer),
			[](const std::string& v) {
				return v;
			});
	}
}