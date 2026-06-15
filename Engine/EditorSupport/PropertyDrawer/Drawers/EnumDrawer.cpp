#include "pch.h"
#include "EnumDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"


namespace CurryEngine
{
	void EnumDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		const EnumInfo* enumInfo = prop.GetAttribute("Enum") ? ReflectionRegistry::FindEnum(prop.GetAttribute("Enum")->args[0]) : nullptr;
		if (!enumInfo)
		{
			LOG_WARNING("EnumDrawer: No EnumInfo found for property: " + prop.name);
			return;
		}

		int value = std::any_cast<int>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<int>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);

		bool edited = false;
		std::string previewValue = mixed ? "---" : enumInfo->isClass ? (enumInfo->name + "::" + enumInfo->values[value].name) : enumInfo->values[value].name;
		if (ImGui::BeginCombo("##enum", previewValue.c_str()))
		{
			// コンボボックスの内容を列挙型の値で埋める
			for (const auto& enumValue : enumInfo->values)
			{
				bool isSelected = (value == enumValue.value);
				if (ImGui::Selectable(enumValue.name.c_str(), isSelected))
				{
					value = enumValue.value;
					edited = true;
				}

				if (isSelected)
				{
					ImGui::SetItemDefaultFocus(); // 最初のアイテムにフォーカスを当てる
				}
			}
			ImGui::EndCombo();
		}

		if (edited)
		{
			// 値が変更されたときの処理。複数選択されている場合は、すべての対象に対して新しい値を適用します。
			PropertyDrawHelper::ApplyToAll<int>(context, prop, value);
		}

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			// 値のコミット処理。ユーザーが編集を完了したときに、Undo/Redo コマンドを発行します。
			PropertyDrawHelper::CommitEdit<int>(prop, context, m_state, value,
				[enumInfo](const int& v) {
					// 値を文字列に変換する関数。EnumInfo を参照して、値に対応する名前を返します。
					for (const auto& enumValue : enumInfo->values)
					{
						if (enumValue.value == v)
						{
							// enum class なら "EnumName::ValueName"、そうでないなら "ValueName" を返す
							return enumInfo->isClass ? (enumInfo->name + "::" + enumValue.name) : enumValue.name;
						}
					}
					return std::to_string(v); // 対応する名前が見つからない場合は数値を文字列で返す
				}
			);
		}

#endif // USE_IMGUI
	}
}