#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"

namespace CurryEngine
{
	/**
	 * @brief int 型のプロパティを描画するためのクラス。
	 */
	class IntDrawer : public IPropertyDrawer
	{
	public:
		/**
		 * @brief 描画処理を行います。
		 */
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<int> m_state;
	};
}