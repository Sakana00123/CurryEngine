#include "pch.h"
#include "ObtrusiveGadgetItemComponent.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "ItemShop.h"
#include <Engine/UI/Text.h>
#include "ItemTooltipController.h"
#include "RoundManager.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(ObtrusiveGadgetItemComponent, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(ObtrusiveGadgetItemComponent, "UserScripts", ComponentAttributes::None, {})


void ObtrusiveGadgetItemComponent::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	SetItemData(data); // アイテムデータを初期化
}

void ObtrusiveGadgetItemComponent::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

//void ObtrusiveGadgetItemComponent::DrawProperty()
//{
//	// プロパティを描画する処理をここに実装します。
//#ifdef USE_IMGUI
//	Component::DrawProperty(); // 基底クラスのプロパティ描画を呼び出す
//	// ItemData のプロパティを描画
//	bool edited = false; // プロパティが編集されたかどうかを追跡するフラグ
//	IMGUI_PROPERTY_BEGIN();
//	IMGUI_PROPERTY("Name");
//	{
//		ImGui::PushID("Name"); // ユニークなIDをプッシュ
//		char nameBuffer[256];
//		strncpy_s(nameBuffer, data.name.data(), sizeof(nameBuffer)); // std::stringをchar配列にコピー
//		if (ImGui::InputText("##Name", nameBuffer, 256))
//		{
//			data.name = std::string(nameBuffer); // 変更があった場合、std::stringにコピーして保存
//			edited = true;
//		}
//		ImGui::PopID(); // IDをポップ
//	}
//
//	IMGUI_PROPERTY("Description");
//	{
//		ImGui::PushID("Description"); // ユニークなIDをプッシュ
//		char descriptionBuffer[512];
//		strncpy_s(descriptionBuffer, data.description.data(), sizeof(descriptionBuffer)); // std::stringをchar配列にコピー
//		if (ImGui::InputText("##Description", descriptionBuffer, 512))
//		{
//			data.description = std::string(descriptionBuffer); // 変更があった場合、std::stringにコピーして保存
//			edited = true;
//		}
//		ImGui::PopID(); // IDをポップ
//	}
//
//	IMGUI_PROPERTY("Icon Path");
//	{
//		ImGui::PushID("IconPath"); // ユニークなIDをプッシュ
//		char iconPathBuffer[256];
//		strncpy_s(iconPathBuffer, data.iconPath.data(), sizeof(iconPathBuffer)); // std::stringをchar配列にコピー
//		if (ImGui::InputText("##IconPath", iconPathBuffer, 256))
//		{
//			data.iconPath = std::string(iconPathBuffer); // 変更があった場合、std::stringにコピーして保存
//			edited = true;
//		}
//		ImGui::SameLine();
//		if (ImGui::Button("...", ImVec2(0, 0))) // ファイルダイアログを開くボタン
//		{
//			const char* filter = "Image Files (*.png;*.jpg;*.jpeg)\0*.png;*.jpg;*.jpeg\0All Files (*.*)\0*.*\0";
//			char* path = OpenFileDialog(filter, "Select Icon Image");
//			std::string selectedFile = path ? std::string(path) : ""; // ファイルが選択された場合はパスを文字列に変換、キャンセルされた場合は空文字列
//			if (!selectedFile.empty())
//			{
//				data.iconPath = selectedFile; // ファイルが選択された場合、パスを保存
//				edited = true;
//			}
//		}
//		ImGui::PopID(); // IDをポップ
//	}
//
//	IMGUI_PROPERTY("3D Prefab Path");
//	{
//		ImGui::PushID("PrefabPath"); // ユニークなIDをプッシュ
//		char prefabPathBuffer[256];
//		strncpy_s(prefabPathBuffer, data.prefabPath.data(), sizeof(prefabPathBuffer)); // std::stringをchar配列にコピー
//		if (ImGui::InputText("##PrefabPath", prefabPathBuffer, 256))
//		{
//			data.prefabPath = std::string(prefabPathBuffer); // 変更があった場合、std::stringにコピーして保存
//			edited = true;
//		}
//		ImGui::SameLine();
//		if (ImGui::Button("...", ImVec2(0, 0))) // ファイルダイアログを開くボタン
//		{
//			const char* filter = "Prefab Files (*.prefab)\0*.prefab\0All Files (*.*)\0*.*\0";
//			char* path = OpenFileDialog(filter, "Select 3D Prefab");
//			std::string selectedFile = path ? std::string(path) : ""; // ファイルが選択された場合はパスを文字列に変換、キャンセルされた場合は空文字列
//			if (!selectedFile.empty())
//			{
//				data.prefabPath = selectedFile; // ファイルが選択された場合、パスを保存
//				edited = true;
//			}
//		}
//		ImGui::PopID(); // IDをポップ
//	}
//
//	IMGUI_PROPERTY("Durability");
//	{
//		ImGui::PushID("Durability"); // ユニークなIDをプッシュ
//		int durability = data.durability; // 現在の耐久値をローカル変数にコピー
//		if (ImGui::InputInt("##Durability", &durability))
//		{
//			data.durability = durability; // 変更があった場合、データに保存
//			edited = true;
//		}
//		ImGui::PopID(); // IDをポップ
//	}
//
//	IMGUI_PROPERTY("isDurabilityRoundDecrease");
//	{
//		ImGui::PushID("isDurabilityRoundDecrease"); // ユニークなIDをプッシュ
//		bool isDurabilityRoundDecrease = data.isDurabilityRoundDecrease;
//		if (ImGui::Checkbox("##isDurabilityRoundDecrease", &isDurabilityRoundDecrease))
//		{
//			data.isDurabilityRoundDecrease = isDurabilityRoundDecrease; // 変更があった場合、フラグを更新
//			edited = true;
//		}
//		ImGui::PopID(); // IDをポップ
//	}
//
//	IMGUI_PROPERTY_END();
//
//	if (edited)
//	{
//		SetItemData(data, true); // プロパティが編集された場合、関連するコンポーネントに反映させる
//	}
//#endif
//}

json ObtrusiveGadgetItemComponent::Serialize() const
{
	json j = Component::Serialize(); // 基底クラスのシリアライズを呼び出す

	json itemDataJsonArray = json::array();
	// ItemData のプロパティを JSON オブジェクトとして追加
	json itemDataJson;
	itemDataJson["name"] = data.name;
	itemDataJson["description"] = data.description;
	itemDataJson["iconPath"] = data.iconPath;
	itemDataJson["prefabPath"] = data.prefabPath;
	itemDataJson["durability"] = data.durability; // GadgetItemData の耐久値も追加
	itemDataJson["isDurabilityRoundDecrease"] = data.isDurabilityRoundDecrease; // GadgetItemData の耐久値減少フラグも追加

	itemDataJsonArray.push_back(itemDataJson);

	j["itemData"] = itemDataJsonArray; // ItemData の JSON 配列を追加

	return j;
}

void ObtrusiveGadgetItemComponent::Deserialize(const json& j)
{
	Component::Deserialize(j); // 基底クラスのデシリアライズを呼び出す

	if (j.contains("itemData") && j["itemData"].is_array() && !j["itemData"].empty())
	{
		const json& itemDataJson = j["itemData"][0]; // 配列の最初の要素を取得
		if (itemDataJson.is_object())
		{
			data.name = itemDataJson.value("name", ""); // デフォルト値は空文字列
			data.description = itemDataJson.value("description", ""); // デフォルト値は空文字列
			data.iconPath = itemDataJson.value("iconPath", ""); // デフォルト値は空文字列
			data.prefabPath = itemDataJson.value("prefabPath", ""); // デフォルト値は空文字列
			data.durability = itemDataJson.value("durability", 10); // デフォルト値は10
			data.isDurabilityRoundDecrease = itemDataJson.value("isDurabilityRoundDecrease", false); // デフォルト値はfalse
		}
	}
}

void ObtrusiveGadgetItemComponent::SetItemData(const GadgetItemData& newData, bool reloadTexture)
{
	data = newData;
	
	if (auto* iconImage = GetScene()->FindComponentById<Image>(iconImageReference))
	{
		if (!std::filesystem::exists(data.iconPath))
		{
			iconImage->SetSource(nullptr); // アイコンのテクスチャをクリア
		}
		else
		{
			iconImage->SetSource(StringToWstring(data.iconPath).c_str(), reloadTexture); // アイコンのテクスチャを更新
		}
	}
	if (auto* tooltipController = GetScene()->FindComponentById<ItemTooltipController>(tooltipReference))
	{
		tooltipController->SetupTooltip(data); // ツールチップのデータを更新
	}
}
