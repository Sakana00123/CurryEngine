#include "pch.h"
#include "ShopItemList.h"
#include <Engine\Editor\Dialog.h>
#include "Engine/Scenes/Scene.h"
#include "ItemShop.h"
#include "PassiveSkillComponent.h"
#include "PassiveSkillContainer.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
//REGISTER_COMPONENT(ShopItemList, "UserScripts")
REGISTER_COMPONENT_WITH_ATTRIBUTES(ShopItemList, "UserScripts", ComponentAttributes::ExecuteInEditMode, {})


void ShopItemList::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void ShopItemList::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

//void ShopItemList::DrawProperty()
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
//	for (size_t i = 0; i < itemEntries.size(); ++i)
//	{
//		ImGui::PushID(static_cast<int>(i)); // ユニークなIDをプッシュ
//		char buffer[256];
//		strncpy_s(buffer, itemEntries[i].prefabPath.data(), sizeof(buffer)); // std::stringをchar配列にコピー
//		if (ImGui::InputText("##PrefabPath", buffer, sizeof(buffer))) // ラベルを空にしてIDだけで識別
//		{
//			itemEntries[i].prefabPath = std::string(buffer); // 変更があった場合、std::stringにコピーして保存
//		}
//		if (ImGui::IsItemHovered()) // 入力フィールドにマウスオーバーしているときにフルパスをツールチップで表示
//		{
//			ImGui::SetTooltip("%s", itemEntries[i].prefabPath.c_str());
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
//				itemEntries[i].prefabPath = selectedFile; // ファイルが選択された場合、パスを保存
//			}
//		}
//
//		// スタック数の入力フィールドを追加
//		{
//			ImGui::Text("Stack Count:");
//			ImGui::SameLine();
//			ImGui::InputInt("##StackCount", &itemEntries[i].stackCount); // スタック数の入力フィールド
//		}
//		
//		// レアリティの入力フィールドを追加
//		{
//			ImGui::Text("Rarity:");
//			ImGui::SameLine();
//			const char* rarityOptions[] = { "Common", "Rare", "Legendary" };
//			ImGui::Combo("##Rarity", &itemEntries[i].rarity, rarityOptions, IM_ARRAYSIZE(rarityOptions)); // レアリティのコンボボックス
//		}
//
//		if (i < itemEntries.size() - 1) // 最後のエントリ以外には区切り線を表示
//		{
//			ImGui::Separator();
//		}
//		ImGui::PopID(); // IDをポップ
//	}
//	ImGui::EndChild();
//	
//	if (ImGui::Button("+", ImVec2(30, 30)))
//	{
//		ItemEntry newEntry{
//			.prefabPath = "", // デフォルトのプレハブパスは空文字列
//			.stackCount = 99 // デフォルトのスタック数は99
//		};
//		itemEntries.push_back(newEntry); // 新しいエントリを追加
//	}
//	ImGui::SameLine();
//	if (ImGui::Button("-", ImVec2(30, 30)) && !itemEntries.empty())
//	{
//		itemEntries.pop_back(); // 最後のエントリを削除
//	}
//
//	IMGUI_PROPERTY_END();
//
//	ImGui::Separator();
//
//	if (ImGui::Button("Reroll Items"))
//	{
//		RerollItems(); // アイテムを再生成する関数を呼び出す
//	}
//
//#endif // USE_IMGUI
//}

json ShopItemList::Serialize() const
{
	// シリアライズ処理をここに実装します。
	json j = Component::Serialize(); // 基底クラスのシリアライズを呼び出す
	j["spacing"] = spacing;
	j["itemCount"] = itemCount;

	// アイテムのプレハブのファイルパスのリストをシリアライズ
	json entriesJson = json::array();
for (const auto& entry : itemEntries)
{
	json entryJson;
	entryJson["prefabPath"] = entry.prefabPath;
	entryJson["stackCount"] = entry.stackCount;
	entryJson["rarity"] = entry.rarity;
	entriesJson.push_back(entryJson);
}
	j["entries"] = entriesJson;
	return j;
}

void ShopItemList::Deserialize(const json& j)
{
	// デシリアライズ処理をここに実装します。
	Component::Deserialize(j); // 基底クラスのデシリアライズを呼び出す
	if (j.contains("spacing")) spacing = j["spacing"].get<float>();
	if (j.contains("itemCount")) itemCount = j["itemCount"].get<int>();

	// 古い形式に対応するため、itemPrefabPaths が存在する場合はそれを優先してデシリアライズし、entries が存在する場合はそれを使用するようにします。
	if (j.contains("itemPrefabPaths")) {
		std::vector<std::string> itemPrefabPaths = j["itemPrefabPaths"].get<std::vector<std::string>>();
		itemEntries.clear();
		for (const auto& path : itemPrefabPaths)
		{
			ItemEntry entry;
			entry.prefabPath = path;
			entry.stackCount = 99; // デフォルトのスタック数を設定（必要に応じて変更）
			entry.rarity = 0; // デフォルトのレアリティを設定（必要に応じて変更）
			itemEntries.push_back(entry);
		}
	}

	if (j.contains("entries")) {
		itemEntries.clear();
		for (const auto& entryJson : j["entries"])
		{
			ItemEntry entry;
			entry.prefabPath = entryJson.value("prefabPath", ""); // デフォルト値は空文字列
			entry.stackCount = entryJson.value("stackCount", 99); // デフォルト値は99
			entry.rarity = entryJson.value("rarity", 0); // デフォルト値は0 (Common)
			itemEntries.push_back(entry);
		}
	}
}

void ShopItemList::RerollItems()
{
	// アイテムを再生成する処理をここに実装します。

	// 子のアイテムをすべて削除
	for (auto& child : GetOwner()->GetChildren())
	{
		child->Destroy();
	}

	// 初期のレアリティの排出確率を定義します。必要に応じて調整してください。
	float rarityProbabilities[] = { 0.8f, 0.15f, 0.05f }; // Common: 80%, Rare: 15%, Legendary: 5%

	// レアリティアップループを追加
	const auto& containers = GetScene()->FindComponents<PassiveSkillContainer>();
	if (!containers.empty())
	{
		PassiveSkillContainer* passiveSkillContainer = containers.front();
		if (passiveSkillContainer)
		{
			float value = passiveSkillContainer->GetModifier("RarityUp"); // プレイヤーのレアリティアップの効果値を取得する関数を呼び出す
			// レアリティアップの効果値に基づいて、レアリティの排出確率を調整します。
			rarityProbabilities[1] += value * 0.1f; // Rareの確率を増加させる
			rarityProbabilities[2] += value * 0.05f; // Legendaryの確率を増加させる
			// 確率の合計が1を超えないように正規化します。
			float totalProbability = rarityProbabilities[0] + rarityProbabilities[1] + rarityProbabilities[2];
			for (int i = 0; i < 3; ++i)
			{
				rarityProbabilities[i] /= totalProbability;
			}
			// 調整後のレアリティの排出確率をログに出力します。必要に応じて削除してください。
			Console::Log(std::format("Adjusted Rarity Probabilities - Common: {:.2f}%, Rare: {:.2f}%, Legendary: {:.2f}%",
				rarityProbabilities[0] * 100, rarityProbabilities[1] * 100, rarityProbabilities[2] * 100));
		}
	}

	std::vector<int> rolledIndices; // ロールされたアイテムのインデックスを記録するリスト
	std::unordered_map<std::string, int> rolledPrefabPaths; // ロールされたアイテムのプレハブパスとその出現回数を記録するマップ
	
	PassiveSkillContainer* container = GetScene()->FindComponentById<PassiveSkillContainer>(itemContainerReference);
	if (container)
	{
		std::unordered_map<std::string, int> rollableCounts; // ロール可能なアイテムのプレハブパスとそのロール可能回数を記録するマップ
		for (int i = 0; i < itemEntries.size(); ++i)
		{
			int maxStack = itemEntries[i].stackCount;
			if (maxStack > 0)
			{
				int currentStack = container->GetCurrentStack(itemEntries[i].prefabPath);
				int rollableCount = maxStack - currentStack;
				if (rollableCount > 0)
				{
					rollableCounts[itemEntries[i].prefabPath] = rollableCount;
				}
			}
		}

		for (int i = 0; i < itemCount && !itemEntries.empty(); ++i)
		{
			// スタック制限を考慮して、各レアリティごとにロール可能なアイテムのリストを作成する
			std::vector<int> availableIndicesByRarity[3];
			for (int j = 0; j < itemEntries.size(); ++j)
			{
				const std::string& prefabPath = itemEntries[j].prefabPath;
				if (rollableCounts.find(prefabPath) != rollableCounts.end() && rollableCounts[prefabPath] > 0)
				{
					int rarity = itemEntries[j].rarity;
					if (rarity >= 0 && rarity < 3)
					{
						availableIndicesByRarity[rarity].push_back(j);
					}
				}
			}

			// ロール可能なアイテムが一つもない場合はループを終了する
			bool hasRollableItems = !availableIndicesByRarity[0].empty() || !availableIndicesByRarity[1].empty() || !availableIndicesByRarity[2].empty();
			if (!hasRollableItems) break;

			// そのレアリティに属するアイテムが枯渇している場合を考慮して、確率を再計算する
			float currentProbs[3];
			float totalProb = 0.0f;
			for (int r = 0; r < 3; ++r)
			{
				currentProbs[r] = availableIndicesByRarity[r].empty() ? 0.0f : rarityProbabilities[r];
				totalProb += currentProbs[r];
			}

			if (totalProb <= 0.0f) break;

			// 調整後の確率に基づいて、排出するレアリティを抽選する
			float randChoice = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * totalProb;
			int chosenRarity = 0;
			float cumulative = 0.0f;
			for (int r = 0; r < 3; ++r)
			{
				cumulative += currentProbs[r];
				if (randChoice <= cumulative)
				{
					chosenRarity = r;
					break;
				}
			}

			// 選択されたレアリティのアイテム群の中から、ランダムに1つを選択する
			int poolSize = availableIndicesByRarity[chosenRarity].size();
			int randomIndex = availableIndicesByRarity[chosenRarity][rand() % poolSize];
			const std::string& prefabPath = itemEntries[randomIndex].prefabPath;

			rolledIndices.push_back(randomIndex);
			rolledPrefabPaths[prefabPath]++;
			rollableCounts[prefabPath]--;
		}
	}
	else
	{
		for (int i = 0; i < itemCount && !itemEntries.empty(); ++i)
		{
			// コンテナがない場合も同様にレアリティ抽選を適用する
			std::vector<int> availableIndicesByRarity[3];
			for (int j = 0; j < itemEntries.size(); ++j)
			{
				int rarity = itemEntries[j].rarity;
				if (rarity >= 0 && rarity < 3)
				{
					availableIndicesByRarity[rarity].push_back(j);
				}
			}

			bool hasRollableItems = !availableIndicesByRarity[0].empty() || !availableIndicesByRarity[1].empty() || !availableIndicesByRarity[2].empty();
			if (!hasRollableItems) break;

			float currentProbs[3];
			float totalProb = 0.0f;
			for (int r = 0; r < 3; ++r)
			{
				currentProbs[r] = availableIndicesByRarity[r].empty() ? 0.0f : rarityProbabilities[r];
				totalProb += currentProbs[r];
			}

			if (totalProb <= 0.0f) break;

			float randChoice = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * totalProb;
			int chosenRarity = 0;
			float cumulative = 0.0f;
			for (int r = 0; r < 3; ++r)
			{
				cumulative += currentProbs[r];
				if (randChoice <= cumulative)
				{
					chosenRarity = r;
					break;
				}
			}

			int poolSize = availableIndicesByRarity[chosenRarity].size();
			int randomIndex = availableIndicesByRarity[chosenRarity][rand() % poolSize];

			rolledIndices.push_back(randomIndex);
		}
	}

	// ロールされたアイテムのプレハブパスのリストをもとに、アイテムを生成する
	for (size_t i = 0; i < rolledIndices.size(); ++i)
	{
		const ItemEntry& entry = itemEntries[rolledIndices[i]];
		const std::string& prefabPath = entry.prefabPath;
		GameObject* newItem = Instantiate(prefabPath, GetOwner()->GetTransform());
		if (newItem)
		{
			if (RectTransform* rect = newItem->GetComponent<RectTransform>())
			{
				rect->SetAnchoredPosition(Vector2(i * spacing, 0)); // アイテムを横に並べる
			}
			if (PassiveSkillComponent* skill = newItem->GetComponent<PassiveSkillComponent>())
			{
				PassiveSkillData data = skill->GetPassiveSkillData();
				data.shopPrefabPath = prefabPath; // ショップ内でのアイテムのプレハブのファイルパスを設定(データに保存)
				data.maxStack = entry.stackCount; // スタック制限を設定
				skill->SetPassiveSkillData(data); // データを再設定
			}
		}
	}
}
