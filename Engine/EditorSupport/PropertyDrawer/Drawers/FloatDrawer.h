#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"

namespace CurryEngine
{
	/**
	 * @brief float 型のプロパティを描画するためのクラス。
	 */
	class FloatDrawer : public IPropertyDrawer
	{
	public:
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<float> m_state;
	};
}