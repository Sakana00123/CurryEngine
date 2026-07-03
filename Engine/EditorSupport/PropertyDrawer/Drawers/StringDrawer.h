#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"
#include <string>

namespace CurryEngine
{
	/**
	 * @brief string 型のプロパティを描画するためのクラス。
	 */
	class StringDrawer : public IPropertyDrawer
	{
	public:
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<std::string> m_state;
	};
}