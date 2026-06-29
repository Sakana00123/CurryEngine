#include "pch.h"
#include "HierarchyWindow.h"

#ifdef USE_IMGUI
#include "Engine/Core/ObjectManager.h"
#include "Engine/Core/GameObject.h"
#include "Engine/EditorSupport/EditorSelection.h"
#include "Engine/Scenes/Scene.h"
#include <imgui_internal.h>
#include <Engine\UI\RectTransform.h>
#include <Engine\EditorSupport\OrderManager.h>

#include "Engine/Scenes/SceneManager.h"
#include "Engine/Editor/AssetBrowser.h"
#include <Engine\Factory\GameObjectFactory.h>
#include "Dialog.h"
#include "EditorGUI.h"

namespace
{
	// オブジェクトの階層構造を考慮して、親を持たないオブジェクトのみを返す
	static std::vector<std::weak_ptr<GameObject>> OrganizeObjects(const std::vector<std::shared_ptr<GameObject>>& objects, GameObject* parent)
	{
		std::vector<std::weak_ptr<GameObject>> organized;
		for (const auto& obj : objects) {
			std::weak_ptr<GameObject> weakObj = obj;
			if (weakObj.expired()) continue;
			if (weakObj.lock()->parent == parent) {
				organized.push_back(obj);
			}
		}
		return organized;
	}

	static void AddChildren(std::vector<std::shared_ptr<GameObject>>& result, const std::shared_ptr<GameObject>& obj) {
		if (!obj) return;
		result.push_back(obj);
		for (const auto& child : obj->children) {
			std::shared_ptr<GameObject> sharedObj = ObjectManager::Find_Ptr(child->GetId());
			if (sharedObj) {
				AddChildren(result, sharedObj);
			}
		}
	}

	static std::vector<std::shared_ptr<GameObject>> OrganizeTreeNodes(const std::vector<std::shared_ptr<GameObject>>& objects)
	{
		std::vector<std::weak_ptr<GameObject>> organized = OrganizeObjects(objects, nullptr);
		std::vector<std::shared_ptr<GameObject>> result;

		for (const auto& obj : organized) {
			AddChildren(result, obj.lock());
		}
		return result;
	}
}


namespace CurryEngine
{
	void HierarchyWindow::Draw(ObjectManager* objectManager)
	{
		if (!objectManager) return;
		auto scene = SceneManager::GetCurrentScene();
		auto selection = objectManager->GetEditorSelection();
		auto& objects = objectManager->GetAllMutable();
		auto selectNode = selection->GetPrimary().get();
		if (ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_None))
		{
			auto* window = ImGui::GetCurrentWindow();
			bool isNotDestroyObject = false;
			bool isDroppedGameObjectThisFrame = false;
			int i = 0;

			auto DrawDropZone = [&](GameObject* target, int& idCounter, bool accept, bool appendToEnd = false)
				{
					ImGui::PushID(idCounter++);

					// 高さ6pxの空間を確保（カーソルは進む）
					ImVec2 pos = ImGui::GetCursorScreenPos();
					float w = ImGui::GetContentRegionAvail().x;
					float h = 6.0f;
					ImGui::Dummy(ImVec2(w, h));

					if (accept && ImGui::BeginDragDropTarget())
					{
						// ホバー中のみライン表示
						if (ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + w, pos.y + h)))
						{
							ImGui::GetWindowDrawList()->AddLine(
								ImVec2(pos.x, pos.y + h * 0.5f),
								ImVec2(pos.x + w, pos.y + h * 0.5f),
								IM_COL32(100, 180, 255, 255), 2.0f
							);
						}
						if (ImGui::AcceptDragDropPayload("GameObject"))
						{
							m_pendingDrop = PendingDrop{ target, /*reorder=*/true, /*appendToEnd=*/appendToEnd };
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::PopID();
				};


			std::function<void(GameObject*)> DrawNodeTree = [&](GameObject* object)
				{
					// オブジェクトが削除されている可能性があるためチェック
					if (!object) return;

					ImGui::PushID(i++);

					//矢印をクリックで階層を開く。当たり判定は余白も含める
					ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
						| ImGuiTreeNodeFlags_FramePadding
						| ImGuiTreeNodeFlags_SpanAvailWidth;

					// デフォルトで階層を開いておくかどうか
					if (object->isDefaultOpenOnHierarchy) {
						nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
					}

					//子がいない場合は矢印をつけない
					size_t childCount = object->children.size();
					if (childCount == 0) {
						nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					}

					//選択フラグ
					if (selection->IsSelected(object)) {
						nodeFlags |= ImGuiTreeNodeFlags_Selected;
					}
					if (isDroppedGameObjectThisFrame) {
						nodeFlags |= ImGuiTreeNodeFlags_Selected;
					}

					//PersistentObjectManagerに登録されているオブジェクトかどうか
					bool acceptDrop = false;

					//ドロップ先（親子関係を解除したいとき、もしくは優先度の並び替えのとき）にドロップ可能かどうか
					if (selection)
					{
						for (auto& selectObj : selection->GetAll()) {
							// ドロップソースのオブジェクトがUIオブジェクトの場合、ドロップ先がUIオブジェクトでないと親子関係を構築できないようにする
							if (selectObj && selectObj->GetComponent<RectTransform>()) {
								if (!object->GetComponent<RectTransform>()) {
									if (object->GetParent() != selectObj->GetParent())
										acceptDrop = false;
								}
								break;
							}
						}
					}


					DrawDropZone(object, i, acceptDrop);

					//ツリーノードを描画
					float alpha = 1;//uniqueId-isVisible
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, alpha));
					bool isActive = object->IsActiveSelf();
					if (ImGui::Checkbox(/*std::to_string(i + 1).c_str()*/ "", &isActive))
						object->SetActive(isActive);
					ImGui::PopStyleColor();
					ImGui::SameLine();
					//ImGui::Text(std::to_string(object->id.Value()).c_str());
					//ImGui::SameLine();
#if 0
				// 優先度表示
					ImGui::Text("%d", object->priority);
					ImGui::SameLine();
#endif // 0
					float textColor = object->IsActive() ? 1.0f : 0.5f;
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(textColor, textColor, textColor, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
					// ノードを描画。開いているかどうかを返す
					bool opened = ImGui::TreeNodeEx(object, nodeFlags, object->GetName().c_str());
					object->isDefaultOpenOnHierarchy = opened; //開いているかどうかを保存しておく

					//ノードに対してドラッグ（親子関係構築）
					if (acceptDrop && ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
							IM_ASSERT(payload->DataSize == sizeof(ObjectId*));
							//IM_ASSERT(payload->DataSize == sizeof(EditorSelection*));
							ObjectId* pRef = static_cast<ObjectId*>(payload->Data);
							if (pRef)
							{
								m_pendingDrop = PendingDrop{
									.target = object,
									.reorder = false
								};
								isDroppedGameObjectThisFrame = true;
							}
						}
						ImGui::EndDragDropTarget();
					}
					//GameObject*データとしてドラッグ
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
						ImGui::SetDragDropPayload("GameObject", &object->id, sizeof(ObjectId*));
						ImGui::Text(std::format("Dragging {} object(s)", selection->GetAll().size()).c_str());
						/*for (auto& notDestroyObject : PersistentObjectManager::GetObjects()) {
							if (notDestroyObject->GetId() == selectNode->GetId()) {
								draggingObjectIsNotDestroyObject = true;
								break;
							}
						}*/

						ImGui::EndDragDropSource();
					}
					//フォーカスされたノードを選択する
					bool shift = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
					bool ctrl = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
					//static bool delayClick = false;
					if ((ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) && !ImGui::IsItemToggledOpen())
					{
						if (selection)
						{
							/*if (selection->IsSelected(object))
							{
								delayClick = true;
							}
							else*/
							{
								const std::shared_ptr<GameObject>& spObject = objectManager->Find_Ptr(object->GetId());
								//if (shift) // Shiftキーが押されている場合は、クリックしたオブジェクトから現在の選択範囲までを選択する
								//	selection->SelectRange(spObject, OrganizeTreeNodes(objects), /*additive=*/ctrl);
								//else // Shiftキーが押されていない場合は、クリックしたオブジェクトを選択する。Ctrlキーが押されている場合は、選択に追加する。押されていない場合は、選択を置き換える
								//	selection->Select(spObject, /*additive=*/ctrl);
								if (shift) selection->SelectRange(spObject, OrganizeTreeNodes(objects), ctrl);
								else selection->SelectTemp(spObject, ctrl);
							}
						}
					}
					// クリックしてからマウスを動かしても選択されないように、クリック後のフレームで選択する
					bool isReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right);
					//if (delayClick && ImGui::IsItemHovered() && isReleased)
					//{
					//	if (selection)
					//	{
					//		const std::shared_ptr<GameObject>& spObject = Find_Ptr(object->GetId());
					//		if (shift) selection->SelectRange(spObject, OrganizeTreeNodes(objects));
					//		else
					//		{
					//			if (ctrl) {
					//				selection->Select(spObject, /*additive=*/true);
					//			}
					//			else
					//			{
					//				selection->Select(spObject, /*additive=*/false);
					//				SelectInspectorNode(spObject.get());
					//			}
					//		}
					//	}
					//	delayClick = false;
					//}

					ImGui::PopStyleColor(4);

					// 選択されているノードをInspectorに表示する
					bool flag = ImGui::IsItemActive() || ImGui::IsItemHovered(); //ノードがアクティブまたはホバーされているか
					bool isActiveAndHovered = ImGui::IsItemActive() && ImGui::IsItemHovered(); //ノードがアクティブかつホバーされているか
					
					if (/*selectNode && */flag && isReleased) {
						if (!ImGui::GetDragDropPayload()) {
							// 選択されているノードをInspectorに表示する
							selection->CommitTempSelection(ctrl);
							//SelectInspectorNode(selectNode);
							objectManager->SelectInspectorNode(selection->GetPrimary().get());
						}
					}

					// ダブルクリックでそのオブジェクトのフォーカスに移動
					if (selectNode && (isActiveAndHovered) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						if (selectNode->transform && scene)
						{
							Vector3 pos = (selectNode->transform->GetWorldPosition());
							scene->GetSceneViewEditorCamera()->SetPosition(pos);
						}
					}

					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Priority: %d\nID: %d", object->GetPriority(), object->GetId().Value());
					}

					// ノードIDを戻す
					ImGui::PopID();
					//開かれている場合、子階層にも同じ処理をする
					if (opened && childCount > 0) {
						if (object) {
							for (GameObject* child : object->children) {
								DrawNodeTree(child);
							}
						}
						DrawDropZone(object, i, acceptDrop);
						ImGui::TreePop();
					}
				};

			// 空白クリックで選択解除
			if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && ImGui::IsWindowHovered())
			{
				// ノード上ではなく、ウィンドウの余白がクリックされた場合
				if (!ImGui::IsAnyItemHovered())
				{
					objectManager->Reset();
				}
			}

			// --- 右クリックメニュー ---
			if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("HierarchyContextMenu");
			}

			//ドロップ先（親子関係を解除したいとき）
			bool acceptDrop = true;
			if (selection)
			{
				for (auto& selectObj : selection->GetAll()) {
					// ドロップソースのオブジェクトがUIオブジェクトの場合、ドロップ先がUIオブジェクトでないと受け入れない
					if (selectObj && selectObj->GetComponent<RectTransform>()) {
						acceptDrop = false;
						break;
					}
				}
			}
			if (acceptDrop)
			{
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
						IM_ASSERT(payload->DataSize == sizeof(ObjectId*));
						ObjectId* pRef = static_cast<ObjectId*>(payload->Data);
						if (pRef)
						{
							for (auto& pObj : selection->GetAll())
							{
								GameObject* obj = pObj.get();
								obj->SetParent(nullptr);
							}
						}
					}
					ImGui::EndDragDropTarget();
				}
			}

			for (auto& object : objects) {
				//開かれている場合、子階層にも同じ処理をする
				if (!object || object->parent) continue;
				DrawNodeTree(object.get());
			}

			// リスト末尾の番兵DropZone
			// ドロップ先として「最後のルートオブジェクト」を使う
			{
				GameObject* lastRoot = nullptr;
				for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
					if (*it && !(*it)->parent) { lastRoot = it->get(); break; }
				}
				bool sentinelAccept = true;
				if (selection) {
					for (auto& selectObj : selection->GetAll()) {
						if (selectObj && selectObj->GetComponent<RectTransform>()) {
							sentinelAccept = false; break;
						}
					}
				}
				if (lastRoot) {
					DrawDropZone(lastRoot, i, sentinelAccept, true);
				}
			}

			if (m_pendingDrop.has_value() && selection)
			{
				auto& drop = m_pendingDrop.value();
				for (auto& pObj : selection->GetAll())
				{
					GameObject* obj = pObj.get();
					if (!obj) continue;

					// ドロップ先が同じ親を持つオブジェクトの並び替えかどうかは、ドロップ先とドロップ元の親が同じかどうかで判断する
					if (drop.reorder)
					{
						if (obj->GetParent() != drop.target->GetParent())
							obj->SetParent(drop.target->GetParent());

						auto itObj = std::find_if(objects.begin(), objects.end(),
							[&](const auto& o) { return o.get() == obj; });
						if (itObj == objects.end()) continue;
						size_t fromIdx = std::distance(objects.begin(), itObj);

						size_t toIdx = 0;
						if (drop.appendToEnd)
						{
							// ドロップ先の親と同じ親を持つオブジェクトの数を数える
							toIdx = objects.size();
						}
						else
						{
							auto itTarget = std::find_if(objects.begin(), objects.end(),
								[&](const auto& o) { return o.get() == drop.target; });

							if (itTarget == objects.end()) continue;
							toIdx = std::distance(objects.begin(), itTarget);
						}
						CurryEngine::OrderManager::MoveObject(objects, fromIdx, toIdx);
					}
					else
					{
						// ドロップ先がドロップ元の子であるかどうかを確認するために、ドロップ先の親をたどっていく
						bool isChild = false;
						GameObject* parent = drop.target;
						while (parent)
						{
							if (parent == obj)
							{
								isChild = true;
								break;
							}
							parent = parent->GetParent();
						}
						// ドロップ先がドロップ元の子でない場合のみ親子関係を構築する
						if (!isChild) {
							obj->SetParent(drop.target); // 親子関係構築
						}
					}
				}
				m_pendingDrop.reset();
			}

			isNotDestroyObject = true;

#if 0
			//DontDestroyOnLoadで保持されているオブジェクト
			if (!PersistentObjectManager::GetObjects().empty()) {
				if (ImGui::TreeNodeEx("Don't Destroy Objects", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) {
					for (auto& object : PersistentObjectManager::GetObjects()) {
						//開かれている場合、子階層にも同じ処理をする
						if (!object || object->parent) continue;
						DrawNodeTree(object.get());
					}
					ImGui::TreePop();
				}
			}
#endif // 0


			//アセットブラウザからドラッグアンドドロップでモデルインスタンス生成
			if (window)
			{
				if (ImGui::BeginDragDropTargetCustom(window->Rect(), window->ID))
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
						const char* p = static_cast<const char*>(payload->Data);
						std::filesystem::path path = p ? p : "";
						AssetType assetType = AssetBrowser::DetectAssetTypeFromFile(path);
						Scene* currentScene = SceneManager::GetCurrentScene();
						switch (assetType)
						{
						case AssetType::Unknown:
							break;
						case AssetType::Texture:
							GameObjectFactory::CreateImage(currentScene, path.stem().string(), nullptr, path.wstring().c_str());
							break;
						case AssetType::Model:
							GameObjectFactory::CreateModel(currentScene, path.stem().string(), path.string());
							break;
						case AssetType::Sound:
							GameObjectFactory::CreateAudioSource(currentScene, path.stem().string(), path.wstring().c_str());
							break;
						case AssetType::Scene:
						{
							SceneManager::ChangeScene(path.stem().string());
							break;
						}
						case AssetType::Prefab:
						{
							json prefabJson;
							JsonFileHandler::LoadJsonFromFile(prefabJson, path.string(), JsonIOFormat::Binary);
							GameObject* newObject = objectManager->Instantiate(prefabJson);
							newObject->SetParent(selectNode);
							break;
						}
						default:
							break;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}

			// --- 右クリックメニューの中身 ---
			if (selection)
			{
				if (ImGui::BeginPopup("HierarchyContextMenu"))
				{
					// 選択中のノードに対するメニュー
					if (!selection->IsEmpty())
					{
						// 削除ボタン
						if (ImGui::MenuItem("Delete", "Del", false))
						{
							auto& selectAll = selection->GetAll();
							for (int i = static_cast<int>(selectAll.size()) - 1; i >= 0; --i)
							{
								if (selectAll[i])
								{
									objectManager->Destroy(selectAll[i]->GetName());
								}
							}
							selection->Clear();
						}
						// 複製ボタン
						if (ImGui::MenuItem("Duplicate", "Ctrl+D", false))
						{
							auto selectAll = selection->GetAll();
							selection->Clear();
							for (auto& pObj : selectAll)
							{
								if (GameObject* newObject = objectManager->Duplicate(pObj.get()))
								{
									selection->Select(objectManager->Find_Ptr(newObject->GetId()), true);
									// 複製したオブジェクトをInspectorに表示する
									objectManager->SelectInspectorNode(newObject);
								}
							}
						}
						//// 優先度変更ボタン
						//if (ImGui::MenuItem("Increase Priority", "Alt+Up", false))
						//{
						//	for (auto& pObj : selection->GetAll())
						//	{
						//		int oldPriority = pObj->priority;

						//		pObj->priority++;
						//	}
						//}
						//if (ImGui::MenuItem("Decrease Priority", "Alt+Down", false))
						//{
						//	for (auto& pObj : selection->GetAll())
						//	{
						//		pObj->priority--;
						//	}
						//}
						// プレハブ化ボタン
						// TODO: プレハブ化を右クリックメニューからではなく、ドラッグアンドドロップでできるようにする。（複数選択のときの挙動が難しいため。）
						if (ImGui::MenuItem("Create Prefab", "", false))
						{
							if (selectNode)
							{
								char buffer[256] = "";
								if (Dialog::SaveFileName(buffer, 256, "Prefab Files\0*.prefab\0All Files\0*.*\0", "prefab") == DialogResult::OK)
								{
									// 拡張子を.prefabに変更
									std::filesystem::path savePath = buffer;
									savePath.replace_extension(".prefab");
									// プレハブとして保存
									objectManager->SaveGameObject(selectNode, savePath.string());
								}
							}
						}
					}

					EditorGUI::DrawGameObjectMenu();
					ImGui::EndPopup();
				}

				if (!selection->IsEmpty())
				{
					// 削除のショートカットキー
					if (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_RouteFocused))
					{
						// コピーを作成してループ中のリスト変更による問題を回避する
						auto selectAll = selection->GetAll();
						for (auto& pObj : selectAll)
						{
							if (pObj)
							{
								objectManager->Destroy(pObj->GetName());
							}
						}
						selection->Clear(); // 処理後に選択をクリア
					}
					// 優先度変更のショートカットキー
					/*if (ImGui::Shortcut(ImGuiMod_Alt | ImGuiKey_UpArrow, ImGuiInputFlags_RouteFocused))
					{
						for (auto& pObj : selection->GetAll())
						{
							if (pObj)
							{
								pObj->priority++;
							}
						}
					}
					if (ImGui::Shortcut(ImGuiMod_Alt | ImGuiKey_DownArrow, ImGuiInputFlags_RouteFocused))
					{
						for (auto& pObj : selection->GetAll())
						{
							if (pObj)
							{
								pObj->priority--;
							}
						}
					}*/
					// 複製のショートカットキー
					if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_D, ImGuiInputFlags_RouteFocused))
					{
						auto& selectAll = selection->GetAll();
						selection->Clear();
						for (auto& pObj : selectAll)
						{
							if (pObj)
							{
								if (GameObject* newObject = objectManager->Duplicate(pObj.get()))
								{
									selection->Select(objectManager->Find_Ptr(newObject->GetId()), true);
									// 複製したオブジェクトをInspectorに表示する
									objectManager->SelectInspectorNode(newObject);
								}
							}
						}
					}
				}
			}

		}
		ImGui::End();
	}
}

#endif // USE_IMGUI
