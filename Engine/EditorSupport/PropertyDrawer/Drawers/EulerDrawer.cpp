#include "pch.h"
#include "EulerDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"


namespace CurryEngine
{
	void EulerDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
        Quaternion value = std::any_cast<Quaternion>(prop.getter(context.Primary()));
        bool mixed = PropertyDrawHelper::HasMixedValues<Quaternion>(context, prop);

        float vSpeed = 0.1f;
        float vMin = 0.0f;
        float vMax = 0.0f;
        const char* format = "%.3f";

        {
            const AttributeInfo* rangeAttr = prop.GetAttribute("Range");
            if (rangeAttr && rangeAttr->args.size() >= 2)
            {
                vMin = std::stof(rangeAttr->args[0]);
                vMax = std::stof(rangeAttr->args[1]);
            }
            const AttributeInfo* speedAttr = prop.GetAttribute("Speed");
            if (speedAttr && !speedAttr->args.empty())
            {
                vSpeed = std::stof(speedAttr->args[0]);
            }
            const AttributeInfo* formatAttr = prop.GetAttribute("Format");
            if (formatAttr && !formatAttr->args.empty())
            {
                format = formatAttr->args[0].c_str();
            }
        }

        Vector3& euler = m_eulerState.Prev(prop.name);
        Quaternion& prevQuat = m_externalChangeState.Prev(prop.name);
        Vector3& eulerOnActivated = m_eulerOnActivated.Prev(prop.name);
        bool& isEditing = m_isEditing.Prev(prop.name);
		bool& isEditingPending = m_isEditingPending.Prev(prop.name);

        if (isEditingPending)
        {
            // 前回のフレームで編集開始が検出されているのに、現在のフレームで IsItemActivated() が false の場合は、編集中フラグをリセットする
            isEditing = false;
            isEditingPending = false;
		}

        // 外部変更検出（編集中でなく、かつ自分が最後に書いた値と異なる場合のみ同期する）
        if (!isEditing && value != prevQuat)
        {
            euler = value.ToEuler();
            prevQuat = value;
        }

        PropertyDrawHelper::BeginPropertyLabel(prop);
        bool edited = ImGui::DragFloat3("##Euler", &euler.x, vSpeed, vMin, vMax, mixed ? "---" : format);

        // 編集開始
        if (ImGui::IsItemActivated())
        {
            isEditing = true;
            eulerOnActivated = euler;  // 編集開始時のオイラー角を記録（ToEuler() を呼ばない）
        }

        if (edited)
        {
            Quaternion newQuat = Quaternion::FromEuler(euler);
            PropertyDrawHelper::ApplyToAll<Quaternion>(context, prop, newQuat);
            prevQuat = newQuat;
        }

        // 編集完了 → Undo/Redo コマンド発行
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
			isEditingPending = true; // 編集完了が検出されたフレームで、次のフレームの冒頭で編集中フラグをリセットするためのフラグをセット
            
            Quaternion quatBefore = Quaternion::FromEuler(eulerOnActivated);
            Quaternion quatAfter = Quaternion::FromEuler(euler);

			prevQuat = quatAfter; // コマンド実行前に prevQuat を更新しておく（コマンドの実行中に外部変更があっても、古い値と比較して正しく検出できるようにするため）

            if (Quaternion::NotEqual(quatBefore, quatAfter))
            {
                // Undo ログ文字列はオイラー角から直接生成（ToEuler() を呼ばない）
                auto toStr = [](const Vector3& e) {
                    return "(" + std::to_string(e.x) + ", "
                        + std::to_string(e.y) + ", "
                        + std::to_string(e.z) + ")";
                    };
                std::string description = "Set " + prop.name
                    + " [ new: " + toStr(euler) + "]"
                    + " [ old: " + toStr(eulerOnActivated) + "]";

                struct CommandData
                {
                    PropertyDrawContext ctx;
                    PropertyInfo        prop;
                    Quaternion          quat;
                    Vector3             euler;
                    DrawerState<Vector3>* eulerState;
                    DrawerState<Quaternion>* externalChangeState;
                };

                CurryEngine::History::ExecuteCommand(
                    std::make_shared<SetValueCommand<CommandData>>(
                        description,
                        [](const CommandData& data) {
                            PropertyDrawHelper::ApplyToAll<Quaternion>(data.ctx, data.prop, data.quat);
                            // Quaternion と同時にオイラー角も復元 → ToEuler() が呼ばれない
                            data.eulerState->Prev(data.prop.name) = data.euler;
                            data.externalChangeState->Prev(data.prop.name) = data.quat;
                        },
                        CommandData{ context, prop, quatBefore, eulerOnActivated, &m_eulerState, &m_externalChangeState },
                        CommandData{ context, prop, quatAfter,  euler,            &m_eulerState, &m_externalChangeState }
                    )
                );
            }
        }
#endif // USE_IMGUI
	}
}