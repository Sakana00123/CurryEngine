#include "pch.h"
#include "GameObject.h"

#include "ScriptComponent.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Utils/UniqueIdGenerator.h"
#include "Engine/Core/Reflection/Meta.h"
#include "Engine/Core/Reflection/TypeSerializerRegistry.h"
#include "Engine/Scripting/ScriptSystem.h"
#include "Engine/EditorSupport/OrderManager.h"
#include "Engine/EditorSupport/EditorSelection.h"
#include <Engine\UI\RectTransform.h>

#include <profiler.h>

GameObject::~GameObject() {
    for (auto child : children) {
        child->parent = nullptr;
    }
	if (_components.size() > 0)
    {
        OnDestroy();
    }
    SetParent(nullptr);
}

void GameObject::Create(const std::string& name) {
    this->name = MakeUniqueName(name);
    isCreated = true;
    transform = AddComponent<Transform>();
}

void GameObject::SetName(const std::string& newName) {
    if (name == newName) return; // 名前が同じ場合は何もしない
    std::string uniqueName = MakeUniqueName(newName);
    name = uniqueName;
}

std::string GameObject::MakeUniqueName(const std::string& name) {
    std::string baseName = name;

    // 名前の末尾が "(数字)" になっているかチェックしてベース名を抽出
    if (!baseName.empty() && baseName.back() == ')') {
        size_t openParen = baseName.find_last_of('(');
        if (openParen != std::string::npos) {
            bool isAllDigit = true;
            // カッコの中身がすべて数字か確認
            for (size_t j = openParen + 1; j < baseName.size() - 1; ++j) {
                if (!std::isdigit(static_cast<unsigned char>(baseName[j]))) {
                    isAllDigit = false;
                    break;
                }
            }
            // 中身が数字のみであればベース名からカッコ部分を削除
            if (isAllDigit && openParen + 1 < baseName.size() - 1) {
                size_t trimPos = openParen;
                // カッコの前の半角スペースも削除
                while (trimPos > 0 && baseName[trimPos - 1] == ' ') {
                    trimPos--;
                }
                baseName = baseName.substr(0, trimPos);
            }
        }
    }

    // まず引数で渡された名前が衝突していないか確認し、使用可能であればそのまま返す
    std::string result = name;
    if (!ObjectManager::Find(result)) {
        return result;
    }

    // 衝突している場合はベース名に " (番号)" を付けて空いている番号を探す
    int i = 1;
    while (true) {
        result = baseName + " (" + std::to_string(i) + ")";
        if (!ObjectManager::Find(result)) {
            break;
        }
        i++;
    }
	return result;
}

void GameObject::SetParent(GameObject* newParent) {
    if (parent) { //すでに親が設定されていたら、その親の子から自身を削除
        auto& children = parent->children;
        children.erase(std::remove(children.begin(), children.end(), this), children.end());
    }
    parent = newParent;
    if (parent) {
        newParent->children.push_back(this);
    }
	// 親子関係が変わったので、再計算が必要な状態にする
	if (Transform* transform = GetTransform())
    {
        transform->MarkNeedsUpdate();
    }

	// 階層構造の変更に伴い、アクティブ状態を更新する
	this->RefreshActiveInHierarchy();
}

void GameObject::BeginFrame()
{
	ProfileScopedSection_2(0, "GameObject::BeginFrame", ImGuiControl::Profiler::Color::Green);
    for (size_t i = 0; i < _components.size(); i++) {
        std::weak_ptr<Component> weakComp = _components.at(i);
        if (const auto& component = weakComp.lock()) {
            if (component->IsEnabled())
            {
				ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Yellow);
                component->BeginFrame();
            }
        }
	}

	// 破棄予約されたコンポーネントを削除
    if (!removes.empty()) {

        // 削除前にコンポーネントのOnDestroyを呼び出す
        for (const auto& comp : removes) {
            if (comp) {
                comp->OnDestroy();
				comp->SetEnabled(false); // 無効化してから削除する
            }
        }

        // 削除対象のコンポーネントをリストから削除
        for (const auto& comp : removes) {
            _components.erase(std::remove_if(_components.begin(), _components.end(),
                [&comp](const std::shared_ptr<Component>& c) { return c == comp; }),
                _components.end());
        }
        removes.clear();
    }
}

void GameObject::EndFrame()
{
	ProfileScopedSection_2(0, "GameObject::EndFrame", ImGuiControl::Profiler::Color::Green);
    for (size_t i = 0; i < _components.size(); i++) {
        std::weak_ptr<Component> weakComp = _components.at(i);
        if (const auto& component = weakComp.lock()) {
            if (component->IsEnabled())
            {
				ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Yellow);
                component->EndFrame();
            }
        }
    }
}

void GameObject::Update(float deltaTime)
{
    //優先度でソート
    //std::sort(_components.begin(), _components.end(),
    //    [](const std::shared_ptr<Component>& a, const std::shared_ptr<Component>& b) {
    //        if (a == nullptr && b == nullptr)
    //            return false;
    //        if (a == nullptr) return false;
    //        if (b == nullptr) return true;
    //        return a->priority < b->priority;
    //    });
	// ソートの安定性を保つため、OrderManager::Sort を使用してソートする
    {
		ProfileScopedSection_3(0, "GameObject::Update - Sort Components", ImGuiControl::Profiler::Color::Green);
        CurryEngine::OrderManager::Sort(_components);
    }

    if (IsActive()) {
		ProfileScopedSection_2(0, "GameObject::Update", ImGuiControl::Profiler::Color::Green);
        for (size_t i = 0; i < _components.size(); i++) {
            std::weak_ptr<Component> weakComp = _components.at(i);
            if (const auto& component = weakComp.lock()) {
				if (SceneManager::state == SceneManager::State::Playing || 
                    (SceneManager::state == SceneManager::State::Editing && (component->GetAttributeFlags() & ComponentAttributes::ExecuteInEditMode)))
                {
                    if (component->IsEnabled())
                    {
						if (component->m_started == false) { // Start() がまだ呼び出されていない場合は呼び出す
							ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Yellow);
                            component->Start();
                            component->m_started = true;
						}
						else { // Start() が呼び出された後は Update() を呼び出す(Start() が呼び出されたフレームでは Update() は呼び出さない)
							ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Blue);
                            component->Update(deltaTime);
                        }
                    }
                }
            }
        }
    }
}

void GameObject::LateUpdate(float deltaTime) {
    if (IsActive()) {
		ProfileScopedSection_2(0, "GameObject::LateUpdate", ImGuiControl::Profiler::Color::Green);
        for (size_t i = 0; i < _components.size(); i++) {
            std::weak_ptr<Component> weakComp = _components.at(i);
            if (const auto& component = weakComp.lock()) {
                if (component->IsEnabled())
                {
					ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Purple);

                    component->LateUpdate(deltaTime);
                }
            }
        }
    }
}

void GameObject::FixedUpdate(float fixedDeltaTime) {
    if (IsActive()) {
		ProfileScopedSection_2(0, "GameObject::FixedUpdate", ImGuiControl::Profiler::Color::Green);
        for (size_t i = 0; i < _components.size(); i++) {
            std::weak_ptr<Component> weakComp = _components.at(i);
            if (const auto& component = weakComp.lock()) {
                if (component->IsEnabled())
                {
					ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Purple);

                    component->FixedUpdate(fixedDeltaTime);
                }
            }
        }
    }
}

void GameObject::BeginRendering(RenderContext* rtx) {
    if (IsActive()) {
		ProfileScopedSection_2(0, "GameObject::BeginRendering", ImGuiControl::Profiler::Color::Green);
        for (size_t i = 0; i < _components.size(); i++) {
			std::weak_ptr<Component> weakComp = _components.at(i);
            if (const auto& component = weakComp.lock()) {
                if (component->IsEnabled())
                {
					ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Yellow);
                    component->BeginRendering(rtx);
                }
            }
        }
    }
}
void GameObject::Render(RenderContext* rtx) {
    if (IsActive()) {
        ProfileScopedSection_2(0, "GameObject::Render", ImGuiControl::Profiler::Color::Green);
        for (size_t i = 0; i < _components.size(); i++) {
            std::weak_ptr<Component> weakComp = _components.at(i);
            if (const auto& component = weakComp.lock()) {
                if (component->IsEnabled())
                {
					ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Red);
                    component->Render(rtx);
                }
            }
        }
    }
}
void GameObject::EndRendering(RenderContext* rtx) {
    if (IsActive()) {
		ProfileScopedSection_2(0, "GameObject::EndRendering", ImGuiControl::Profiler::Color::Yellow);
        for (size_t i = 0; i < _components.size(); i++) {
            std::weak_ptr<Component> weakComp = _components.at(i);
            if (const auto& component = weakComp.lock()) {
                if (component->IsEnabled())
                {
					ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Yellow);
                    component->EndRendering(rtx);
                }
            }
        }
    }
}
void GameObject::Begin(RenderContext* rtx) {
    if (IsActive()) {
		ProfileScopedSection_2(0, "GameObject::Begin", ImGuiControl::Profiler::Color::Green);
        for (size_t i = 0; i < _components.size(); i++) {
            std::weak_ptr<Component> weakComp = _components.at(i);
            if (const auto& component = weakComp.lock()) {
                if (component->IsEnabled())
                {
					ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Yellow);
                    component->Begin(rtx);
                }
            }
        }
    }
}
//すべてのコンポーネントの2D描画処理
void GameObject::Draw(RenderContext* rtx) {
    if (IsActive()) {
		ProfileScopedSection_2(0, "GameObject::Draw", ImGuiControl::Profiler::Color::Green);
        for (size_t i = 0; i < _components.size(); i++) {
            std::weak_ptr<Component> weakComp = _components.at(i);
            if (const auto& component = weakComp.lock()) {
                if (component->IsEnabled())
                {
					ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Red);
                    component->Draw(rtx);
                }
            }
        }
    }
}
//すべてのコンポーネントの2D描画後処理
void GameObject::End(RenderContext* rtx) {
    if (IsActive()) {
		ProfileScopedSection_2(0, "GameObject::End", ImGuiControl::Profiler::Color::Green);
        for (size_t i = 0; i < _components.size(); i++) {
            std::weak_ptr<Component> weakComp = _components.at(i);
            if (const auto& component = weakComp.lock()) {
                if (component->IsEnabled())
                {
                    ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Yellow);
                    component->End(rtx);
                }
            }
        }
    }
}
//すべてのコンポーネントの破棄コールバック処理
void GameObject::OnDestroy() {
    ProfileScopedSection_3(0, (name + " OnDestroy").c_str(), ImGuiControl::Profiler::Color::Red);
    
    for (size_t i = 0; i < _components.size(); i++) {
        std::weak_ptr<Component> weakComp = _components.at(i);
        if (const auto& component = weakComp.lock()) {
            component->OnDestroy();
            component->Finalize();
			component->SetEnabled(false); // 破棄されたコンポーネントは無効にする
        }
    }
	// 破棄後にコンポーネントリストをクリア
	_components.clear();
}
//すべてのコンポーネントのインスペクタ描画
void GameObject::DrawProperty() {
#ifdef USE_IMGUI

	ProfileScopedSection_3(0, (name + " DrawProperty").c_str(), ImGuiControl::Profiler::Color::Green);

    // 選択中のすべてのオブジェクトに対して処理を適用するヘルパー関数
    auto applyToSelectedObjects = [&](const std::function<void(GameObject*)>& func) {
        bool applied = false;
        if (this->scene && this->scene->objectManager && this->scene->objectManager->selection) {
            auto& selectAll = this->scene->objectManager->selection->GetAll();
            if (!selectAll.empty()) {
                for (auto& obj : selectAll) {
                    if (auto go = obj.get()) {
                        func(go);
                    }
                }
                applied = true;
            }
        }
        if (!applied) {
            func(this);
        }
        };


	ImGui::BeginChild("##Components", ImVec2(0, 0), true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImVec2 cursorPos = ImGui::GetCursorPos(); // 現在のカーソル位置を保存
	// コンポーネントごとに表示
    for (auto& component : _components) {
        if (component->hideInspector) continue;

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.75f, 0.75f, 0.75f, 0.75f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.75f, 0.75f, 0.75f, 0.75f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

        // IDをプッシュしてTreeNodeExとポップアップを区別
        ImGui::PushID(component.get());

        // 有効/無効チェックボックス
        bool enable = component->IsEnabledSelf();
        if (ImGui::Checkbox("##enabled", &enable)) {
            component->SetEnabled(enable);
        }
        ImGui::SameLine(); // チェックボックスの右にTreeNodeを並べる

		// コンポーネント名のラベル
        std::string treeLabel = component->GetName();
		// スクリプトコンポーネントの場合、スクリプト名も表示
        if (treeLabel == "ScriptComponent")
        {
            if (auto scriptComp = std::dynamic_pointer_cast<ScriptComponent>(component))
            {
                treeLabel = scriptComp->GetTypeName() + " (" + treeLabel + ")";
            }
		}
		// コンポーネント名表示
        bool open = ImGui::TreeNodeEx(treeLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

		// ドラッグドロップの開始
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ImGui::SetDragDropPayload((component->GetTypeName()).c_str(), &component->id, sizeof(ObjectId*));
            ImGui::Text("%s", component->GetTypeName().c_str());
            ImGui::EndDragDropSource();
		}

        // 右クリックメニュー
        if (ImGui::BeginPopupContextItem("component_context_menu", ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::MenuItem("Remove")) {
                applyToSelectedObjects([component](GameObject* obj)
                    {
                        if (auto comp = obj->GetComponentByTypeName(component->GetTypeName())) {
                            obj->Destroy(comp.get());
                        }
                    });
            }
            ImGui::EndPopup();
        }

        if (open) {
            ImGui::Separator();
            if (!component->hideInspectorProperty)
                component->DrawProperty();
            ImGui::TreePop();
        }

        ImGui::PopID();
        ImGui::PopStyleColor(3);
        ImGui::Separator();
    }

    //AddComponent
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

	// ドラッグドロップの受け入れ(インスペクタ全体)
	// インスペクタ全体をドロップターゲットにするために、ウィンドウを覆う透明なドロップターゲットを作成
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

	ImGui::EndChild();
#endif // USE_IMGUI
}

bool GameObject::IsActive() const
{
	return activeInHierarchy;
}

void GameObject::SetActive(bool set)
{
	activeSelf = set;
	RefreshActiveInHierarchy();
}

bool GameObject::IsActiveSelf() const
{
    return activeSelf;
}

void GameObject::RefreshActiveInHierarchy()
{
    bool previousActiveInHierarchy = activeInHierarchy;

    // 親が非アクティブの場合は影響を受けない
    activeInHierarchy = activeSelf && (parent ? parent->activeInHierarchy : true);

	// シーン開始前ならこれ以降の処理は行わない
    if (GetScene() && !GetScene()->IsStarted()) {
        return;
	}

	// シーンが開始されている状態で、アクティブになってたらAwakeを呼び出す
	if (activeInHierarchy)
    {
		AwakeComponents(); // アクティブ状態が変化したタイミングでAwakeを呼び出す
	}

	// アクティブ状態が変化した場合、コンポーネントに伝播
    for (size_t i = 0; i < _components.size(); i++)
    {
        std::weak_ptr<Component> weakComp = _components.at(i);
        if (const auto& component = weakComp.lock())
        {
            RefreshComponentActive(component.get());
		}
    }

    // 子オブジェクトにも伝播
    for (GameObject* child : children)
    {
        child->RefreshActiveInHierarchy();
    }
}

void GameObject::RefreshComponentActive(Component* component)
{
    if (GetScene() && !GetScene()->IsStarted()) {
        return; // シーン開始前はコンポーネントのアクティブ状態を更新しない
	}

	if (const auto& comp = component)
    {
		bool oldEnable = comp->enabledInGame;
		bool newEnable = IsActive() && comp->enabledSelf;
        // アクティブ状態が変化した場合にコールバックを呼び出す
        if (oldEnable != newEnable)
        {
			// コンポーネントのアクティブ状態を更新
			comp->enabledInGame = newEnable;

			// アクティブ状態に応じてコールバックを呼び出す
            if (newEnable)
                comp->OnEnable();
            else
                comp->OnDisable();
        }
	}
}

void GameObject::Destroy(float delay)
{
    if (Scene* scene = SceneManager::GetCurrentScene()) {
        scene->Destroy(this->name);

		destroyDelay = delay; // 破棄までの遅延時間を設定

		// 遅延時間が0以下の場合は即座に破棄する (epsilon以下の値を0とみなす)
		if (delay <= FLT_EPSILON)
        {
            for (auto& component : _components) {
                removes.push_back(component);
            }
        }
    }
}

void GameObject::Destroy(Component* component)
{
    for (auto& p : _components) {
        if (p.get() == component) {
            removes.push_back(p);
        }
    }
}


void GameObject::SetLayer(int layer)
{
    this->layer = layer;
}

void GameObject::AttachComponent(const std::string& name, std::shared_ptr<Component>& component, bool generateId)
{
	std::string sectionName = GetName() + " AttachComponent: " + name;
	ProfileScopedSection_3(0, sectionName.c_str(), ImGuiControl::Profiler::Color::Green);

	int tailPriority = _components.empty()
        ? 0
		: _components.back()->GetPriority() + CurryEngine::OrderManager::STEP; // 追加されたコンポーネントの優先度を、現在の最後尾のコンポーネントの優先度よりも大きくする
	component->SetPriority(tailPriority);

    _components.emplace_back(component);
    component->SetOwner(this);
	component->SetName(name);
}

void GameObject::InitializeComponent(std::shared_ptr<Component>& component)
{
    //初期化処理
	if (!component->m_initialized)
    {
		ProfileScopedSection_3(0, (name + " Initialize: " + component->name).c_str(), ImGuiControl::Profiler::Color::Green);
        component->Initialize();
        component->m_initialized = true; // 初期化フラグを立てる
    }
}

void GameObject::AwakeComponents()
{
	ProfileScopedSection_2(0, "GameObject::AwakeComponents", ImGuiControl::Profiler::Color::Green);

    for (size_t i = 0; i < _components.size(); i++) {
        std::weak_ptr<Component> weakComp = _components.at(i);
        if (const auto& component = weakComp.lock()) {
            if (!component->m_awaked) {
				ProfileScopedSection_3(0, component->name.c_str(), ImGuiControl::Profiler::Color::Green);
                component->Awake();
                component->m_awaked = true; // Awakeが呼び出されたフラグを立てる
            }
        }
	}
}

json GameObject::Serialize() const {

	ProfileScopedSection_3(0, name.c_str(), ImGuiControl::Profiler::Color::Green);

    json j;
	// バージョン情報を追加(将来の互換性のため)
	j["version"] = (int)CurryEngine::GameObjectSerializeVersion::Latest;

	// 基本的なプロパティを手動でシリアライズ
    j["id"] = id.ToString();
	j["layer"] = layer;
    //j["isActive"] = activeSelf;
    // リフレクションシステムを使用したシリアライズ
	std::string className = "GameObject";
#if 0
    while (auto* meta = ReflectionRegistry::FindClass(className)) {
        for (const auto& prop : meta->properties) {
            const char* base = reinterpret_cast<const char*>(this);
            const char* propAddr = base + prop.offset;
            const auto* typeInfo = TypeSerializerRegistry::Find(prop.type);
            if (typeInfo) {
                json value;
                typeInfo->serialize(propAddr, value);
                j[prop.name] = value;
            }
        }
        // 継承元クラスも処理
        if (meta->base.empty())
            break;
        className = meta->base;
    }
#else
	std::vector<std::string> toVisit = { className }; // 訪問予定のクラス名のスタック
	std::unordered_set<std::string> visited; // 訪問済みクラスのセット
    while (!toVisit.empty()) {
        std::string currentClass = toVisit.front();
		toVisit.erase(toVisit.begin());
        if (visited.count(currentClass)) {
            continue; // すでに訪問済みの場合はスキップ
		}
        visited.insert(currentClass);
        auto* meta = ReflectionRegistry::FindClass(currentClass);
        if (!meta) {
            Console::LogWarning("Reflection metadata not found for class: " + currentClass);
            continue; // メタデータが見つからない場合はスキップ
		}

		// 各プロパティを走査
        for (const auto& prop : meta->properties) {
            const char* base = reinterpret_cast<const char*>(this);
            const char* propAddr = base + prop.offset;
            const auto* typeInfo = TypeSerializerRegistry::Find(prop.type);
            if (typeInfo) {
                json value;
                typeInfo->serialize(propAddr, value);
                j[prop.name] = value;
            }
        }

		// 継承元クラスも訪問予定に追加
        for (const auto& base : meta->bases) {
            if (!base.empty()) {
                toVisit.push_back(base);
			}
		}

	}

#endif // 0


    if (parent) {
        j["parent"] = parent->id.ToString();
    } else {
		j["parent"] = ObjectId::Invalid().ToString(); // 親がいない場合は無効なIDを設定
    }
    j["components"] = json::array();
    for (const auto& component : _components) {
        if (component) {
            json compJson = component->Serialize();
            compJson["type"] = component->name;
			compJson["id"] = component->id.ToString();
			// Reflectionによるプロパティシリアライズ
			std::string compName = component->name;
#if 0
            while (auto* meta = ReflectionRegistry::FindClass(compName))
            {
                // 各プロパティを走査
                for (const auto& prop : meta->properties)
                {
                    if (prop.GetAttribute("NonSerialized"))
                    {
                        continue; // NonSerialized属性がある場合はスキップ
                    }

                    // オフセットからプロパティのアドレスを取得(componentの先頭アドレス + オフセット)
                    const char* base = reinterpret_cast<const char*>(component.get());
                    const char* propAddr = base + prop.offset;

                    // プロパティの型に対応するシリアライザを取得
                    const auto* typeInfo = TypeSerializerRegistry::Find(prop.type);
                    if (!typeInfo)
                    {
                        Console::LogWarning("Unsupported type: " + prop.type + " for property: " + prop.name + " in component: " + component->name);
                        continue;
                    }

                    // シリアライズ実行
                    json value;
                    typeInfo->serialize(propAddr, value);
                    compJson[prop.name] = value;
                }

                // 継承元クラスも処理
                for (const auto& base : meta->bases)
                {
                    if (!base.empty())
                    {
                        toVisit.push_back(base);
                        break;
                    }
                }
            }
#else
            std::vector<std::string> toVisit = { compName }; // 訪問予定のクラス名のスタック
            std::unordered_set<std::string> visited; // 訪問済みクラスのセット
            while (!toVisit.empty()) {
                std::string currentClass = toVisit.front();
                toVisit.erase(toVisit.begin());
                if (visited.count(currentClass)) {
                    continue; // すでに訪問済みの場合はスキップ
                }
                visited.insert(currentClass);
                auto* meta = ReflectionRegistry::FindClass(currentClass);
                if (!meta) {
                    Console::LogWarning("Reflection metadata not found for class: " + currentClass);
                    continue; // メタデータが見つからない場合はスキップ
                }

                // 各プロパティを走査
                for (const auto& prop : meta->properties)
                {
                    if (prop.GetAttribute("NonSerialized"))
                    {
                        continue; // NonSerialized属性がある場合はスキップ
                    }

                    // オフセットからプロパティのアドレスを取得(componentの先頭アドレス + オフセット)
                    const char* base = reinterpret_cast<const char*>(component.get());
                    const char* propAddr = base + prop.offset;

                    // プロパティの型に対応するシリアライザを取得
                    const auto* typeInfo = TypeSerializerRegistry::Find(prop.type);
                    if (!typeInfo)
                    {
                        Console::LogWarning("Unsupported type: " + prop.type + " for property: " + prop.name + " in component: " + component->name);
                        continue;
                    }

                    // シリアライズ実行
                    json value;
                    typeInfo->serialize(propAddr, value);
                    compJson[prop.name] = value;
                }

                // 継承元クラスも訪問予定に追加
                for (const auto& base : meta->bases) {
                    if (!base.empty()) {
                        toVisit.push_back(base);
                    }
                }

            }
#endif // 0


			// オブジェクトとしてシリアライズされた場合のみ追加
            if (compJson.is_object())
            {
				// コンポーネントが登録されてるか確認
                if (!ComponentFactory::Exists(component->name))
                {
					// 登録されてないコンポーネントの場合はログを出してスキップ
					Console::LogError("Object::Serialize: Unknown component type: " + component->name);
                    continue;
                }
				else // 登録されているコンポーネントの場合は配列に追加
                {
                    j["components"].push_back(compJson);
                }
            }
        }
    }
    return j;
}

void GameObject::Deserialize(const json& j)
{
	ProfileScopedSection_3(0, name.c_str(), ImGuiControl::Profiler::Color::Green);

    // バージョン情報を取得
    version = static_cast<int>(
        j.value("version", static_cast<int>(CurryEngine::GameObjectSerializeVersion::Legacy)));

	// 名前、ID、アクティブ状態の復元

    if (j.contains("id")) {
        if (j["id"].is_number_integer()) {
            id = ObjectId::FromLegacy(j["id"].get<int>());
        }
        else {
            // 新フォーマット: uint64 文字列
			id = ObjectId::FromString(j["id"].get<std::string>());
        }
    }

    if (j.contains("layer")) {
        layer = j["layer"].get<int>();
	}

	// リフレクションシステムを使用したデシリアライズ
    std::string className = "GameObject";
#if 0
    while (auto* meta = ReflectionRegistry::FindClass(className)) {
        for (const auto& prop : meta->properties) {
            if (!j.contains(prop.name))
                continue;
            char* base = reinterpret_cast<char*>(this);
            char* propAddr = base + prop.offset;
            const auto* typeInfo = TypeSerializerRegistry::Find(prop.type);
            if (typeInfo) {
                const json& value = j[prop.name];
                typeInfo->deserialize(propAddr, value);
            }
        }
        // 継承元クラスも処理
        if (meta->base.empty())
            break;
        className = meta->base;
    }
#else
    std::vector<std::string> toVisit = { className }; // 訪問予定のクラス名のスタック
    std::unordered_set<std::string> visited; // 訪問済みクラスのセット
    while (!toVisit.empty()) {
        std::string currentClass = toVisit.front();
        toVisit.erase(toVisit.begin());
        if (visited.count(currentClass)) {
            continue; // すでに訪問済みの場合はスキップ
        }
        visited.insert(currentClass);
        auto* meta = ReflectionRegistry::FindClass(currentClass);
        if (!meta) {
            Console::LogWarning("Reflection metadata not found for class: " + currentClass);
            continue; // メタデータが見つからない場合はスキップ
        }

        // 各プロパティを走査
        for (const auto& prop : meta->properties) {
            char* base = reinterpret_cast<char*>(this);
            char* propAddr = base + prop.offset;
            const auto* typeInfo = TypeSerializerRegistry::Find(prop.type);
            if (typeInfo) {
                if (!j.contains(prop.name))
                    continue; // JSONにプロパティが存在しない場合はスキップ
                const json& value = j[prop.name];
                typeInfo->deserialize(propAddr, value);
            }
        }

        // 継承元クラスも訪問予定に追加
        for (const auto& base : meta->bases) {
            if (!base.empty()) {
                toVisit.push_back(base);
            }
        }

    }
#endif // 0


	bool isActiveSelf = this->activeSelf; // デフォルトは現在のactiveSelfの値
    if (j.contains("isActive")) {
        isActiveSelf = j["isActive"].get<bool>();
    }
	// アクティブ状態の設定
	SetActive(isActiveSelf); // activeSelfを先に設定

	// 優先度の復元
	ObjectId parentId = ObjectId::Invalid();
    if (j.contains("parent")) {
        if (j["parent"].is_number_integer()) {
            parentId = ObjectId::FromLegacy(j["parent"].get<int>());
        }
        else {
            // 新フォーマット: uint64 文字列
            parentId = ObjectId::FromString(j["parent"].get<std::string>());
		}
    }


    isCreated = true;
    //親オブジェクトの設定は、すべてのオブジェクトが生成された後に行う
    if (parentId.IsValid()) {
        pendingParentID = parentId;
    }
}

void GameObject::DeserializeComponents(const json& j, const std::unordered_map<ObjectId, ObjectId>& idMap) {
    if (j.contains("components")) {
        for (const auto& compJson : j["components"]) {
            if (compJson.contains("type")) {
                std::string type = compJson["type"].get<std::string>();

				// 作成する前に、属性チェックを行う
				auto& allComponents = ComponentFactory::GetAll();
                auto it = allComponents.find(type);
				ComponentFactory::Entry* entry = it != allComponents.end() ? &it->second : nullptr;
				if (entry)
                {
					// DisallowMultiple属性がある場合、すでに同じ型のコンポーネントが存在しないかチェック
                    if (entry->attributes & ComponentAttributes::DisallowMultiple) {
                        bool exists = false;
                        for (const auto& comp : _components) {
                            if (comp->name == type) {
                                exists = true;
                                break;
                            }
                        }
                        if (exists) {
                            Console::LogError("GameObject::DeserializeComponents: Multiple components of type " + type + " are not allowed. Skipping.");
                            continue; // 同じ型のコンポーネントがすでに存在する場合はスキップ
						}
                    }
				}
				auto component = ComponentFactory::Create(type);

                if (!component)
                {
                    Console::LogError("GameObject::DeserializeComponents: Failed to create component of type: " + type);
                    continue;
				}

				// TODO: コンポーネントIDの復元と重複チェック。将来的にはIDのマッピングも必要になるかもしれない。
				std::unordered_set<ObjectId>& existingComponentIds = GetScene()->objectManager->GetExistingComponentIds();

				// IDの復元
				if (compJson.contains("id"))
                {
                    ObjectId oldCompId;
                    if (compJson["id"].is_number_integer()) {
                        oldCompId = ObjectId::FromLegacy(compJson["id"].get<int>());
                    }
					else {
                        oldCompId = ObjectId::FromString(compJson["id"].get<std::string>());
                    }

                    if (!idMap.empty() && idMap.find(oldCompId) != idMap.end()) {
                        component->id = idMap.at(oldCompId); // 複製などの場合のIDマッピングを適用
                    }
                    else {
                        component->id = oldCompId;
                        if (!component->id.IsValid()) {
                            component->id = ObjectId::Generate(); // IDが無効な場合は新たに生成
                        }
                        if (existingComponentIds.count(component->GetId()) > 0) {
                            component->id = ObjectId::Generate(); // IDが重複している場合は新たに生成
                        }
                    }
                }
				existingComponentIds.insert(component->GetId()); // IDを既存IDセットに追加

				// Transformコンポーネントは特別扱いしてGameObjectのtransformメンバにセット
                if (type == "Transform") {
					transform = std::dynamic_pointer_cast<Transform>(component).get();
				}
                else if (type == "RectTransform") {
					transform = std::dynamic_pointer_cast<RectTransform>(component).get();
                }

                // コンポーネントのセットアップ
                AttachComponent(type, component, false);
				int priority = component->GetPriority();

                // コンポーネントのデシリアライズ
                component->Deserialize(compJson);

                // Reflectionによるプロパティデシリアライズ
                std::string compName = type;

#if 0
                while (auto* meta = ReflectionRegistry::FindClass(compName))
                {
                    // 各プロパティを走査
                    for (const auto& prop : meta->properties)
                    {
                        // JSONにプロパティが存在するか確認
                        if (!compJson.contains(prop.name))
                            continue;
                        if (prop.GetAttribute("NonSerialized"))
                            continue; // NonSerialized属性がある場合はスキップ

                        // オフセットからプロパティのアドレスを取得(componentの先頭アドレス + オフセット)
                        char* base = reinterpret_cast<char*>(component.get());
                        char* propAddr = base + prop.offset;
                        // プロパティの型に対応するシリアライザを取得
                        const auto* typeInfo = TypeSerializerRegistry::Find(prop.type);
                        if (!typeInfo)
                        {
                            Console::LogWarning("Unsupported type: " + prop.type + " for property: " + prop.name + " in component: " + type);
                            continue;
                        }
                        // デシリアライズ実行
                        const json& value = compJson[prop.name];
                        typeInfo->deserialize(propAddr, value);
                    }
                    // 継承元クラスも処理
                    if (meta->base.empty())
                        break;
                    compName = meta->base;
                }
#else
                std::vector<std::string> toVisit = { compName }; // 訪問予定のクラス名のスタック
                std::unordered_set<std::string> visited; // 訪問済みクラスのセット
                while (!toVisit.empty()) {
                    std::string currentClass = toVisit.front();
                    toVisit.erase(toVisit.begin());
                    if (visited.count(currentClass)) {
                        continue; // すでに訪問済みの場合はスキップ
                    }
                    visited.insert(currentClass);
                    auto* meta = ReflectionRegistry::FindClass(currentClass);
                    if (!meta) {
                        Console::LogWarning("Reflection metadata not found for class: " + currentClass);
                        continue; // メタデータが見つからない場合はスキップ
                    }

                    // 各プロパティを走査
                    for (const auto& prop : meta->properties)
                    {
                        // JSONにプロパティが存在するか確認
                        if (!compJson.contains(prop.name))
                            continue;
                        if (prop.GetAttribute("NonSerialized"))
                            continue; // NonSerialized属性がある場合はスキップ

                        // オフセットからプロパティのアドレスを取得(componentの先頭アドレス + オフセット)
                        char* base = reinterpret_cast<char*>(component.get());
                        char* propAddr = base + prop.offset;
                        // プロパティの型に対応するシリアライザを取得
                        const auto* typeInfo = TypeSerializerRegistry::Find(prop.type);
                        if (!typeInfo)
                        {
                            Console::LogWarning("Unsupported type: " + prop.type + " for property: " + prop.name + " in component: " + type);
                            continue;
                        }
                        // デシリアライズ実行
                        const json& value = compJson[prop.name];
                        typeInfo->deserialize(propAddr, value);
                    }

                    // 継承元クラスも訪問予定に追加
                    for (const auto& base : meta->bases) {
                        if (!base.empty()) {
                            toVisit.push_back(base);
                        }
                    }

                }
#endif // 0

				component->SetPriority(priority); // デシリアライズ前の優先度を復元

                // 最初の初期化処理
                InitializeComponent(component);
            }
        }
    }

    if (GetScene() && GetScene()->IsStarted())
    {
        AwakeComponents(); // シーンがすでに開始されている場合はAwakeも呼び出す
		RefreshActiveInHierarchy(); // アクティブ状態を更新して、コンポーネントのOnEnable/OnDisableを正しく呼び出す
    }
}