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
		DrawerState<Vector3>    m_eulerState;           // 表示用オイラー角
		DrawerState<Quaternion> m_externalChangeState;  // 外部変更検出用
		DrawerState<Vector3>    m_eulerOnActivated;     // 編集開始時のオイラー角（Undo 用）
		DrawerState<bool>       m_isEditing;            // ドラッグ編集中フラグ
		DrawerState<bool>       m_isEditingPending;     // ドラッグ編集中フラグの保留（IsItemActivated と IsItemDeactivatedAfterEdit のタイミングがずれているため）
	};
}