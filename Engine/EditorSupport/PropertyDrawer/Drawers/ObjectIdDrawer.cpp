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


namespace CurryEngine
{
	void ObjectIdDrawer::Draw(const PropertyInfo& prop, const PropertyDrawContext& context)
	{
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

		PropertyDrawHelper::BeginPropertyLabel(prop);
		
		if (auto referenceAttr = prop.GetAttribute("ObjectReference")) // ObjectReference 属性がある場合のみドロップを受け入れる
		{
			std::string displayText = "None";
			if (refTypeName == "GameObject")
			{
				const auto& refObj = ObjectManager::Find(value);
				if (refObj)
				{
					const auto& refObjName = refObj ? refObj->GetName() : "Unknown";
					displayText = (value).IsValid() ? refObjName : "None"; // 参照先のオブジェクト名を表示に追加
				}
				else
				{
					displayText = (value).IsValid() ? ("Unknown(" + std::to_string((value).Value()) + ")") : "None"; // 参照先の情報が見つからない場合の表示
				}
			}
			else
			{
				const auto& componentCacheMap = SceneManager::GetLoadingSceneOrCurrentScene()->GetObjectManager()->GetComponentCacheMap();
				auto it = componentCacheMap.find(value);
				if (it != componentCacheMap.end())
				{
					if (auto refComponent = it->second.lock())
					{
						const auto& refObj = refComponent->GetOwner();
						const auto& refObjName = refObj ? refObj->GetName() : "Unknown";
						displayText = (value).IsValid() ? refObjName : "None"; // 参照先のオブジェクト名を表示に追加
					}
					else
					{
						displayText = (value).IsValid() ? ("Unknown(" + std::to_string((value).Value()) + ")") : "None"; // 参照先の情報が見つからない場合の表示
					}
				}
				else
				{
					displayText = (value).IsValid() ? ("Unknown(" + std::to_string((value).Value()) + ")") : "None"; // 参照先の情報が見つからない場合の表示
				}
			}
			displayText += "(" + refTypeName + ")"; // 参照先の型名を表示に追加
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
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) // "GameObject" タグのペイロードを受け入れる
						{
							if (payload->DataSize == sizeof(ObjectId)) // ペイロードのサイズが ObjectId と同じであることを確認
							{
								ObjectId droppedId = *reinterpret_cast<const ObjectId*>(payload->Data); // ペイロードから ObjectId を取得
								ObjectId newComponentId = ObjectId::Invalid(); // ドロップされた GameObject から取得したコンポーネントの ObjectId を保持する変数
								// ドロップされた GameObject に指定された型のコンポーネントがあればそのコンポーネントの ObjectId を取得
								if (auto* droppedObj = ObjectManager::Find(droppedId))
								{
									if (auto refComponent = droppedObj->GetComponentByTypeName(refTypeName))
									{
										newComponentId = refComponent->GetId(); // ドロップされた GameObject の指定された型のコンポーネントの ObjectId を使用
									}
									else
									{
										Console::LogWarning("Dropped GameObject does not have the required component type: " + refTypeName);
									}
								}
								else
								{
									Console::LogWarning("Dropped GameObject not found: " + std::to_string(droppedId.Value()));
								}
								value = newComponentId; // フィールドにドロップされたコンポーネントの ObjectId を設定
								std::string refTypeDisplay = (refTypeName == "GameObject") ? "GameObject" : ("Component(" + refTypeName + ")");
								const auto& droppedObj = ObjectManager::Find(newComponentId);
								const auto& droppedObjName = droppedObj ? droppedObj->GetName() : "Unknown";

								CurryEngine::PropertyDrawHelper::CommitEdit<ObjectId>(prop, context, m_state, newComponentId,
									[refTypeDisplay](const ObjectId& v) -> std::string {
										if (v.IsValid())
										{
											const auto& prevObj = ObjectManager::Find(v);
											const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
											return std::to_string(v.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")";
										}
										else
										{
											return std::string("None");
										}
									});


								//const auto& prevObj = ObjectManager::Find(prevValue);
								//const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
								//std::string newValueLog = newComponentId.IsValid() ? (std::to_string(newComponentId.Value()) + "(" + refTypeDisplay + ": " + droppedObjName + ")") : "None";
								//std::string prevValueLog = prevValue.IsValid() ? (std::to_string(prevValue.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")") : "None";
								///*IMGUI_PROPERTY_COMMAND(label, ObjectId, newComponentId, prevValue,
								//	newValueLog,
								//	prevValueLog);*/
								//prevValue = newComponentId; // 前回の値を更新
							}
						}
					}
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType)) // "ObjectReference" タグのペイロードを受け入れる
					{
						if (payload->DataSize == sizeof(ObjectId)) // ペイロードのサイズが ObjectId と同じであることを確認
						{
							ObjectId droppedId = *reinterpret_cast<const ObjectId*>(payload->Data); // ペイロードから ObjectId を取得
							value = droppedId; // フィールドにドロップされた ObjectId を設定
							std::string refTypeDisplay = (refTypeName == "GameObject") ? "GameObject" : ("Component(" + refTypeName + ")");
							const auto& droppedObj = ObjectManager::Find(droppedId);
							const auto& droppedObjName = droppedObj ? droppedObj->GetName() : "Unknown";

							CurryEngine::PropertyDrawHelper::CommitEdit<ObjectId>(prop, context, m_state, droppedId,
								[refTypeDisplay](const ObjectId& v) -> std::string {
									if (v.IsValid())
									{
										const auto& prevObj = ObjectManager::Find(v);
										const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
										return std::to_string(v.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")";
									}
									else
									{
										return std::string("None");
									}
								});


							/*const auto& prevObj = ObjectManager::Find(prevValue);
							const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
							std::string newValueLog = droppedId.IsValid() ? (std::to_string(droppedId.Value()) + "(" + refTypeDisplay + ": " + droppedObjName + ")") : "None";
							std::string prevValueLog = prevValue.IsValid() ? (std::to_string(prevValue.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")") : "None";*/

							/*IMGUI_PROPERTY_COMMAND(label, ObjectId, droppedId, prevValue,
								newValueLog,
								prevValueLog);*/
							//prevValue = droppedId; // 前回の値を更新
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

					const std::string refTypeDisplay = (refTypeName == "GameObject") ? "GameObject" : ("Component(" + refTypeName + ")");
					const auto& prevObj = ObjectManager::Find(oldValue);
					const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
					const auto& newValueLog = "None";
					const auto& prevValueLog = oldValue.IsValid() ? (std::to_string(oldValue.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")") : "None";

					CurryEngine::PropertyDrawHelper::CommitEdit<ObjectId>(prop, context, m_state, ObjectId::Invalid(),
						[](const ObjectId& v) -> std::string {
							return "None";
						});

					/*IMGUI_PROPERTY_COMMAND(label, ObjectId, ObjectId::Invalid(), oldValue,
						newValueLog,
						prevValueLog);*/
					//prevValue = ObjectId::Invalid(); // 前回の値を更新
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
										//ObjectId oldValue = value;
										//value = obj->id; // フィールドに選択されたオブジェクトの ID を設定

										//std::string refTypeDisplay = "GameObject";
										//const auto& prevObj = ObjectManager::Find(oldValue);
										//const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
										//const auto& newValueLog = std::to_string(obj->id.Value()) + "(" + refTypeDisplay + ": " + obj->GetName() + ")";
										//const auto& prevValueLog = oldValue.IsValid() ? (std::to_string(oldValue.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")") : "None";

										PropertyDrawHelper::CommitEdit<ObjectId>(prop, context, m_state, obj->id,
											[](const ObjectId& v) -> std::string {
												if (v.IsValid())
												{
													const auto& prevObj = ObjectManager::Find(v);
													const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
													return std::to_string(v.Value()) + "(" + "GameObject" + ": " + prevObjName + ")";
												}
												else
												{
													return std::string("None");
												}
											});

										/*IMGUI_PROPERTY_COMMAND(label, ObjectId, obj->id, oldValue,
											std::to_string(obj->id.Value()),
											std::to_string(oldValue.Value()));*/
										//prevValue = obj->id; // 前回の値を更新
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
													//ObjectId oldValue = value;
													//value = refComponent->id; // フィールドに選択されたオブジェクトの ID を設定
													std::string refTypeDisplay = "Component(" + refTypeName + ")";
													//const auto& prevObj = ObjectManager::Find(oldValue);
													//const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
													//const auto& newValueLog = std::to_string(refComponent->id.Value()) + "(" + refTypeDisplay + ": " + obj->GetName() + ")";
													//const auto& prevValueLog = oldValue.IsValid() ? (std::to_string(oldValue.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")") : "None";

													PropertyDrawHelper::CommitEdit<ObjectId>(prop, context, m_state, refComponent->id,
														[refTypeDisplay](const ObjectId& v) -> std::string {
															if (v.IsValid())
															{
																const auto& prevObj = ObjectManager::Find(v);
																const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
																return std::to_string(v.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")";
															}
															else
															{
																return std::string("None");
															}
														});

													/*IMGUI_PROPERTY_COMMAND(label, ObjectId, refComponent->id, oldValue,
														newValueLog,
														prevValueLog);*/
													//prevValue = refComponent->id; // 前回の値を更新
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
														//ObjectId oldValue = value;
														//value = comp->id; // フィールドに選択されたオブジェクトの ID を設定

														std::string refTypeDisplay = "Component(" + refTypeName + ")";
														//const auto& prevObj = ObjectManager::Find(oldValue);
														//const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
														//const auto& newValueLog = std::to_string(comp->id.Value()) + "(" + refTypeDisplay + ": " + obj->GetName() + ")";
														//const auto& prevValueLog = oldValue.IsValid() ? (std::to_string(oldValue.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")") : "None";

														PropertyDrawHelper::CommitEdit<ObjectId>(prop, context, m_state, comp->id,
															[refTypeDisplay](const ObjectId& v) -> std::string {
																if (v.IsValid())
																{
																	const auto& prevObj = ObjectManager::Find(v);
																	const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
																	return std::to_string(v.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")";
																}
																else
																{
																	return std::string("None");
																}
															});

														/*IMGUI_PROPERTY_COMMAND(label, ObjectId, comp->id, oldValue,
															newValueLog,
															prevValueLog);*/
														//prevValue = comp->id; // 前回の値を更新
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


		/*PropertyDrawHelper::CommitEdit<ObjectId>(prop, context, m_state, value,
			[](const ObjectId& v) {
				return std::to_string(v.Value());
			});*/
	}
}