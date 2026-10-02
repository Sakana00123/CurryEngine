#pragma once
struct PropertyInfo;
struct PropertyDrawContext;

namespace CurryEngine
{
	/** @brief PropertyEditor を表すクラスです。 */
	class PropertyEditor
	{
	public:
		/**
		 * @brief 描画処理を行います。
		 */
		static void DrawProperty(const PropertyInfo* prop, const PropertyDrawContext* context);
	};

}