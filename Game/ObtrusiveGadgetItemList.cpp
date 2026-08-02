#include "pch.h"
#include "ObtrusiveGadgetItemList.h"
#include <Engine\Editor\Dialog.h>
#include "Engine/Scenes/Scene.h"
#include <Engine\UI\Text.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(ObtrusiveGadgetItemList, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(ObtrusiveGadgetItemList, "UserScripts", ComponentAttributes::None, {})


void ObtrusiveGadgetItemList::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void ObtrusiveGadgetItemList::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

//void ObtrusiveGadgetItemList::DrawProperty()
//{
//	// プロパティを描画する処理をここに実装します。
//#ifdef USE_IMGUI
//
//	Component::DrawProperty(); // 基底クラスのプロパティ描画を呼び出す
//
//
//	IMGUI_PROPERTY_BEGIN();
//	// プレハブのファイルパスのリストを描画
//	IMGUI_PROPERTY("Item Prefab Paths");
//	ImGui::BeginChild("PrefabPathsChild", ImVec2(0, 0), ImGuiChildFlags_Borders);
//	for (size_t i = 0; i < itemPrefabPaths.size(); ++i)
//	{
//		ImGui::PushID(static_cast<int>(i)); // ユニークなIDをプッシュ
//		char buffer[256];
//		strncpy_s(buffer, itemPrefabPaths[i].data(), sizeof(buffer)); // std::stringをchar配列にコピー
//		if (ImGui::InputText("##PrefabPath", buffer, sizeof(buffer))) // ラベルを空にしてIDだけで識別
//		{
//			itemPrefabPaths[i] = std::string(buffer); // 変更があった場合、std::stringにコピーして保存
//		}
//		if (ImGui::IsItemHovered()) // 入力フィールドにマウスオーバーしているときにフルパスをツールチップで表示
//		{
//			ImGui::SetTooltip("%s", itemPrefabPaths[i].c_str());
//		}
//
//		ImGui::SameLine();
//		if (ImGui::Button("...", ImVec2(0, 0))) // ファイルダイアログを開くボタン
//		{
//			const char* filter = "Prefab Files (*.prefab)\0*.prefab\0All Files (*.*)\0*.*\0";
//			char* path = OpenFileDialog(filter, "Select Item Prefab");
//			std::string selectedFile = path ? std::string(path) : ""; // ファイルが選択された場合はパスを文字列に変換、キャンセルされた場合は空文字列
//			if (!selectedFile.empty())
//			{
//				itemPrefabPaths[i] = selectedFile; // ファイルが選択された場合、パスを保存
//			}
//		}
//
//		ImGui::PopID(); // IDをポップ
//	}
//	ImGui::EndChild();
//
//	if (ImGui::Button("+", ImVec2(30, 30)))
//	{	
//		itemPrefabPaths.push_back(""); // 新しいエントリを追加
//	}
//	ImGui::SameLine();
//	if (ImGui::Button("-", ImVec2(30, 30)) && !itemPrefabPaths.empty())
//	{
//		itemPrefabPaths.pop_back(); // 最後のエントリを削除
//	}
//
//	IMGUI_PROPERTY_END();
//
//	ImGui::Separator();
//
//	if (ImGui::Button("Reroll"))
//	{
//		RerollItem(); // アイテムを再生成する関数を呼び出す
//	}
//
//#endif // USE_IMGUI
//}

json ObtrusiveGadgetItemList::Serialize() const
{
	json j;
	// シリアライズするプロパティをここに追加します。
	j["itemPrefabPaths"] = itemPrefabPaths; // プレハブのファイルパスのリストをシリアライズ
	return j;
}

void ObtrusiveGadgetItemList::Deserialize(const json& j)
{
	// デシリアライズするプロパティをここに追加します。
	if (j.contains("itemPrefabPaths") && j["itemPrefabPaths"].is_array())
	{
		itemPrefabPaths = j["itemPrefabPaths"].get<std::vector<std::string>>(); // プレハブのファイルパスのリストをデシリアライズ
	}
}

void ObtrusiveGadgetItemList::RerollItem()
{
	// アイテムを再生成する処理をここに実装します。
	// 子のアイテムをすべて削除
	for (auto& child : GetOwner()->GetChildren())
	{
		child->Destroy();
	}

	if (itemSpawnedList.size() >= itemPrefabPaths.size())
	{
		itemSpawnedList.clear(); // すべてのアイテムが出現している場合、リストをリセット
	}

	// プレハブのファイルパスのリストから新しいアイテムを生成
	int randomIndex = 0;
	do
	{
		randomIndex = rand() % itemPrefabPaths.size(); // ランダムにインデックスを選択
	} while (std::find(itemSpawnedList.begin(), itemSpawnedList.end(), randomIndex) != itemSpawnedList.end() && !itemPrefabPaths.empty());
	std::string selectedPrefabPath = itemPrefabPaths[randomIndex]; // 選択されたプレハブのファイルパス

	// プレハブからアイテムをインスタンス化
	GameObject* newItem = Instantiate(selectedPrefabPath, GetOwner()->GetTransform());
	itemSpawnedList.push_back(randomIndex); // 出現したアイテムのインデックスをリストに追加
}

void ObtrusiveGadgetItemList::SpawnItem(int itemIndex)
{
	if (itemIndex < 0 || itemIndex >= itemPrefabPaths.size())
	{
		return; // インデックスが範囲外の場合は何もしない
	}
	// 子のアイテムをすべて削除
	for (auto& child : GetOwner()->GetChildren())
	{
		child->Destroy();
	}
	std::string selectedPrefabPath = itemPrefabPaths[itemIndex]; // 指定されたインデックスのプレハブのファイルパス
	// プレハブからアイテムをインスタンス化
	GameObject* newItem = Instantiate(selectedPrefabPath, GetOwner()->GetTransform());
	itemSpawnedList.push_back(itemIndex); // 出現したアイテムのインデックスをリストに追加
}

void ObtrusiveGadgetItemList::UpdateRoundText(int roundsRemaining)
{
	if (auto* roundText = GetScene()->FindComponentById<Text>(roundTextReference))
	{
		if (roundsRemaining == 3)
		{
			roundText->SetText(L"おじゃまガジェット\nねくすと！"); // ラウンドテキストを更新
		}
		else
		{
			roundText->SetText(L"出現まであと\n" + std::to_wstring(roundsRemaining) + L"ラウンド"); // ラウンドテキストを更新
		}
	}
}
