#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"
#include "Engine/Core/Math/Vector2.h"

namespace CurryEngine
{
	/**
	 * @brief Vector2 型のプロパティを描画するためのクラス。
	 */
	class Vector2Drawer : public IPropertyDrawer
	{
	public:
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<Vector2> m_state;
	};
}