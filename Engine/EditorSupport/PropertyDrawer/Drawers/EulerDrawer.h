#pragma once
#include "../IPropertyDrawer.h"
#include "../DrawerState.h"
#include "Engine/Core/Math/Vector3.h"
#include "Engine/Core/Math/Quaternion.h"

namespace CurryEngine
{
	/**
	 * @brief Euler 型のプロパティを描画するためのクラス。
	 */
	class EulerDrawer : public IPropertyDrawer
	{
	public:
		void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) override;
	private:
		DrawerState<Vector3> m_eulerState;
		DrawerState<Quaternion> m_prevQuaternionState; // Undo/Redo のために、前フレームの Quaternion 値も保存しておく
	};
}