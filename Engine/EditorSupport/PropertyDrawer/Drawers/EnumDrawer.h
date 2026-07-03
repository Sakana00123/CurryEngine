#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"

namespace CurryEngine
{
	/**
	 * @brief 列挙型のプロパティを描画するためのクラス。
	 */
	class EnumDrawer : public IPropertyDrawer
	{
	public:
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<int> m_state;
	};
}