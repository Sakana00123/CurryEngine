#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"
#include "Engine/Core/Color.h"

namespace CurryEngine
{
	/**
	 * @brief Color 型のプロパティを描画するためのクラス。
	 */
	class ColorDrawer : public IPropertyDrawer
	{
	public:
		/**
		 * @brief 描画処理を行います。
		 */
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<Color> m_state;
	};
}