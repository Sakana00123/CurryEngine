#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"

namespace CurryEngine
{
	/**
	 * @brief PrefabReference 型のプロパティを描画するためのクラス。
	 */
	class PrefabReferenceDrawer : public IPropertyDrawer
	{
	public:
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<std::string> m_state;
	};
}
