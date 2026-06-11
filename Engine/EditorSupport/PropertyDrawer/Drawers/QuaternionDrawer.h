#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"
#include "Engine/Core/Math/Quaternion.h"

namespace CurryEngine
{
	/**
	 * @brief Quaternion 型のプロパティを描画するためのクラス。
	 */
	class QuaternionDrawer : public IPropertyDrawer
	{
	public:
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<Quaternion> m_state;
	};
}