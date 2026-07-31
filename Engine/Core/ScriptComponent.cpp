#include "pch.h"
#include "ScriptComponent.h"

#include "Engine/Scripting/ScriptSystem.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(ScriptComponent, "Scripts", ComponentAttributes::HideInAddComponentMenu, {})

void ScriptComponent::OnEnable()
{
    if (m_gcHandle)
    {
        ScriptSystem::OnEnableScript(m_gcHandle);
    }
}

void ScriptComponent::OnDisable()
{
    if (m_gcHandle)
    {
        ScriptSystem::OnDisableScript(m_gcHandle);
    }
}

void ScriptComponent::OnCollisionEnter(const CollisionInfo& info)
{
    if (m_gcHandle)
    {
        ScriptSystem::OnCollisionEnterScript(m_gcHandle, info);
    }
}

void ScriptComponent::OnCollisionStay(const CollisionInfo& info)
{
    if (m_gcHandle)
    {
        ScriptSystem::OnCollisionStayScript(m_gcHandle, info);
    }
}

void ScriptComponent::OnCollisionExit(const CollisionInfo& info)
{
    if (m_gcHandle)
    {
        ScriptSystem::OnCollisionExitScript(m_gcHandle, info);
    }
}

void ScriptComponent::OnTriggerEnter(const TriggerInfo& info)
{
    if (m_gcHandle)
    {
        ScriptSystem::OnTriggerEnterScript(m_gcHandle, info);
    }
}

void ScriptComponent::OnTriggerStay(const TriggerInfo& info)
{
    if (m_gcHandle)
    {
        ScriptSystem::OnTriggerStayScript(m_gcHandle, info);
    }
}

void ScriptComponent::OnTriggerExit(const TriggerInfo& info)
{
    if (m_gcHandle)
    {
        ScriptSystem::OnTriggerExitScript(m_gcHandle, info);
    }
}

void ScriptComponent::Initialize()
{
    if (scriptName.empty()) return; // スクリプト名が空の場合は何もしない

    // 既存インスタンスを破棄
    OnScriptUnload();

    // C# Behaviour インスタンスを生成
    uint64_t ownerId = GetOwner()->GetId().Value();
    uint64_t componentId = GetId().Value();
    m_gcHandle = ScriptSystem::CreateScript(scriptName, ownerId, componentId);

    if (m_gcHandle)
    {
        // 保持しているフィールド値を適用
        if (!m_pendingFields.empty())
        {
            for (auto& [fieldName, value] : m_pendingFields.items())
            {
                std::string valueStr = value.dump();
                //LOG_INFO("[Deserialize] field: " + fieldName + " = " + valueStr);
                ScriptSystem::SetScriptField(m_gcHandle, fieldName.c_str(), valueStr.c_str());
            }
            m_pendingFields.clear();
        }

        // Awake を呼び出す
        ScriptSystem::AwakeScript(m_gcHandle);
        //m_isStartCalled = false; // Start はまだ呼び出されていない状態にする(Update 内で呼び出すため)

        // TODO:一時的な措置。スクリプトが有効化されたときに OnEnable を呼び出す。C++のライフサイクルを完全に理解してから見直すこと。
        OnEnable(); // ここに来てる時点でスクリプトが有効化されてるのでここで強制的に呼び出す
        LOG_INFO("[ScriptComponent] Created: " + scriptName);
    }
    else
    {
        LOG_ERROR("[ScriptComponent] Failed: " + scriptName);
    }
}

void ScriptComponent::Start()
{
    if (!m_gcHandle) return;
    //if (!m_isStartCalled)
    {
        ScriptSystem::StartScript(m_gcHandle);
        //m_isStartCalled = true;
    }
}

void ScriptComponent::Update(float deltaTime)
{
	if (!m_gcHandle) return; // GCHandle が有効でない場合は何もしない
	// Update を呼び出す
	ScriptSystem::UpdateScript(m_gcHandle);
}

#ifdef USE_IMGUI

#include "imgui_internal.h"
#include "Engine/Editor/History.h"
#include "Engine/EditorSupport/SetValueCommand.h"
#include "Engine/Editor/PropertyEditor.h"

void ScriptComponent::DrawProperty(const PropertyDrawContext& context)
{
	// スクリプトの編集ボタン
	// TODO: スクリプトアセットの管理方法が決まったら、ファイル名からスクリプトアセットを検索して開くようにする。現状はUserScriptsフォルダを再帰的に検索して最初に見つかったものを開く。
    if (ImGui::Button("Edit Script"))
    {
		// ファイル名に一致するスクリプトアセットを検索
		std::vector<fs::directory_entry> results;
		fs::path rootDir = "./UserScripts";
        if (fs::exists(rootDir) && fs::is_directory(rootDir))
        {
            for (const auto& entry : fs::recursive_directory_iterator(rootDir))
            {
                if (entry.is_regular_file() && entry.path().stem() == scriptName)
                {
                    results.push_back(entry);
                }
            }
		}
        if (!results.empty())
        {
            if (results.size() > 1)
            {
                LOG_WARNING("[ScriptComponent] Multiple script assets found with name: " + scriptName + ". Opening the first one.");
            }
            // 最初の一致を開く
            AssetBrowser::OpenAsset(results[0].path());
        }
        else
        {
            LOG_ERROR("[ScriptComponent] Script asset not found: " + scriptName);
        }
	}

	if (!m_gcHandle) return; // GCHandle が有効でない場合は何もしない

	// C# からフィールド情報を Json 形式で取得
	auto* jsonPtr = static_cast<char*>(ScriptSystem::GetScriptFields(m_gcHandle));
	if (!jsonPtr) return;
	std::string jsonStr = jsonPtr;
	CoTaskMemFree(jsonPtr); // C# 側で StringToCoTaskMemUTF8 で確保したメモリを解放

	// Json をパースして ImGui で描画
	json j = json::parse(jsonStr, nullptr,false);
    if (j.is_discarded())
    {
        LOG_ERROR("[ScriptComponent] Failed to parse script fields JSON: " + jsonStr);
        return;
	}
	// フィールド値をマップに格納
	m_fieldValues.clear();
    for (const auto& field : j)
    {
        std::string name = field.value("name", "");
        m_fieldValues[name] = field;
	}
	IMGUI_PROPERTY_BEGIN();
    for (const auto& field : j)
    {
        std::string name = field.value("name", "");
        std::string typeName = field.value("type", "");
        std::string header = field.value("header", "");
        std::string tooltip = field.value("tooltip", "");
		bool isComponentReference = field.value("isComponentReference", false);
        float rangeMin = field.value("rangeMin", 0.0f);
        float rangeMax = field.value("rangeMax", 0.0f);
		bool  hasRange = field.contains("rangeMin") && field.contains("rangeMax");

        // フィールドの型に応じて描画
        PropertyInfo prop;
        prop.type = typeName;
        prop.name = name;
        if (!header.empty())
            prop.attributes.push_back({ "Header", { header } });
        if (!tooltip.empty())
            prop.attributes.push_back({ "Tooltip", { tooltip } });
        if (isComponentReference)
            prop.attributes.push_back({ "ComponentReference", {} });
        if (hasRange)
			prop.attributes.push_back({ "Range", { std::to_string(rangeMin), std::to_string(rangeMax) } });
        
        if (typeName == "float")
        {
            prop.getter = [name](void* instance) -> std::any {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return std::any();
                auto it = comp->m_fieldValues.find(name);
                if (it != comp->m_fieldValues.end())
                {
                    return it->second.value("value", 0.0f);
                }
                return 0.0f;
				};
            prop.setter = [name](void* instance, std::any value) {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return;
                if (value.type() == typeid(float))
                {
                    float v = std::any_cast<float>(value);
                    ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(), std::to_string(v).c_str());
                    comp->m_fieldValues[name]["value"] = v;
                }
				};
        }
        else if (typeName == "int")
        {
            prop.getter = [name](void* instance) -> std::any {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return std::any();
                auto it = comp->m_fieldValues.find(name);
                if (it != comp->m_fieldValues.end())
                {
                    return it->second.value("value", 0);
                }
				return 0;
                };
            prop.setter = [name](void* instance, std::any value) {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return;
                if (value.type() == typeid(int))
                {
                    int v = std::any_cast<int>(value);
                    ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(), std::to_string(v).c_str());
                    comp->m_fieldValues[name]["value"] = v;
                }
				};
        }
        else if (typeName == "string")
        {
            prop.getter = [name](void* instance) -> std::any {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return std::any();
                auto it = comp->m_fieldValues.find(name);
                if (it != comp->m_fieldValues.end())
                {
                    return it->second.value("value", std::string());
				}
				return std::string();
                };
            prop.setter = [name](void* instance, std::any value) {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return;
                if (value.type() == typeid(std::string))
                {
                    std::string v = std::any_cast<std::string>(value);
                    ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(),
						"\"" + v + "\""); // 文字列をダブルクォートで囲む
                    comp->m_fieldValues[name]["value"] = v;
				}
				};
        }
        else if (typeName == "bool")
        {
            prop.getter = [name](void* instance) -> std::any {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return std::any();
                auto it = comp->m_fieldValues.find(name);
                if (it != comp->m_fieldValues.end())
                {
                    return it->second.value("value", false);
                }
                return false;
				};
            prop.setter = [name](void* instance, std::any value) {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return;
                if (value.type() == typeid(bool))
                {
                    bool v = std::any_cast<bool>(value);
                    ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(), v ? "true" : "false");
                    comp->m_fieldValues[name]["value"] = v;
                }
                };
        }
        else if (typeName == "Vector3")
        {
            prop.getter = [name](void* instance) -> std::any {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return std::any();
                auto it = comp->m_fieldValues.find(name);
                if (it != comp->m_fieldValues.end())
                {
                    json& field = it->second;
                    bool hasValue = !field["value"].is_null();
					if (!hasValue) return std::any();
					float x = field["value"]["x"].get<float>();
					float y = field["value"]["y"].get<float>();
					float z = field["value"]["z"].get<float>();
                    return Vector3(x, y, z);
                }
				return Vector3::Zero;
                };
            prop.setter = [name](void* instance, std::any value) {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return;
                if (value.type() == typeid(Vector3))
                {
                    Vector3 v = std::any_cast<Vector3>(value);
                    ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(),
                        ("{\"x\":" + std::to_string(v.x) + ",\"y\":" + std::to_string(v.y) + ",\"z\":" + std::to_string(v.z) + "}").c_str());
					comp->m_fieldValues[name]["value"] = { {"x", v.x}, {"y", v.y}, {"z", v.z} };
				}
                };
		}
        else if (typeName == "Quaternion")
        {
            prop.getter = [name](void* instance) -> std::any {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return std::any();
                auto it = comp->m_fieldValues.find(name);
                if (it != comp->m_fieldValues.end())
                {
                    json& field = it->second;
					bool hasValue = !field["value"].is_null();
					if (!hasValue) return std::any();
					float x = field["value"]["x"].get<float>();
					float y = field["value"]["y"].get<float>();
					float z = field["value"]["z"].get<float>();
                    float w = field["value"]["w"].get<float>();
                    return Quaternion(x, y, z, w);
				}
				return Quaternion::Identity;
                };
            prop.setter = [name](void* instance, std::any value) {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return;
                if (value.type() == typeid(Quaternion))
                {
                    Quaternion v = std::any_cast<Quaternion>(value);
                    ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(),
						("{\"x\":" + std::to_string(v.x) + ",\"y\":" + std::to_string(v.y) + ",\"z\":" + std::to_string(v.z) + ",\"w\":" + std::to_string(v.w) + "}").c_str());
					comp->m_fieldValues[name]["value"] = { {"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w} };
                    }
                };
		}
        else if (typeName == "Color")
        {
            prop.getter = [name](void* instance) -> std::any {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return std::any();
                auto it = comp->m_fieldValues.find(name);
                if (it != comp->m_fieldValues.end())
                {
                    json& field = it->second;
                    bool hasValue = !field["value"].is_null();
                    if (!hasValue) return std::any();
                    float r = field["value"]["r"].get<float>();
                    float g = field["value"]["g"].get<float>();
                    float b = field["value"]["b"].get<float>();
                    float a = field["value"]["a"].get<float>();
                    return Color(r, g, b, a);
                }
                return Color::White;
                };
            prop.setter = [name](void* instance, std::any value) {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return;
                if (value.type() == typeid(Color))
                {
                    Color v = std::any_cast<Color>(value);
					ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(),
						("{\"r\":" + std::to_string(v.r) + ",\"g\":" + std::to_string(v.g) + ",\"b\":" + std::to_string(v.b) + ",\"a\":" + std::to_string(v.a) + "}").c_str());
                    comp->m_fieldValues[name]["value"] = { {"r", v.r}, {"g", v.g}, {"b", v.b}, {"a", v.a} };
                }
                };
		}
        else if (typeName == "GameObject")
        {
			prop.attributes.push_back({ "ObjectReference", { "GameObject" } }); // GameObject 参照用の属性を追加
			prop.type = "ObjectId"; // GameObject は ObjectId として扱う
            prop.getter = [name](void* instance) -> std::any {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return std::any();
                auto it = comp->m_fieldValues.find(name);
                if (it != comp->m_fieldValues.end())
                {
                    json& field = it->second;
                    bool hasValue = !field["value"].is_null();
					uint64_t goId = ObjectId::Invalid().Value();
                    if (hasValue)
                    {
					    std::string valueStr = field["value"].get<std::string>();
                        if (valueStr.find("objectId: ") != std::string::npos)
                        {
                            size_t start = valueStr.find("objectId: ") + strlen("objectId: ");
                            size_t end = valueStr.find(")", start);
                            if (end != std::string::npos)
                            {
                                std::string idStr = valueStr.substr(start, end - start);
                                try
                                {
                                    goId = std::stoull(idStr);
                                }
                                catch (const std::exception& e)
                                {
                                    LOG_ERROR("[ScriptComponent] Failed to parse GameObject ID from string: " + valueStr + ". Error: " + e.what());
                                    return ObjectId::Invalid();
                                }
                            }
						}
                    }
					ObjectId id(goId);
                    return id;
                }
				return ObjectId::Invalid();
                };
            prop.setter = [name](void* instance, std::any value) {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return;
                if (value.type() == typeid(ObjectId))
                {
                    ObjectId id = std::any_cast<ObjectId>(value);
					uint64_t goId = id.Value();
					std::string valueStr = "GameObject(objectId: " + std::to_string(goId) + ")";
                    ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(),
						valueStr.c_str()); // GameObject の参照を文字列として渡す
                    comp->m_fieldValues[name]["value"] = valueStr;
                }
                };
		}
        else if (isComponentReference)
        {
			prop.attributes.push_back({ "ObjectReference", {typeName} }); // Component 参照用の属性を追加
			prop.type = "ObjectId"; // Component は ObjectId として扱う
            prop.getter = [name](void* instance) -> std::any {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return std::any();
                auto it = comp->m_fieldValues.find(name);
                if (it != comp->m_fieldValues.end())
                {
                    json& field = it->second;
                    bool hasValue = !field["value"].is_null();
					uint64_t compId = ObjectId::Invalid().Value();
                    if (hasValue)
                    {
					    std::string valueStr = field["value"].get<std::string>();
                        if (valueStr.find("objectId: ") != std::string::npos)
                        {
                            size_t start = valueStr.find("objectId: ") + strlen("objectId: ");
                            size_t end = valueStr.find(",", start);
                            if (end != std::string::npos)
                            {
                                std::string idStr = valueStr.substr(start, end - start);
                                try
                                {
                                    compId = std::stoull(idStr);
                                }
                                catch (const std::exception& e)
                                {
                                    LOG_ERROR("[ScriptComponent] Failed to parse Component ID from string: " + valueStr + ". Error: " + e.what());
                                    return ObjectId::Invalid();
                                }
                            }
                        }
                        if (valueStr.find("ownerId: ") != std::string::npos)
                        {
                            size_t start = valueStr.find("ownerId: ") + strlen("ownerId: ");
                            size_t end = valueStr.find(")", start);
                            if (end != std::string::npos)
                            {
                                std::string ownerIdStr = valueStr.substr(start, end - start);
                                try
                                {
                                    uint64_t ownerId = std::stoull(ownerIdStr);
                                    // ここで必要に応じて ownerId を使用することができます
                                }
                                catch (const std::exception& e)
                                {
                                    LOG_ERROR("[ScriptComponent] Failed to parse owner ID from string: " + valueStr + ". Error: " + e.what());
                                    return ObjectId::Invalid();
                                }
                            }
						}
                    }
                    ObjectId id(compId);
                    return id;
                }
				return ObjectId::Invalid();
                };
            prop.setter = [name](void* instance, std::any value) {
                auto comp = static_cast<ScriptComponent*>(instance);
                if (!comp || !comp->GetGCHandle()) return;
                if (value.type() == typeid(ObjectId))
                {
                    ObjectId c = std::any_cast<ObjectId>(value);
					uint64_t compId = c.Value();
					std::string valueStr = "Component(objectId: " + std::to_string(compId) + ", ownerId: " + std::to_string(comp->GetOwner()->GetId().Value()) + ")";
                    ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(),
						valueStr.c_str()); // Component の参照を文字列として渡す
					comp->m_fieldValues[name]["value"] = valueStr;
                    }
				};
        }
        else
        {
            LOG_WARNING("[ScriptComponent] Unsupported field type: " + prop.type);
			continue;
		}
		prop.hasCustomGetter = true;
		prop.hasCustomSetter = true;
		CurryEngine::PropertyEditor::DrawProperty(&prop, &context); // プロパティの描画
    }
	IMGUI_PROPERTY_END();
}
#endif // USE_IMGUI

json ScriptComponent::Serialize() const
{
	json j = Component::Serialize();
	j["scriptName"] = scriptName;

    if (m_gcHandle)
    {
		if (auto* fieldsJson = static_cast<char*>(ScriptSystem::GetScriptFields(m_gcHandle)))
		{
			std::string jsonStr = fieldsJson;
			CoTaskMemFree(fieldsJson); // C# 側で StringToCoTaskMemUTF8 で確保したメモリを解放

			json fields = json::parse(jsonStr, nullptr, false);
            if (!fields.is_discarded())
            {
                json fieldValues;
                for (const auto& field : fields)
                {
                    std::string name = field.value("name", "");
					if (!name.empty())
                	{
						fieldValues[name] = field["value"];
                    }
				}
                j["fields"] = fieldValues;
            }
		}
    }

	return j;
}

void ScriptComponent::Deserialize(const json& j)
{
	Component::Deserialize(j);
	scriptName = j.value("scriptName", "");

    if (j.contains("fields") && j["fields"].is_object())
    {
		// フィールドの値を一時的に保持しておく。スクリプトインスタンスが生成された後に適用する。
		m_pendingFields = j["fields"];
	}
}

void ScriptComponent::OnScriptUnload()
{
	if (!m_gcHandle) return; // GCHandle が有効でない場合は何もしない

	ScriptSystem::OnDestroyScript(m_gcHandle);
	ScriptSystem::ReleaseScript(m_gcHandle);
	m_gcHandle = nullptr;
}

void ScriptComponent::OnPreScriptReload()
{
    if (!m_gcHandle) return; // GCHandle が有効でない場合は何もしない

	// スクリプトのリロード前に、必要に応じて現在のスクリプトインスタンスからデータを保存する処理をここに追加することができます。
	m_pendingFields.clear();
	json j = Serialize(); // 現在の状態を JSON にシリアライズ
    if (j.contains("fields") && j["fields"].is_object())
    {
        m_pendingFields = j["fields"]; // フィールドの値を保持しておく
    }
}

void ScriptComponent::OnPostScriptReload()
{
    if (!m_gcHandle) return; // GCHandle が有効でない場合は何もしない
    // スクリプトのリロード後に、m_pendingFields に保持しておいたフィールドの値をスクリプトインスタンスに適用する
    for (auto& [name, value] : m_pendingFields.items())
    {
        ScriptSystem::SetScriptField(m_gcHandle, name.c_str(), value.dump().c_str());
    }
    m_pendingFields.clear();
}

void ScriptComponent::OnScriptReload()
{
	// スクリプトのリロード処理
	if (!m_gcHandle) return; // GCHandle が有効でない場合は何もしない

	// 既存のスクリプトインスタンスをホットスワップで更新する
    uint64_t ownerId = GetOwner()->GetId().Value();
    uint64_t componentId = GetId().Value();
	ScriptSystem::HotSwapScript(m_gcHandle, ownerId, componentId);
	OnPostScriptReload(); // ホットスワップ後にフィールドの値を再適用
}