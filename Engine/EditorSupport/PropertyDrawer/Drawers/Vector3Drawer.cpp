#include "pch.h"
#include "Vector3Drawer.h"
#include "Engine/Core/Math/Vector3.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"

namespace CurryEngine
{
	void Vector3Drawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		// targets[0] を基準にしてプロパティ値を取得
		Vector3 value = std::any_cast<Vector3>(prop.getter(context.Primary()));
		// 複数選択されているオブジェクトのプロパティ値が混在しているかどうかを判定
		bool hasMixedValues = PropertyDrawHelper::HasMixedValues<Vector3>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);
		bool edited = ImGui::DragFloat3("##Vector3", &value.x, 1.0f, 0, 0, hasMixedValues ? "---" : "%.3f");
		if (edited)
		{
			// 値が変更されたときの処理。複数選択されている場合は、すべての対象に対して新しい値を適用します。
			PropertyDrawHelper::ApplyToAll<Vector3>(context, prop, value);
		}

		// 値のコミット処理。ユーザーが編集を完了したときに、Undo/Redo コマンドを発行します。
		PropertyDrawHelper::CommitEdit<Vector3>(prop, context, m_state, value,
			[](const Vector3& v) {
				return "(" + std::to_string(v.x) + ", "
					+ std::to_string(v.y) + ", "
					+ std::to_string(v.z) + ")";
			},
			[](const Vector3& a, const Vector3& b) {
				return Vector3::Equal(a, b);
			},
			[&]() {
				// 編集開始前の状態を保存する関数。ここでは、現在の Vector3 値を m_state に保存しています。
				return ImGui::IsItemActivated();
			},
			[&]() {
				// コミットしてもいいかどうかをチェックする関数。ここでは常に true を返していますが、必要に応じて条件を追加できます。
				return ImGui::IsItemDeactivatedAfterEdit();
			}
		);

#endif // USE_IMGUI
	}
}
