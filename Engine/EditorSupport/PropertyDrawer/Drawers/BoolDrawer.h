#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"

namespace CurryEngine
{
	/**
	 * @brief bool 型のプロパティを描画するためのクラス。
	 */
	class BoolDrawer : public IPropertyDrawer
	{
	public:
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<bool> m_state;
	};
}