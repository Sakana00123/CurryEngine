#include "pch.h"
#include "InspectorWindow.h"
#ifdef USE_IMGUI

#include "Engine/Core/ObjectManager.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Core/Component.h"
#include "Engine/Scenes/Scene.h"

#include "Engine/EditorSupport/PropertyDrawContext.h"

#include "Engine/EditorSupport/EditorSelection.h"

#include <imgui.h>
#include <profiler.h>
#include <Engine\Scripting\ScriptSystem.h>
#include <Engine\Core\ScriptComponent.h>
#include "AssetBrowser.h"

inline static void DrawLayerComboBox(GameObject* inspectorNode)
{
	auto layers = LayerManager::Get().GetLayerNames();
	std::vector<std::string> layerNames;
	for (int i = 0; i < layers.size(); ++i) {
		//空のレイヤーは表示しない
		if (layers[i].empty()) break;
		//レイヤー番号とレイヤー名を表示
		std::string layerName = "Layer" + std::to_string(i) + ": " + layers[i];
		layerNames.push_back(layerName);
	}
	int currentLayer = inspectorNode->GetLayer();
	//レイヤーのドロップダウン
	ImGui::Text("Layer");
	ImGui::SameLine();
	if (ImGui::BeginCombo("##Layer", layers[currentLayer].c_str()))
	{
		for (int i = 0; i < layerNames.size(); ++i) {
			bool isSelected = (currentLayer == i);
			if (ImGui::Selectable(layerNames[i].c_str(), isSelected)) {
				currentLayer = i;
				inspectorNode->SetLayer(currentLayer);
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		//レイヤーの管理画面に遷移する選択肢
		ImGui::Separator();
		if (ImGui::Selectable("Add Layer..."))
		{
			//レイヤー管理画面に遷移
			LayerManager::Get().OpenLayerSettingsGUI();
		}
	    ImGui::EndCombo();
	}
}

inline static void DrawInspectorLockCheckbox(ObjectManager* objectManager)
{
	bool lock = objectManager->IsInspectorLocked();
	if (ImGui::Checkbox("Lock", &lock))
	{
		objectManager->LockInspector(lock);
	}
}

inline static void DrawActiveCheckbox(GameObject* inspectorNode)
{
	bool isActive = inspectorNode->IsActiveSelf(); // ローカルの有効状態
	if (ImGui::Checkbox("", &isActive)) {
		inspectorNode->SetActive(isActive); // グローバルの有効状態を更新
	}
	ImGui::SameLine();
}

inline static void DrawNameInput(GameObject* inspectorNode)
{
	static size_t bufferSize = 256;
	static char buffer[256] = "";
	if (!ImGui::IsItemEdited()) {
		strncpy_s(buffer, inspectorNode->GetName().c_str(), bufferSize);
		buffer[bufferSize - 1] = '\0';
	}
	ImGui::Text("Name");
	ImGui::SameLine();
	ImGui::PushItemWidth(210);
	ImGui::InputText("##GameObjectName", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll);
	if (ImGui::IsItemEdited()) {
		inspectorNode->SetName(buffer);
	}
	ImGui::PopItemWidth();
}

//inline static void DrawTagInput(GameObject* inspectorNode)
//{
//    static size_t bufferSize = 256;
//    static char buffer[256] = "";
//    if (!ImGui::IsItemEdited()) {
//        strncpy_s(buffer, inspectorNode->GetTag().c_str(), bufferSize);
//        buffer[bufferSize - 1] = '\0';
//    }
//    ImGui::Text("Tag");
//    ImGui::SameLine();
//    ImGui::PushItemWidth(210);
//    ImGui::InputText("##GameObjectTag", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll);
//    if (ImGui::IsItemEdited()) {
//        inspectorNode->SetTag(buffer);
//    }
//    ImGui::PopItemWidth();
//}


inline static void DrawAddComponentButton(GameObject* inspectorNode, std::function<void(std::function<void(GameObject*)>)> applyToSelectedObjects)
{
    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup")) {
        static char searchBuffer[64] = "";
        ImGui::InputText("##search", searchBuffer, IM_ARRAYSIZE(searchBuffer));

        // 検索文字列
        std::string filter = searchBuffer;
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

        // カテゴリごとに表示
        std::unordered_map<std::string, std::vector<std::string>> categorized;
        for (auto& [name, entry] : ComponentFactory::GetAll()) {
            if (entry.attributes & ComponentAttributes::HideInAddComponentMenu) {
                continue; // Add Component メニューに表示しない属性がある場合はスキップ
            }
            categorized[entry.category].push_back(name);
        }

        // カテゴリごとに表示
        for (auto& [category, names] : categorized)
        {
            // カテゴリ名を小文字に変換してフィルタリング
            std::string lowerCategory = category;
            std::transform(lowerCategory.begin(), lowerCategory.end(), lowerCategory.begin(), ::tolower);

            // カテゴリ名がフィルタに一致するか、カテゴリ内のコンポーネント名がフィルタに一致するかをチェック
            bool showCategory = false;
            bool categoryMatches = filter.empty() || lowerCategory.find(filter) != std::string::npos;
            if (filter.empty() || categoryMatches) {
                showCategory = true;
            }
            else
            {
                for (auto& name : names)
                {
                    if (name == "ScriptComponent") {
                        continue; // スクリプトコンポーネントは後で別途表示
                    }
                    // 小文字変換してフィルタリング
                    std::string lowerName = name;
                    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

                    // フィルタに一致する名前があればカテゴリを表示
                    if (lowerName.find(filter) != std::string::npos) {
                        showCategory = true;
                        break;
                    }
                }
            }

            // フィルタに一致しないカテゴリはスキップ
            if (!showCategory) continue;

            // カテゴリ表示
            if (ImGui::TreeNodeEx(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                for (auto& name : names) {
                    std::string lowerName = name;
                    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

                    // フィルタに一致する名前がなければスキップ（カテゴリが一致している場合は表示）
                    if (!filter.empty() && lowerName.find(filter) == std::string::npos && !categoryMatches) {
                        continue; // フィルタに一致しない場合はスキップ
                    }

                    if (ImGui::Selectable(name.c_str())) {
                        // 選択されたコンポーネントをすべての選択中のオブジェクトに追加
                        applyToSelectedObjects([name](GameObject* obj)
                            {
                                auto component = ComponentFactory::Create(name);
                                obj->AttachComponent(name, component);
                                component->SetEnabled(true); // 追加したコンポーネントは有効にする
                                obj->InitializeComponent(component);
                            });

                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::TreePop();
            }
        }

        // スクリプトコンポーネントも表示
        if (ImGui::TreeNodeEx("Scripts", ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (const auto& scriptName : ScriptSystem::GetRegisteredScriptNames())
            {
                std::string lowerName = scriptName;
                std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                if (!filter.empty() && lowerName.find(filter) == std::string::npos)
                {
                    continue;
                }
                if (ImGui::Selectable(scriptName.c_str()))
                {
                    applyToSelectedObjects([scriptName](GameObject* obj)
                        {
                            std::shared_ptr<Component> component = std::make_shared<ScriptComponent>();
                            if (auto scriptComp = std::dynamic_pointer_cast<ScriptComponent>(component)) {
                                scriptComp->scriptName = scriptName;
                                obj->AttachComponent("ScriptComponent", component);
                                component->SetEnabled(true); // 追加したコンポーネントは有効にする
                                obj->InitializeComponent(component);
                            }
                        });
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::TreePop();
        }

        ImGui::EndPopup();
    }
}

inline static void DrawDropTarget(ImVec2 cursorPos, GameObject* inspectorNode, std::function<void(std::function<void(GameObject*)>)> applyToSelectedObjects)
{
    if (ImGui::GetDragDropPayload() && std::strcmp(ImGui::GetDragDropPayload()->DataType, "ASSET_PATH") == 0)
    {
        float scrollY = ImGui::GetScrollY();
        ImVec2 offset(0, scrollY); // スクロールオフセットを考慮
        ImGui::SetCursorPos(cursorPos + offset); // ドロップターゲットの位置を調整
        ImVec2 contentRegion = ImGui::GetContentRegionAvail(); // 利用可能な幅を取得
        ImVec2 size = ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin();

        ImGui::InvisibleButton("##drop_target", size); // 利用可能な領域全体をドロップターゲットにする

        // ドロップされたときの処理
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
                const char* p = static_cast<const char*>(payload->Data);
                std::filesystem::path path = p ? p : "";
                AssetType assetType = AssetBrowser::DetectAssetTypeFromFile(path);
                switch (assetType) {
                case AssetType::Script:
                {
                    std::string scriptName = path.stem().string();
                    applyToSelectedObjects([scriptName](GameObject* obj)
                        {
                            std::shared_ptr<Component> component = std::make_shared<ScriptComponent>();
                            if (auto scriptComp = std::dynamic_pointer_cast<ScriptComponent>(component)) {
                                scriptComp->scriptName = scriptName;
                                obj->AttachComponent("ScriptComponent", component);
                                component->SetEnabled(true); // 追加したコンポーネントは有効にする
                                obj->InitializeComponent(component);
                            }
                        });
                }
                break;
                default:
                    break;
                }
            }
            ImGui::EndDragDropTarget();
        }
    }
}

inline static void DrawInspectorHeader(GameObject* inspectorNode, ObjectManager* objectManager)
{
    ImGui::PushID(inspectorNode->GetId().Value());
    //Inspectorロック
    DrawInspectorLockCheckbox(objectManager);
    ImGui::SameLine();
    // layerを変更するドロップダウン
    DrawLayerComboBox(inspectorNode);
    //オブジェクトの有効状態を切り替えるチェックボックス
    DrawActiveCheckbox(inspectorNode);
    //名前を変更するテキストボックス
    DrawNameInput(inspectorNode);
    ImGui::PopID();
}

inline static void DrawInspectorProperties(GameObject* inspectorNode)
{
    ProfileScopedSection_3(0, (inspectorNode->name + " DrawProperty").c_str(), ImGuiControl::Profiler::Color::Green);


    ImGui::BeginChild("##Components", ImVec2(0, 0), true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImVec2 cursorPos = ImGui::GetCursorPos(); // 現在のカーソル位置を保存
    // コンポーネントごとに表示
    for (auto& primaryComp : inspectorNode->GetAllComponents()) {
        if (primaryComp->hideInspector) continue;

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.75f, 0.75f, 0.75f, 0.75f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.75f, 0.75f, 0.75f, 0.75f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

        // IDをプッシュしてTreeNodeExとポップアップを区別
        ImGui::PushID(primaryComp.get());

        // 有効/無効チェックボックス
        bool enable = primaryComp->IsEnabledSelf();
        if (ImGui::Checkbox("##enabled", &enable)) {
            primaryComp->SetEnabled(enable);
        }
        ImGui::SameLine(); // チェックボックスの右にTreeNodeを並べる

        // コンポーネント名のラベル
        std::string treeLabel = primaryComp->GetName();
        // スクリプトコンポーネントの場合、スクリプト名も表示
        if (treeLabel == "ScriptComponent")
        {
            if (auto scriptComp = std::dynamic_pointer_cast<ScriptComponent>(primaryComp))
            {
                treeLabel = scriptComp->GetTypeName() + " (" + treeLabel + ")";
            }
        }
        // コンポーネント名表示
        bool open = ImGui::TreeNodeEx(treeLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

        // ドラッグドロップの開始
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ImGui::SetDragDropPayload((primaryComp->GetTypeName()).c_str(), &primaryComp->id, sizeof(ObjectId*));
            ImGui::Text("%s", primaryComp->GetTypeName().c_str());
            ImGui::EndDragDropSource();
        }

        // 右クリックメニュー
        if (ImGui::BeginPopupContextItem("component_context_menu", ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::MenuItem("Remove")) {
                /*applyToSelectedObjects([component](GameObject* obj)
                    {
                        if (auto comp = obj->GetComponentByTypeName(component->GetTypeName())) {
                            obj->Destroy(comp.get());
                        }
                    });*/
            }
            ImGui::EndPopup();
        }

        if (open) {
            ImGui::Separator();
            if (!primaryComp->hideInspectorProperty)
            {
				const Scene* scene = inspectorNode ? inspectorNode->GetScene() : nullptr;
				const ObjectManager* objectManager = scene ? scene->GetObjectManager() : nullptr;
				const EditorSelection* editorSelection = objectManager ? objectManager->GetEditorSelection() : nullptr;

                if (editorSelection)
                {
					std::vector<std::shared_ptr<GameObject>> selection = editorSelection->GetAll();
					std::vector<Component*> sameTypeComps{ primaryComp.get() }; // 主選択のコンポーネントを最初に追加
                    
					// 同じコンポーネントを持つ選択中のオブジェクトを探す
                    for (const auto& selectedObj : selection)
                    {
                        if (selectedObj.get() == inspectorNode) continue; // 主選択はスキップ
                        if (auto comp = selectedObj->GetComponentByTypeName(primaryComp->GetTypeName())) {
                            sameTypeComps.push_back(comp.get());
                        }
					}

                    PropertyDrawContext context = sameTypeComps.size() > 1
                        ? PropertyDrawContext::MakeMulti(sameTypeComps)
						: PropertyDrawContext::MakeSingle(primaryComp.get());

                    primaryComp->DrawProperty(context);
                }
            }
            ImGui::TreePop();
        }

        ImGui::PopID();
        ImGui::PopStyleColor(3);
        ImGui::Separator();
    }

    //AddComponent
    {
        DrawAddComponentButton(inspectorNode, nullptr);
    }

    // ドラッグドロップの受け入れ(インスペクタ全体)
    // インスペクタ全体をドロップターゲットにするために、ウィンドウを覆う透明なドロップターゲットを作成
	DrawDropTarget(cursorPos, inspectorNode, nullptr);

    ImGui::EndChild();

}



namespace CurryEngine
{


	void InspectorWindow::Draw(ObjectManager* objectManager)
	{
		if (ImGui::Begin("Inspector"))
		{
			if (GameObject* inspectorNode = objectManager->GetInspectorNode())
			{
                DrawInspectorHeader(inspectorNode, objectManager);
                ImGui::Separator();
				DrawInspectorProperties(inspectorNode);
			}
		}
		ImGui::End();
	}
}
#endif // USE_IMGUI