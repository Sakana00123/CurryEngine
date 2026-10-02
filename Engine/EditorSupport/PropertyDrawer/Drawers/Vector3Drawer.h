#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"
struct Vector3;

namespace CurryEngine
{
	/**
	 * @brief int 型のプロパティを描画するためのクラス。
	 */
	class Vector3Drawer : public IPropertyDrawer
	{
	public:
		/**
		 * @brief 描画処理を行います。
		 */
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<Vector3> m_state;
	};
}