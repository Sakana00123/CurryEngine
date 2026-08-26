#include "pch.h"
#include "AssetIdDrawer.h"
#include "Engine/Resources/AssetDatabase.h"
#include "Engine/Resources/AssetTypeUtils.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"


namespace CurryEngine
{
	void AssetIdDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		Resources::AssetId value = std::any_cast<Resources::AssetId>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<Resources::AssetId>(context, prop);

		PropertyDrawHelper::BeginPropertyLabel(prop);

		auto meta = Resources::AssetDatabase::Find(value);
		std::string displayName = mixed ? "---" : (meta ? meta->path.stem().string() : "None");
		ImGui::Text("%s", displayName.c_str());
		ImGui::SameLine();
		auto attr = prop.GetAttribute("AssetTypeExtension");
		if (!attr) 
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: AssetType attribute is missing.");
			return;
		}
		std::string assetTypeStr = attr->args.empty() ? "" : attr->args[0];
		if (assetTypeStr.empty())
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: AssetTypeExtension attribute has no argument.");
			return;
		}
		AssetType assetType = Resources::AssetTypeUtils::DetectFromExtension(assetTypeStr);

		// アセット選択ボタン
		if (ImGui::Button("Select Asset"))
		{
			// ダイアログを開いてアセットを選択する処理
			ImGuiWindowFlags popupFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking;
			ImGui::OpenPopup("Select Asset", popupFlags);
		}
		// ポップアップが開いている場合、アセットのリストを表示
		else if (ImGui::IsPopupOpen("Select Asset"))
		{
			if (ImGui::BeginPopup("Select Asset"))
			{
				auto assets = Resources::AssetDatabase::FindAllByType(assetType);
				for (const auto& asset : assets)
				{
					if (ImGui::Selectable(asset.path.stem().string().c_str()))
					{
						value = asset.id;
						PropertyDrawHelper::CommitEdit<Resources::AssetId>(prop, context, m_state, value,
							[](const Resources::AssetId& v) { return v.id; },
							[](const Resources::AssetId& a, const Resources::AssetId& b) { return a == b; });
						break;
					}
				}
				ImGui::EndPopup();
			}
		}
		ImGui::SameLine();
		// クリアボタン
		if (ImGui::Button("X"))
		{
			value = Resources::AssetId();
			PropertyDrawHelper::CommitEdit<Resources::AssetId>(prop, context, m_state, value,
				[](const Resources::AssetId& v) { return v.id; },
				[](const Resources::AssetId& a, const Resources::AssetId& b) { return a == b; },
				nullptr,
				[]() { return true; });
		}



#endif // USE_IMGUI
	}
}
