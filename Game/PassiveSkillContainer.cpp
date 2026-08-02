#include "pch.h"
#include "PassiveSkillContainer.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "PassiveSkillComponent.h"
#include "PassiveSkillView.h"
#include <Engine\UI\RectTransform.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(PassiveSkillContainer, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(PassiveSkillContainer, "UserScripts", ComponentAttributes::None, {})


void PassiveSkillContainer::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void PassiveSkillContainer::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

//void PassiveSkillContainer::DrawProperty()
//{
//	// プロパティを描画する処理をここに実装します。
//#ifdef USE_IMGUI
//	Component::DrawProperty(); // 基底クラスのプロパティ描画を呼び出す
//	IMGUI_PROPERTY_BEGIN();
//	// 獲得したスキルのリストを描画
//	IMGUI_PROPERTY("Acquired Skills");
//	ImGui::BeginChild("AcquiredSkillsChild", ImVec2(0, 100), ImGuiChildFlags_Borders);
//	for (const auto& [_, skillInfo] : acquiredSkills)
//	{
//		const auto& skillData = skillInfo.skillData;
//		ImGui::Text("%s (Stack: %d/%d)", skillData.name.c_str(), skillInfo.currentStack, skillData.maxStack);
//		ImGui::Text("Description: %s", skillData.description.c_str());
//		ImGui::Separator();
//	}
//	ImGui::EndChild();
//
//	IMGUI_PROPERTY_END();
//
//#endif
//}

void PassiveSkillContainer::AddSkill(const PassiveSkillData& skillData)
{
	// プレイヤーが新しいパッシブスキルを獲得する処理をここに実装します。
	if (!CanAcquireSkill(skillData))
	{
		Console::Log("Cannot acquire skill: " + skillData.name);
		return; // 獲得できない場合は処理を中断
	}
	GameObject* parentObject = GetOwner()->GetScene()->GetObjectManager()->Find(containerReference);
	if (parentObject)
	{
		if (HasSkill(skillData.name))
		{
			// 既に獲得しているスキルの場合はスタック数を増やす
			auto& skillInfo = acquiredSkills.at(skillData.name); // スキルのスタック情報
			skillInfo.currentStack++;
			skillInfo.totalModifier += skillData.modifier; // 効果値も更新
			if (auto skillView = skillInfo.skillView.lock())
			{
				skillView->UpdateStackCount(skillInfo.currentStack); // スキルビューにスタック数の更新を通知
			}
			Console::Log("Increased stack for skill: " + skillData.name + " (Current Stack: " + std::to_string(acquiredSkills.at(skillData.name).currentStack) + ")");
		}
		else // 新しいスキルを獲得する場合はアイコンを生成してスタック情報を保存
		{
			int childIndex = static_cast<int>(parentObject->GetChildren().size()); // 現在の子オブジェクトの数を取得してインデックスに使用
			GameObject* skillPrefab = Instantiate(skillData.prefabPath, parentObject->GetTransform());
			if (skillPrefab)
			{
				if (RectTransform* rectTransform = skillPrefab->GetComponent<RectTransform>())
				{
					Vector2 offset = Vector2(150.0f * childIndex, 0); // 配置のオフセット

					// アイコンを横に並べるためのオフセットを計算
					rectTransform->SetAnchoredPosition(offset);
				}
				StackableSkillInfo skillInfo;
				skillInfo.totalModifier = skillData.modifier;
				skillInfo.currentStack = 1; // 新しいスキルを獲得した場合はスタック数を1に設定
				skillInfo.skillData = skillData;

				// プレハブから PassiveSkillView コンポーネントを取得して、スキルデータを設定
				if (const auto& skillView = skillPrefab->GetComponentShared<PassiveSkillView>())
				{
					skillView->SetPassiveSkillData(skillData);
					skillInfo.skillView = skillView; // スキルビューへの参照を保存
				}
				else
				{
					_ASSERT_EXPR_A(false, ("PassiveSkillComponent not found in prefab: " + skillData.prefabPath).c_str());
				}
				acquiredSkills[skillData.name] = skillInfo; // スキルデータを保存
			}
		}
	}
}

bool PassiveSkillContainer::HasSkill(const std::string& skillName) const
{
	// プレイヤーが特定のスキルを持っているかを確認する処理をここに実装します。
	return acquiredSkills.contains(skillName);
}

bool PassiveSkillContainer::CanAcquireSkill(const PassiveSkillData& skillData) const
{
	// プレイヤーが特定のスキルを獲得できるかを確認する処理をここに実装します。
	// 例えば、同じスキルを複数回獲得できない場合は、既に獲得しているかどうかをチェックします。
	if (HasSkill(skillData.name))
	{
		if (acquiredSkills.size() > 0)
		{
			if (acquiredSkills.at(skillData.name).currentStack >= skillData.maxStack)
			{
				return false; // スキルのスタックが最大に達している場合は獲得できない
			}
			else
			{
				return true; // スキルのスタックがまだ余裕がある場合は獲得可能
			}
		}
	}
	// その他の条件（例: レベル要件、前提スキルなど）もここでチェックできます。
	return true; // 獲得可能な場合は true を返す
}

int PassiveSkillContainer::GetCurrentStack(const std::string& shopPrefabPath) const
{
	// ショップ内のアイテムのプレハブパスを指定して、そのスキルを獲得できるかを確認する処理をここに実装します。
	for (const auto& [_, skillInfo] : acquiredSkills)
	{
		if (skillInfo.skillData.shopPrefabPath == shopPrefabPath)
		{
			return skillInfo.currentStack; // 該当するスキルが見つかった場合は現在のスタック数を返す
		}
	}
	return 0; // 該当するスキルが見つからない場合はスタック数0を返す
}

int PassiveSkillContainer::GetMaxStack(const std::string& shopPrefabPath) const
{
	// ショップ内のアイテムのプレハブパスを指定して、そのスキルの最大スタック数を取得する処理をここに実装します。
	for (const auto& [_, skillInfo] : acquiredSkills)
	{
		if (skillInfo.skillData.shopPrefabPath == shopPrefabPath)
		{
			return skillInfo.skillData.maxStack; // 該当するスキルが見つかった場合は最大スタック数を返す
		}
	}
	return 0; // 該当するスキルが見つからない場合は最大スタック数0を返す
}

float PassiveSkillContainer::GetModifier(const std::string& propertyName) const
{
	// 指定したプロパティに対する全てのスキルの効果値を合計して返す処理をここに実装します。
	float totalModifier = 0.0f;
	for (const auto& [_, skillInfo] : acquiredSkills)
	{
		auto& skillData = skillInfo.skillData; // スキルのスタック情報
		if (skillData.targetProperty == propertyName)
		{
			totalModifier += skillInfo.totalModifier;
		}
	}
	return totalModifier;
}
