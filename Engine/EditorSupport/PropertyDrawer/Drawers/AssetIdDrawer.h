#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"
#include "Engine/Resources/AssetId.h"

namespace CurryEngine
{
	/**
	 * @brief AssetId 型のプロパティを描画するためのクラス。
	 */
	class AssetIdDrawer : public IPropertyDrawer
	{
	public:
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<Resources::AssetId> m_state;
	};
}
