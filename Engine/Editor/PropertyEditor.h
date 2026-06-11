#pragma once
struct PropertyInfo;
struct PropertyDrawContext;

namespace CurryEngine
{
	class PropertyEditor
	{
	public:
		static void DrawProperty(const PropertyInfo* prop, const PropertyDrawContext* context);
	};

}