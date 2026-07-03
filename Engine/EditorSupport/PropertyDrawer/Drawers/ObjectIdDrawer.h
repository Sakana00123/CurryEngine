#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"
#include "Engine/Core/ObjectId.h"

namespace CurryEngine
{
	/**
	 * @brief ObjectId 型のプロパティを描画するためのクラス。
	 */
	class ObjectIdDrawer : public IPropertyDrawer
	{
	public:
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<ObjectId> m_state;
	};
}