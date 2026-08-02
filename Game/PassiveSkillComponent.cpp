#include "pch.h"
#include "PassiveSkillComponent.h"
#include <Engine\Editor\Dialog.h>
#include "Engine/Scenes/Scene.h"
#include "Engine/Core/GameObject.h"
#include "PassiveSkillContainer.h"
#include "ItemTooltipController.h"
#include <Engine\UI\Text.h>
#include "ItemShop.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(PassiveSkillComponent, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(PassiveSkillComponent, "UserScripts", ComponentAttributes::None, {})


void PassiveSkillComponent::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	SetPassiveSkillData(data); // 初期データをセットして、関連するコンポーネントに反映させる
}

void PassiveSkillComponent::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

//void PassiveSkillComponent::DrawProperty()
//{
//	// プロパティを描画する処理をここに実装します。
//#ifdef USE_IMGUI
//	Component::DrawProperty(); // 基底クラスのプロパティ描画を呼び出す
//
//	// PassiveSkillData のプロパティを描画
//	
//	IMGUI_PROPERTY_BEGIN();
//	bool edited = false; // プロパティが編集されたかどうかを追跡するフラグ
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
//	IMGUI_PROPERTY("Prefab Path");
//	{
//		ImGui::PushID("PrefabPath"); // ユニークなIDをプッシュ
//		char prefabPathBuffer[256];
//		strncpy_s(prefabPathBuffer, data.prefabPath.data(), sizeof(prefabPathBuffer)); // std::stringをchar配列にコピー
//		if (ImGui::InputText("##PrefabPath", prefabPathBuffer, 256))
//		{
//			data.prefabPath = std::string(prefabPathBuffer); // 変更があった場合、std::stringにコピーして保存
//		}
//		ImGui::SameLine();
//		if (ImGui::Button("...", ImVec2(0, 0))) // ファイルダイアログを開くボタン
//		{
//			const char* filter = "Prefab Files (*.prefab)\0*.prefab\0All Files (*.*)\0*.*\0";
//			char* path = OpenFileDialog(filter, "Select Prefab");
//			std::string selectedFile = path ? std::string(path) : ""; // ファイルが選択された場合はパスを文字列に変換、キャンセルされた場合は空文字列
//			if (!selectedFile.empty())
//			{
//				data.prefabPath = selectedFile; // ファイルが選択された場合、パスを保存
//			}
//		}
//		ImGui::PopID(); // IDをポップ
//	}
//
//	IMGUI_PROPERTY("Target Property");
//	{
//		ImGui::PushID("TargetProperty"); // ユニークなIDをプッシュ
//		char targetPropertyBuffer[256];
//		strncpy_s(targetPropertyBuffer, data.targetProperty.data(), sizeof(targetPropertyBuffer)); // std::stringをchar配列にコピー
//		if (ImGui::InputText("##TargetProperty", targetPropertyBuffer, 256))
//		{
//			data.targetProperty = std::string(targetPropertyBuffer); // 変更があった場合、std::stringにコピーして保存
//		}
//		ImGui::PopID(); // IDをポップ
//	}
//
//	IMGUI_PROPERTY("Modifier");
//	ImGui::InputFloat("##Modifier", &data.modifier);
//
//	IMGUI_PROPERTY_END();
//
//	if (edited)
//	{
//		SetPassiveSkillData(data, true); // データが編集された場合、関連するコンポーネントに変更を反映させる
//	}
//
//#endif
//}

json PassiveSkillComponent::Serialize() const
{
	// シリアライズ処理をここに実装します。
	json j = Component::Serialize(); // 基底クラスのシリアライズを呼び出す

	// PassiveSkillData を JSON に変換して追加
	json dataJson;
	dataJson["name"] = data.name;
	dataJson["description"] = data.description;
	dataJson["iconPath"] = data.iconPath;
	dataJson["prefabPath"] = data.prefabPath;
	dataJson["targetProperty"] = data.targetProperty;
	dataJson["modifier"] = data.modifier;
	//dataJson["maxStack"] = data.maxStack;
	j["data"] = dataJson; // "data" キーで PassiveSkillData を追加

	return j;
}

void PassiveSkillComponent::Deserialize(const json& j)
{
	// デシリアライズ処理をここに実装します。
	Component::Deserialize(j); // 基底クラスのデシリアライズを呼び出す
	if (j.contains("data"))
	{
		const json& dataJson = j["data"];
		data.name = dataJson.value("name", "");
		data.description = dataJson.value("description", "");
		data.iconPath = dataJson.value("iconPath", "");
		data.prefabPath = dataJson.value("prefabPath", "");
		data.targetProperty = dataJson.value("targetProperty", "");
		data.modifier = dataJson.value("modifier", 0.0f);
		//data.maxStack = dataJson.value("maxStack", 1);
	}
}

void PassiveSkillComponent::SetPassiveSkillData(const PassiveSkillData& newData, bool reloadTexture)
{
	data = newData;
	if (auto itemShop = GetOwner()->GetComponent<ItemShop>())
	{
		// ItemShop コンポーネントが存在する場合の処理をここに実装
		ObjectId iconImageReference = itemShop->iconImageReference; // ItemShop からアイコンイメージの参照を取得
		ObjectId tooltipReference = itemShop->tooltipReference; // ItemShop からツールチップコントローラーの参照を取得

		// データが更新されたときに、関連するコンポーネントにも変更を反映させる
		if (Image* iconImage = GetScene()->FindComponentById<Image>(iconImageReference))
		{
			iconImage->SetSource(StringToWstring(data.iconPath).c_str(), reloadTexture); // スキルのアイコンをイメージコンポーネントに設定
		}
		if (ItemTooltipController* tooltipController = GetScene()->FindComponentById<ItemTooltipController>(tooltipReference))
		{
			tooltipController->SetupTooltip(data); // ツールチップコントローラーにスキルデータを渡してセットアップ
		}
	}
}
