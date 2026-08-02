#include "pch.h"
#include "GadgetItemComponent.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "ItemShop.h"
#include <Engine/UI/Text.h>
#include "ItemTooltipController.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(GadgetItemComponent, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(GadgetItemComponent, "UserScripts", ComponentAttributes::None, {})


void GadgetItemComponent::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	SetItemData(data); // 初期データをセットして、関連するコンポーネントに反映させる
}

void GadgetItemComponent::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

//void GadgetItemComponent::DrawProperty()
//{
//#ifdef USE_IMGUI
//	// プロパティを描画する処理をここに実装します。
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
//	IMGUI_PROPERTY("Inventory Prefab Path");
//	{
//		ImGui::PushID("InventoryPrefabPath"); // ユニークなIDをプッシュ
//		char inventoryPrefabPathBuffer[256];
//		strncpy_s(inventoryPrefabPathBuffer, data.inventoryItemPath.data(), sizeof(inventoryPrefabPathBuffer)); // std::stringをchar配列にコピー
//		if (ImGui::InputText("##InventoryPrefabPath", inventoryPrefabPathBuffer, 256))
//		{
//			data.inventoryItemPath = std::string(inventoryPrefabPathBuffer); // 変更があった場合、std::stringにコピーして保存
//			edited = true;
//		}
//		ImGui::SameLine();
//		if (ImGui::Button("...", ImVec2(0, 0))) // ファイルダイアログを開くボタン
//		{
//			const char* filter = "Prefab Files (*.prefab)\0*.prefab\0All Files (*.*)\0*.*\0";
//			char* path = OpenFileDialog(filter, "Select Inventory Prefab");
//			std::string selectedFile = path ? std::string(path) : ""; // ファイルが選択された場合はパスを文字列に変換、キャンセルされた場合は空文字列
//			if (!selectedFile.empty())
//			{
//				data.inventoryItemPath = selectedFile; // ファイルが選択された場合、パスを保存
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
//		int durability = data.durability;
//		if (ImGui::InputInt("##Durability", &durability))
//		{
//			data.durability = durability; // 変更があった場合、耐久値を更新
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
//
//#endif // USE_IMGUI
//}

json GadgetItemComponent::Serialize() const
{
	// シリアライズ処理をここに実装します。
	json j = Component::Serialize(); // 基底クラスのシリアライズを呼び出す
	// ItemData を JSON に変換して追加
	json dataJson;
	dataJson["name"] = data.name;
	dataJson["description"] = data.description;
	dataJson["iconPath"] = data.iconPath;
	dataJson["inventoryItemPath"] = data.inventoryItemPath;
	dataJson["prefabPath"] = data.prefabPath;
	dataJson["price"] = data.price;
	dataJson["durability"] = data.durability; // GadgetItemData の耐久値も追加
	dataJson["isDurabilityRoundDecrease"] = data.isDurabilityRoundDecrease; // GadgetItemData のフラグも追加
	j["data"] = dataJson; // "data" キーで ItemData を追加
	return j;
}

void GadgetItemComponent::Deserialize(const json& j)
{
	// デシリアライズ処理をここに実装します。
	Component::Deserialize(j); // 基底クラスのデシリアライズを呼び出す
	if (j.contains("data"))
	{
		const json& dataJson = j["data"];
		data.name = dataJson.value("name", "");
		data.description = dataJson.value("description", "");
		data.iconPath = dataJson.value("iconPath", "");
		data.inventoryItemPath = dataJson.value("inventoryItemPath", "");
		data.prefabPath = dataJson.value("prefabPath", "");
		data.price = dataJson.value("price", 0);
		data.durability = dataJson.value("durability", 10); // GadgetItemData の耐久値もデシリアライズ
		data.isDurabilityRoundDecrease = dataJson.value("isDurabilityRoundDecrease", false); // GadgetItemData のフラグもデシリアライズ
	}
}

void GadgetItemComponent::SetItemData(const GadgetItemData& newData, bool reloadTexture)
{
	data = newData;

	if (auto* itemShop = GetOwner()->GetComponent<ItemShop>())
	{
		ObjectId iconImageReference = itemShop->iconImageReference; // ItemShop からアイコンイメージの参照を取得
		ObjectId tooltipReference = itemShop->tooltipReference; // ItemShop からツールチップコントローラーの参照を取得

		// 取得した参照を使用して、アイテムのアイコンや名前を更新する処理をここに実装します。
		if (auto* iconImage = itemShop->GetScene()->FindComponentById<Image>(iconImageReference))
		{
			if (!std::filesystem::exists(data.iconPath))
			{
				Console::LogWarning("Icon file does not exist: " + data.iconPath);
				iconImage->SetSource(nullptr); // アイコンが存在しない場合はダミー画像をセット
			}
			else
			{
				// アイコンイメージを更新する処理をここに実装します。
				iconImage->SetSource(StringToWstring(data.iconPath).c_str(), reloadTexture); // アイコンのファイルパスをセット
			}
		}
		if (auto* tooltipController = itemShop->GetScene()->FindComponentById<ItemTooltipController>(tooltipReference))
		{
			// ツールチップコントローラーを更新する処理をここに実装します。
			tooltipController->SetupTooltip(data); // 例: アイテムの説明をセット
		}
		
	}

}
