#include "pch.h"
#include "PropertyDrawHelper.h"
#include <Engine\EditorSupport\ImGuiHelpers.h>

namespace CurryEngine
{
	namespace PropertyDrawHelper
	{
		void BeginPropertyLabel(const PropertyInfo& prop)
		{
#ifdef USE_IMGUI
			const char* label = prop.name.c_str();
			const char* tooltip = nullptr;
			if (auto* tooltipAttr = prop.GetAttribute("Tooltip")) // Tooltip 属性があれば、引数からツールチップを取得
			{
				if (!tooltipAttr->args.empty())
				{
					tooltip = tooltipAttr->args[0].c_str();
				}
			}
			IMGUI_PROPERTY_EX(label, tooltip);
#endif // USE_IMGUI
		}
		
	}
}