#include "pch.h"
#include "ObjectIdDrawer.h"

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"

#include "Engine/Core/ObjectManager.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"


inline static std::string GetObjectNameById(const ObjectId& id)
{
	if (!id.IsValid())
		return "None";
	const auto& obj = ObjectManager::Find(id);
	return obj ? obj->GetName() : ("Unknown(" + std::to_string(id.Value()) + ")");
}

inline static std::string GetComponentNameById(const ObjectId& id)
{
	if (!id.IsValid())
		return "None";
	const auto& componentCacheMap = SceneManager::GetLoadingSceneOrCurrentScene()->GetObjectManager()->GetComponentCacheMap();
	auto it = componentCacheMap.find(id);
	if (it != componentCacheMap.end())
	{
		if (auto component = it->second.lock())
		{
			const auto& owner = component->GetOwner();
			return owner ? owner->GetName() : ("Unknown Component(" + std::to_string(id.Value()) + ")");
		}
	}
	return ("Unknown(" + std::to_string(id.Value()) + ")");
}

inline static std::string GetDisplayText(const ObjectId& id, const std::string& refTypeName)
{
	std::string displayText = "None";
	if (refTypeName == "GameObject")
	{
		displayText = GetObjectNameById(id);
	}
	else
	{
		displayText = GetComponentNameById(id);
	}
	displayText += "(" + refTypeName + ")"; // 参照先の型名を表示に追加
	return displayText;
}


namespace CurryEngine
{
	void ObjectIdDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
#ifdef USE_IMGUI
		ObjectId value = std::any_cast<ObjectId>(prop.getter(context.Primary()));
		bool mixed = PropertyDrawHelper::HasMixedValues<ObjectId>(context, prop);
		auto referenceAttr = prop.GetAttribute("ObjectReference");
		// Reference 属性の引数は参照先の型名 (例: "Transform", "GameObejct") を想定
		std::string refTypeName = referenceAttr && !referenceAttr->args.empty() ? referenceAttr->args[0] : "UnknownType"; // 参照先の型名を取得。属性がない場合や引数が空の場合は "UnknownType" とする
		if (!referenceAttr || refTypeName == "UnknownType") // ObjectReference 属性がない場合は警告を表示して描画をスキップ
		{
			LOG_WARNING("ObjectId property '" + prop.name + "' is missing 'ObjectReference' attribute or has invalid reference type. Please add [ObjectReference(\"TypeName\")] attribute to specify the reference type.");
			return;
		}

		bool referenceChanged = false; // 参照が変更されたかどうかを追跡するフラグ


		PropertyDrawHelper::BeginPropertyLabel(prop);

		if (auto referenceAttr = prop.GetAttribute("ObjectReference")) // ObjectReference 属性がある場合のみドロップを受け入れる
		{
			std::string displayText = GetDisplayText(value, refTypeName);
			displayText += "##" + prop.name; // 同じ名前のプロパティが複数ある場合に識別できるように ID を追加
			ImGui::Button(displayText.c_str()); // ドロップターゲットとして機能するボタンを描画

			if (ImGui::BeginDragDropTarget()) // ドロップ操作の受け入れを開始
			{
				// ドロップされたペイロードのタイプを定義（例: "ReferenceFieldName"）。ここでは referenceAttr の引数から参照先のコンポーネントの型名を取得して使用することを想定
				{
					const char* payloadType = (refTypeName).c_str(); // ドロップ可能なペイロードのタイプを定義（例: "ReferenceFieldName"）

					// 参照先の型が GameObject でない場合（Component型）は、GameObject のペイロードも受け入れるようにする。これにより、ユーザーは GameObject をドロップして、その GameObject に指定された型のコンポーネントがあれば自動的にそのコンポーネントを参照することができるようになる。
					if (refTypeName != "GameObject") // 参照先の型が GameObject でない場合は、GameObjectのペイロードも受け入れる
					{
						// -----------------------------------------------------------
						// Hierarchyから GameObject をドロップした場合、その GameObject に指定された型のコンポーネントがあれば自動的にそのコンポーネントを参照するようにする
						// -----------------------------------------------------------
						// TODO: スパゲッティコードになっているので、リファクタリングして整理すること。特に、ドロップされた GameObject から指定された型のコンポーネントを探す処理は、別の関数に切り出すなどして分かりやすくすること。
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) // "GameObject" タグのペイロードを受け入れる
						{
							if (payload->DataSize == sizeof(ObjectId)) // ペイロードのサイズが ObjectId と同じであることを確認
							{
								ObjectId droppedId = *reinterpret_cast<const ObjectId*>(payload->Data); // ペイロードから ObjectId を取得
								// ドロップされた GameObject に指定された型のコンポーネントがあればそのコンポーネントの ObjectId を取得
								if (auto* droppedObj = ObjectManager::Find(droppedId))
								{
									if (auto refComponent = droppedObj->GetComponentByTypeName(refTypeName))
									{
										value = refComponent->GetId(); // ドロップされた GameObject の指定された型のコンポーネントの ObjectId を使用
										referenceChanged = true; // 参照が変更されたことを記録
									}
									else
									{
										LOG_WARNING("Dropped GameObject does not have the required component type: " + refTypeName);
									}
								}
								else
								{
									LOG_WARNING("Dropped GameObject not found: " + std::to_string(droppedId.Value()));
								}
							}
						}
					}
					// -----------------------------------------------------------
					// 主に、Inspectorから 参照先の型と同じ型のコンポーネントをドロップした場合、そのコンポーネントを参照するようにする
					// -----------------------------------------------------------
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType)) // "ObjectReference" タグのペイロードを受け入れる
					{
						if (payload->DataSize == sizeof(ObjectId)) // ペイロードのサイズが ObjectId と同じであることを確認
						{
							ObjectId droppedId = *reinterpret_cast<const ObjectId*>(payload->Data); // ペイロードから ObjectId を取得
							value = droppedId; // フィールドにドロップされた ObjectId を設定
							referenceChanged = true; // 参照が変更されたことを記録
						}
					}
				}
				ImGui::EndDragDropTarget(); // ドロップ操作の受け入れを終了
			}
			if (!prop.GetAttribute("ReadOnly")) // ReadOnly 属性がない場合は、編集用の追加 UI を表示
			{
				// Xボタンで参照先をクリアできるようにする
				ImGui::SameLine();
				if (ImGui::Button(("X##clear" + std::string(prop.name)).c_str()))
				{
					ObjectId oldValue = value;
					value = ObjectId::Invalid(); // 参照をクリア
					referenceChanged = (oldValue != value); // 参照が変更されたことを記録
				}

				// ... ボタンで参照先を選択できるようにする
				ImGui::SameLine();
				if (ImGui::Button(("...##select" + std::string(prop.name)).c_str()))
				{
					ImGuiWindowFlags popupFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings;
					ImGui::OpenPopup(("Select Object##" + prop.name).c_str(), popupFlags);
				}

				// ポップアップが開いている場合のみ、オブジェクト選択用の UI を表示
				else if (ImGui::IsPopupOpen(("Select Object##" + prop.name).c_str()))
				{
					Scene* currentScene = SceneManager::GetLoadingSceneOrCurrentScene();
					if (currentScene) // ポップアップを開くフラグがセットされている場合のみポップアップを表示
					{
						const auto& allObjects = currentScene->GetObjectManager()->GetAll();
						if (ImGui::BeginPopup(("Select Object##" + prop.name).c_str()))
						{
							if (refTypeName == "GameObject")
							{
								// すべてのオブジェクトを選択肢にする
								for (const auto& obj : allObjects)
								{
									bool isSelected = (value == obj->id); // 現在の値とオブジェクトの ID が等しいかどうか
									ImGuiSelectableFlags flags = 0;
									if (ImGui::Selectable(obj->name.c_str(), isSelected, flags))
									{
										//!
										value = obj->id; // フィールドに選択されたオブジェクトの ID を設定
										referenceChanged = true; // 参照が変更されたことを記録
										ImGui::CloseCurrentPopup(); // 選択後にポップアップを閉じる
									}
								}
							}
							else
							{
								// referenceAttr の引数 refTypeName を使って、特定のコンポーネントを持つオブジェクトだけを選択肢にする
								for (const auto& obj : allObjects)
								{
									// オブジェクトが refTypeName のコンポーネントを持っているかどうかをチェック
									if (auto refComponent = obj->GetComponentByTypeName(refTypeName))
									{
										// Headerとして、owenerの名前を表示する
										ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen;
										if (obj->GetComponentsByTypeName(refTypeName).size() == 1)
										{
											nodeFlags |= ImGuiTreeNodeFlags_Leaf; // オブジェクトが refTypeName のコンポーネントを1つしか持っていない場合は、HeaderをLeafにする
											if (value == refComponent->id) // 現在の値とオブジェクトの ID が等しい場合は、Headerを選択状態にする
											{
												nodeFlags |= ImGuiTreeNodeFlags_Selected;
											}
										}
										if (ImGui::TreeNodeEx((obj->GetName() + "##" + std::to_string(obj->id.Value())).c_str(), nodeFlags))
										{
											if (nodeFlags & ImGuiTreeNodeFlags_Leaf) // オブジェクトが refTypeName のコンポーネントを1つしか持っていない場合は、そのコンポーネントを直接選択する
											{
												if (ImGui::IsItemActivated()) // Headerがアクティブになった場合（クリックされた場合）
												{
													value = refComponent->id; // フィールドに選択されたオブジェクトの ID を設定
													referenceChanged = true; // 参照が変更されたことを記録
													ImGui::CloseCurrentPopup(); // 選択後にポップアップを閉じる
												}
											}
											else
											{
												// オブジェクトの名前の下に、そのオブジェクトが持つ refTypeName のコンポーネントをすべて表示する
												auto components = obj->GetComponentsByTypeName(refTypeName);
												for (const auto& comp : components)
												{
													bool isSelected = (value == comp->id); // 現在の値とオブジェクトの ID が等しいかどうか
													ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
													if (isSelected)
													{
														flags |= ImGuiTreeNodeFlags_Selected;
													}
													if (ImGui::TreeNodeEx(comp->GetTypeName().c_str(), flags))
													{
														value = comp->id; // フィールドに選択されたコンポーネントの ID を設定
														referenceChanged = true; // 参照が変更されたことを記録
														ImGui::CloseCurrentPopup(); // 選択後にポップアップを閉じる
													}
												}
											}
										}
									}
								}
							}
							ImGui::EndPopup();
						}
					}
				}
			}
		}

		if (referenceChanged) // 参照が変更された場合のみ、プロパティの値を更新する
		{
			// 変更された値をすべての選択中のオブジェクトに適用
			PropertyDrawHelper::ApplyToAll(context, prop, value);

			// 変更されたことを記録
			PropertyDrawHelper::CommitEdit<ObjectId>(prop, context, m_state, value,
				[refTypeName](const ObjectId& id) { return GetDisplayText(id, refTypeName); }, // toStr: ObjectId を表示用の文字列に変換する関数
				[](const ObjectId& a, const ObjectId& b) { return a == b; }, // equals: 2つの ObjectId が等しいかどうかを比較する関数
				[]() { return false; }, // prevCheck: 前の値を保存するかどうかを判断する関数
				[]() { return true; }  // commitCheck: 変更をコミットするかどうかを判断する関数（ここでは常にコミットする）
			);
		}

#endif // USE_IMGUI

	}
}