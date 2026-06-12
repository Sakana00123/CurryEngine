#include "pch.h"
#include "FloatDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"

namespace CurryEngine
{
	void FloatDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
        float value = std::any_cast<float>(prop.getter(context.Primary()));
        bool mixed = PropertyDrawHelper::HasMixedValues<float>(context, prop);

        PropertyDrawHelper::BeginPropertyLabel(prop);
        bool edited = ImGui::DragFloat("##float", &value, 1.0f, 0, 0, mixed ? "---" : "%.3f");
        if (edited)
        {
            // 値が変更されたときの処理。複数選択されている場合は、すべての対象に対して新しい値を適用します。
            PropertyDrawHelper::ApplyToAll<float>(context, prop, value);
        }

        // 値のコミット処理。ユーザーが編集を完了したときに、Undo/Redo コマンドを発行します。
        PropertyDrawHelper::CommitEdit<float>(prop, context, m_state, value,
            [](const float& v) {
                return std::to_string(v);
            },
            [](const float& a, const float& b) {
                return std::abs(a - b) < 1e-6f; // 浮動小数点数の比較は、絶対値の差が小さいかどうかで判定
            },
            []() {
                // 編集開始前の状態を保存する関数。ここでは、現在の float 値を m_state に保存しています。
                return ImGui::IsItemActivated();
            },
            []() {
                // コミットしてもいいかどうかをチェックする関数。ここでは常に true を返していますが、必要に応じて条件を追加できます。
                return ImGui::IsItemDeactivatedAfterEdit();
			}
        );
#endif // USE_IMGUI
	}
}
