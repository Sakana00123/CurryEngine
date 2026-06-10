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
                //Console::Log("[Deserialize] field: " + fieldName + " = " + valueStr);
                ScriptSystem::SetScriptField(m_gcHandle, fieldName.c_str(), valueStr.c_str());
            }
            m_pendingFields.clear();
        }

        // Awake を呼び出す
        ScriptSystem::AwakeScript(m_gcHandle);
        //m_isStartCalled = false; // Start はまだ呼び出されていない状態にする(Update 内で呼び出すため)

        // TODO:一時的な措置。スクリプトが有効化されたときに OnEnable を呼び出す。C++のライフサイクルを完全に理解してから見直すこと。
        OnEnable(); // ここに来てる時点でスクリプトが有効化されてるのでここで強制的に呼び出す
        Console::Log("[ScriptComponent] Created: " + scriptName);
    }
    else
    {
        Console::LogError("[ScriptComponent] Failed: " + scriptName);
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
                Console::LogWarning("[ScriptComponent] Multiple script assets found with name: " + scriptName + ". Opening the first one.");
            }
            // 最初の一致を開く
            AssetBrowser::OpenAsset(results[0].path());
        }
        else
        {
            Console::LogError("[ScriptComponent] Script asset not found: " + scriptName);
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
        Console::LogError("[ScriptComponent] Failed to parse script fields JSON: " + jsonStr);
        return;
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

		ImGui::PushID(name.c_str());
        // Header 表示
        if (!header.empty())
        {
            ImGui::Separator();
            ImGui::TextColored({ 0.8f, 0.8f, 0.2f, 1.0f }, "%s", header.c_str());
        }

        IMGUI_PROPERTY(name.c_str());

        bool edited = false;

        if (typeName == "float")
        {
			static float prevValue = 0.0f; // 前回の値を保持する変数
            
			//float v = field["value"].get<float>(); // この取得方法だと、nullのときに例外が発生する。nullのときは0.0fを返すようにする。
			float v = field.value("value", 0.0f); // nullのときは0.0fを返す (例外が発生しない)

			// 編集方法は Range 属性の有無で切り替える
            if (hasRange)
                edited = ImGui::SliderFloat(("##" + name).c_str(), &v, rangeMin, rangeMax);
            else
                edited = ImGui::DragFloat(("##" + name).c_str(), &v, 0.1f, rangeMin, rangeMax);
            
			if (ImGui::IsItemActivated()) // 編集開始時に前回の値を保存
            {
                prevValue = v;
			}

			// 確定したタイミングで、Commandを作成してUndoRedoStackに追加する
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
				float newValue = v;
				if (newValue != prevValue) // 値が変更された場合のみコマンドを追加
                {
                    CurryEngine::History::ExecuteCommand(
                        std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, float>>>(
							"Set " + name + " old:" + std::to_string(prevValue) + " new:" + std::to_string(newValue),
                            [this](const std::pair<std::string, float>& pair) {
								// コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
								ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(),
									std::to_string(pair.second).c_str());
                            },
                            std::make_pair(name, prevValue),
                            std::make_pair(name, newValue)
                        )
                    );
				}
				prevValue = newValue; // 前回の値を新しい値に更新
            }

			// 即座にスクリプト側の値も更新する
            if (edited)
                ScriptSystem::SetScriptField(m_gcHandle, name.c_str(),
                    std::to_string(v).c_str());
        }
        else if (typeName == "int")
        {
			static int prevValue = 0; // 前回の値を保持する変数

			int v = field.value("value", 0); // nullのときは0を返す (例外が発生しない)
            
            if (hasRange)
                edited = ImGui::SliderInt(("##" + name).c_str(), &v,
                    (int)rangeMin, (int)rangeMax);
            else
                edited = ImGui::DragInt(("##" + name).c_str(), &v,
                    (int)rangeMin, (int)rangeMax);
            
            if (ImGui::IsItemActivated()) // 編集開始時に前回の値を保存
            {
                prevValue = v;
            }

			// 確定したタイミングで、Commandを作成してUndoRedoStackに追加する
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                int newValue = v;
                if (newValue != prevValue) // 値が変更された場合のみコマンドを追加
                {
                    CurryEngine::History::ExecuteCommand(
						std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, int>>>(
							"Set " + name + " old:" + std::to_string(prevValue) + " new:" + std::to_string(newValue),
							[this](const std::pair<std::string, int>& pair) {
								// コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
								ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(),
                                    std::to_string(pair.second).c_str());
                            },
                            std::make_pair(name, prevValue),
                            std::make_pair(name, newValue)
                        )
                    );
                }
				prevValue = newValue; // 前回の値を新しい値に更新
			}

			// 即座にスクリプト側の値も更新する
            if (edited)
                ScriptSystem::SetScriptField(m_gcHandle, name.c_str(),
                    std::to_string(v).c_str());
        }
        else if (typeName == "bool")
        {
			static bool prevValue = false; // 前回の値を保持する変数
			bool v = field.value("value", false); // nullのときはfalseを返す (例外が発生しない)
            edited = ImGui::Checkbox(("##" + name).c_str(), &v);
            if (ImGui::IsItemActivated()) // 編集開始時に前回の値を保存
            {
                prevValue = v;
			}

			// 確定したタイミングで、Commandを作成してUndoRedoStackに追加する
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                bool newValue = v;
                if (newValue != prevValue) // 値が変更された場合のみコマンドを追加
                {
                    CurryEngine::History::ExecuteCommand(
                        std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, bool>>>(
                            "Set " + name + " old:" + (prevValue ? "true" : "false") + " new:" + (newValue ? "true" : "false"),
                            [this](const std::pair<std::string, bool>& pair) {
                                // コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
                                ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(),
                                    pair.second ? "true" : "false");
                            },
                            std::make_pair(name, prevValue),
                            std::make_pair(name, newValue)
                        )
                    );
                }
                prevValue = newValue; // 前回の値を新しい値に更新
            }

			// 即座にスクリプト側の値も更新する
            if (edited)
                ScriptSystem::SetScriptField(m_gcHandle, name.c_str(),
                    v ? "true" : "false");
        }
        else if (typeName == "string")
        {
			static std::string prevValue; // 前回の値を保持する変数
			std::string v = field.value("value", ""); // nullのときは空文字を返す (例外が発生しない)
            char buf[256];
            strncpy_s(buf, v.c_str(), sizeof(buf));
            edited = ImGui::InputText(("##" + name).c_str(), buf, sizeof(buf));

            if (ImGui::IsItemActivated()) // 編集開始時に前回の値を保存
            {
                prevValue = v;
			}

			// 確定したタイミングで、Commandを作成してUndoRedoStackに追加する
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                std::string newValue(buf);
                if (newValue != prevValue) // 値が変更された場合のみコマンドを追加
                {
                    CurryEngine::History::ExecuteCommand(
                        std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, std::string>>>(
                            "Set " + name + " old:\"" + prevValue + "\" new:\"" + newValue + "\"",
                            [this](const std::pair<std::string, std::string>& pair) {
                                // コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
                                ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(),
                                    ("\"" + pair.second + "\"").c_str());
                            },
                            std::make_pair(name, prevValue),
                            std::make_pair(name, newValue)
                        )
                    );
                }
                prevValue = newValue; // 前回の値を新しい値に更新
			}

			// 即座にスクリプト側の値も更新する
            if (edited)
                ScriptSystem::SetScriptField(m_gcHandle, name.c_str(),
                    ("\"" + std::string(buf) + "\"").c_str());
        }
        else if (typeName == "Vector3")
        {
			static float prevVec[3] = { 0,0,0 }; // 前回の値を保持する変数
            float vec[3] = { 0,0,0 };
			bool isNull = field["value"].is_null();
			vec[0] = isNull ? 0.0f : field["value"]["x"].get<float>(); // nullのときは0.0fを返す (例外が発生しないようにする)
			vec[1] = isNull ? 0.0f : field["value"]["y"].get<float>();
			vec[2] = isNull ? 0.0f : field["value"]["z"].get<float>();
			bool itemActivated = false;
			bool deactivatedAfterEdit = false;
			// Vector3 は 特別な表示方法で、X/Y/Zを分けて表示する
            {
                ImGui::PushMultiItemsWidths(IM_ARRAYSIZE(vec), ImGui::CalcItemWidth());
                ImGui::Text("X");
				ImGui::SameLine();
                edited |= ImGui::DragFloat("##X", &vec[0], 0.1f);
				itemActivated |= ImGui::IsItemActivated();
				deactivatedAfterEdit |= ImGui::IsItemDeactivatedAfterEdit();
                ImGui::PopItemWidth();
                ImGui::SameLine();
				ImGui::Text("Y");
                ImGui::SameLine();
                edited |= ImGui::DragFloat("##Y", &vec[1], 0.1f);
				itemActivated |= ImGui::IsItemActivated();
				deactivatedAfterEdit |= ImGui::IsItemDeactivatedAfterEdit();
                ImGui::PopItemWidth();
				ImGui::SameLine();
                ImGui::Text("Z");
				ImGui::SameLine();
				edited |= ImGui::DragFloat("##Z", &vec[2], 0.1f);
				itemActivated |= ImGui::IsItemActivated();
				deactivatedAfterEdit |= ImGui::IsItemDeactivatedAfterEdit();
				ImGui::PopItemWidth();
            }
            if (itemActivated) // 編集開始時に前回の値を保存
            {
                prevVec[0] = vec[0];
                prevVec[1] = vec[1];
                prevVec[2] = vec[2];
            }
			// 確定したタイミングで、Commandを作成してUndoRedoStackに追加する
            if (deactivatedAfterEdit)
            {
                float newVec[3] = { vec[0], vec[1], vec[2] };
                if (newVec[0] != prevVec[0] || newVec[1] != prevVec[1] || newVec[2] != prevVec[2]) // 値が変更された場合のみコマンドを追加
                {
                    CurryEngine::History::ExecuteCommand(
                        std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, std::array<float, 3>>>>(
                            "Set " + name + " old:(" + std::to_string(prevVec[0]) + "," + std::to_string(prevVec[1]) + "," + std::to_string(prevVec[2]) +
                            ") new:(" + std::to_string(newVec[0]) + "," + std::to_string(newVec[1]) + "," + std::to_string(newVec[2]) + ")",
                            [this](const std::pair<std::string, std::array<float, 3>>& pair) {
                                // コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
                                std::string valueJson =
                                    "{\"x\":" + std::to_string(pair.second[0]) +
                                    ",\"y\":" + std::to_string(pair.second[1]) +
                                    ",\"z\":" + std::to_string(pair.second[2]) + "}";
                                ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(), valueJson.c_str());
                            },
                            std::make_pair(name, std::array<float, 3>{ prevVec[0], prevVec[1], prevVec[2] }),
                            std::make_pair(name, std::array<float, 3>{ newVec[0], newVec[1], newVec[2] })
                        )
                    );
                }
				// 前回の値を新しい値に更新
                prevVec[0] = newVec[0];
                prevVec[1] = newVec[1];
                prevVec[2] = newVec[2];
			}
			// 即座にスクリプト側の値も更新する
            if (edited)
            {
                std::string valueJson =
					"{\"x\":" + std::to_string(vec[0]) +
					",\"y\":" + std::to_string(vec[1]) +
					",\"z\":" + std::to_string(vec[2]) + "}";
                ScriptSystem::SetScriptField(m_gcHandle, name.c_str(), valueJson.c_str());
			}
        }
        else if (typeName == "Quaternion")
        {
			static float prevQuat[4] = { 0,0,0,1 }; // 前回の値を保持する変数
            float quat[4] = { 0,0,0,1 };
			bool isNull = field["value"].is_null();
			quat[0] = isNull ? 0.0f : field["value"].value("x", 0.0f); // nullのときは0.0fを返す (例外が発生しないようにする)
			quat[1] = isNull ? 0.0f : field["value"].value("y", 0.0f);
			quat[2] = isNull ? 0.0f : field["value"].value("z", 0.0f);
			quat[3] = isNull ? 1.0f : field["value"].value("w", 1.0f);
			// クォータニオンをオイラー角に変換して表示
			Vector3 euler = Transform::QuaternionToEuler({ quat[0], quat[1], quat[2], quat[3] });
			float vec[3] = { euler.x, euler.y, euler.z };
			bool itemActivated = false;
			bool deactivatedAfterEdit = false;
			// Quaternion は 特別な表示方法で、オイラー角に変換して X/Y/Z を分けて表示する
            {
                ImGui::PushMultiItemsWidths(IM_ARRAYSIZE(vec), ImGui::CalcItemWidth());
                ImGui::Text("X");
                ImGui::SameLine();
                edited |= ImGui::DragFloat("##X", &vec[0], 0.1f);
				itemActivated |= ImGui::IsItemActivated();
				deactivatedAfterEdit |= ImGui::IsItemDeactivatedAfterEdit();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text("Y");
                ImGui::SameLine();
                edited |= ImGui::DragFloat("##Y", &vec[1], 0.1f);
				itemActivated |= ImGui::IsItemActivated();
				deactivatedAfterEdit |= ImGui::IsItemDeactivatedAfterEdit();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text("Z");
                ImGui::SameLine();
                edited |= ImGui::DragFloat("##Z", &vec[2], 0.1f);
				itemActivated |= ImGui::IsItemActivated();
				deactivatedAfterEdit |= ImGui::IsItemDeactivatedAfterEdit();
                ImGui::PopItemWidth();
            }
            if (itemActivated) // 編集開始時に前回の値を保存
            {
                prevQuat[0] = quat[0];
                prevQuat[1] = quat[1];
                prevQuat[2] = quat[2];
                prevQuat[3] = quat[3];
			}
			// 確定したタイミングで、Commandを作成してUndoRedoStackに追加する
            if (deactivatedAfterEdit)
            {
				float newQuat[4];
				// 編集後のオイラー角をクォータニオンに変換して保存
                Vector3 newEuler = { vec[0], vec[1], vec[2] };
				Quaternion newQ = Transform::EulerToQuaternion(newEuler);
				newQuat[0] = newQ.x;
				newQuat[1] = newQ.y;
				newQuat[2] = newQ.z;
                newQuat[3] = newQ.w;
                if (newQuat[0] != prevQuat[0] || newQuat[1] != prevQuat[1] || newQuat[2] != prevQuat[2] || newQuat[3] != prevQuat[3]) // 値が変更された場合のみコマンドを追加
                {
                    CurryEngine::History::ExecuteCommand(
                        std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, std::array<float, 4>>>>(
                            "Set " + name + " old:(" + std::to_string(prevQuat[0]) + "," + std::to_string(prevQuat[1]) + "," + std::to_string(prevQuat[2]) + "," + std::to_string(prevQuat[3]) +
                            ") new:(" + std::to_string(newQuat[0]) + "," + std::to_string(newQuat[1]) + "," + std::to_string(newQuat[2]) + "," + std::to_string(newQuat[3]) + ")",
                            [this](const std::pair<std::string, std::array<float, 4>>& pair) {
                                // コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
                                std::string valueJson =
                                    "{\"x\":" + std::to_string(pair.second[0]) +
                                    ",\"y\":" + std::to_string(pair.second[1]) +
									",\"z\":" + std::to_string(pair.second[2]) +
                                    ",\"w\":" + std::to_string(pair.second[3]) + "}";
                                ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(), valueJson.c_str());
                            },
                            std::make_pair(name, std::array<float, 4>{ prevQuat[0], prevQuat[1], prevQuat[2], prevQuat[3] }),
                            std::make_pair(name, std::array<float, 4>{ newQuat[0], newQuat[1], newQuat[2], newQuat[3] })
                        )
                    );
				}
                // 前回の値を新しい値に更新
                prevQuat[0] = newQuat[0];
                prevQuat[1] = newQuat[1];
                prevQuat[2] = newQuat[2];
				prevQuat[3] = newQuat[3];
			}
			// 即座にスクリプト側の値も更新する
            if (edited)
            {
				// 編集後のオイラー角をクォータニオンに変換して保存
				Vector3 newEuler = { vec[0], vec[1], vec[2] };
                Quaternion newQuat = Transform::EulerToQuaternion(newEuler);
                std::string valueJson =
                    "{\"x\":" + std::to_string(newQuat.x) +
                    ",\"y\":" + std::to_string(newQuat.y) +
                    ",\"z\":" + std::to_string(newQuat.z) +
					",\"w\":" + std::to_string(newQuat.w) + "}";
				ScriptSystem::SetScriptField(m_gcHandle, name.c_str(), valueJson.c_str());
            }
        }
		else if (typeName == "GameObject") // GameObject 参照用の特別な表示方法
        {
			// "GameObject(objectId: 123456789)" の形式で値が渡される想定。nullのときは null が渡される。
			bool isNull = field["value"].is_null();
			
			uint64_t id = 0;
            if (!isNull)
            {
                std::string valueStr = isNull ? "null" : field["value"].dump(); // nullのときは "null" という文字列を返す (例外が発生しないようにする)
                if (valueStr.find("objectId: ") != std::string::npos)
                {
                    size_t idStart = valueStr.find("objectId: ") + strlen("objectId: ");
                    size_t idEnd = valueStr.find(")", idStart);
                    std::string objectIdStr = valueStr.substr(idStart, idEnd); // objectId の部分だけ抜き取る
                    try {
                        id = std::stoull(objectIdStr); // objectId を uint64_t に変換
                    }
                    catch (const std::exception& e) {
                        LOG_ERROR("[ScriptComponent] Failed to parse GameObject reference: " + valueStr + ". Error: " + e.what());
                    }
                }
            }
			ObjectId v = ObjectId::FromValue(id); // nullのときはInvalidを返す (例外が発生しない)
			auto* vGameObject = ObjectManager::Find(v);
			std::string buttonLabel = vGameObject ? vGameObject->GetName() + "(GameObject)" : "None(GameObject)";
			ImGui::Button(buttonLabel.c_str());

            // ドラッグアンドドロップでターゲット設定
            if (ImGui::BeginDragDropTarget())
            {
				bool isGameObjectPayload = false;
				ObjectId droppedId = ObjectId::Invalid();

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
                    //IM_ASSERT(payload->DataSize == sizeof(ObjectId*));
                    IM_ASSERT(payload->DataSize == sizeof(EditorSelectionReference*));
                    if (EditorSelectionReference* pRef = static_cast<EditorSelectionReference*>(payload->Data))
                    {
						for (const auto& pObj : pRef->selectedObjects)
                        {
							droppedId = pObj.lock()->GetId();
                            isGameObjectPayload = true;
                        }
                    }
                }
                ImGui::EndDragDropTarget();

                if (isGameObjectPayload)
                {
                    ObjectId newValue = droppedId;
                    if (newValue != v) // 値が変更された場合のみコマンドを追加
                    {
                        CurryEngine::History::ExecuteCommand(
                            std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, ObjectId>>>(
                            "Set " + name + " GameObject reference",
                            [this](const std::pair<std::string, ObjectId>& pair) {
                                std::string valueStr = "GameObject(objectId: " + std::to_string(pair.second.Value()) + ")";
                                // コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
                                ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(),
                                    valueStr.c_str());
                            },
                            std::make_pair(name, v), // 変更前の値
                            std::make_pair(name, newValue) // 変更後の値
                            )
                        );
					}
                }
            }
            ImGui::SameLine();
            // クリアボタン
            if (ImGui::Button("X")) {
                CurryEngine::History::ExecuteCommand(
                    std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, ObjectId>>>(
                        "Clear " + name + " GameObject reference",
                        [this](const std::pair<std::string, ObjectId>& pair) {
                            std::string valueStr = "GameObject(objectId: " + std::to_string(pair.second.Value()) + ")";
                            // コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
                            ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(),
                                valueStr.c_str());
                        },
                        std::make_pair(name, v), // 変更前の値
                        std::make_pair(name, ObjectId::Invalid()) // 変更後の値
                    )
                );
            }
			// ... ボタン (シーン内のオブジェクトを選択するためのもの)
            ImGui::SameLine();
            if (ImGui::Button("...")) {
                // シーン内のオブジェクトを選択するためのポップアップを表示
                ImGui::OpenPopup(("Select GameObject##" + name).c_str());
            }
            if (ImGui::BeginPopup(("Select GameObject##" + name).c_str()))
            {
                Scene* scene = GetOwner()->GetScene();
                for (const auto& obj : scene->GetObjectManager()->GetAll())
                {
                    if (!obj) continue;
                    if (ImGui::Selectable(obj->GetName().c_str()))
                    {
                        ObjectId newValue = obj->GetId();
                        if (newValue != v) // 値が変更された場合のみコマンドを追加
                        {
                            CurryEngine::History::ExecuteCommand(
                                std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, ObjectId>>>(
                                    "Set " + name + " GameObject reference",
                                    [this](const std::pair<std::string, ObjectId>& pair) {
                                        std::string valueStr = "GameObject(objectId: " + std::to_string(pair.second.Value()) + ")";
                                        // コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
                                        ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(),
                                            valueStr.c_str());
                                    },
                                    std::make_pair(name, v), // 変更前の値
                                    std::make_pair(name, newValue) // 変更後の値
                                )
                            );
                        }
                    }
                }
                ImGui::EndPopup();
            }
        }
        else if (isComponentReference) // Component 参照用の特別な表示方法
        {
			// "Component(objectId: 123456789, ownerId: 0)" の形式で値が渡される想定。nullのときは null が渡される。
			bool isNull = field["value"].is_null();
			uint64_t objectId = 0;
			uint64_t ownerId = 0;
            if (!isNull)
            {
                std::string valueStr = isNull ? "null" : field["value"].dump(); // nullのときは "null" という文字列を返す (例外が発生しないようにする)
                if (valueStr.find("objectId: ") != std::string::npos)
                {
                    size_t idStart = valueStr.find("objectId: ") + strlen("objectId: ");
                    size_t idEnd = valueStr.find(",", idStart);
                    std::string objectIdStr = valueStr.substr(idStart, idEnd - idStart); // objectId の部分だけ抜き取る
                    try {
                        objectId = std::stoull(objectIdStr); // objectId を uint64_t に変換
                    }
                    catch (const std::exception& e) {
                        LOG_ERROR("[ScriptComponent] Failed to parse Component reference: " + valueStr + ". Error: " + e.what());
                    }
                }
			}
			Scene* scene = GetOwner()->GetScene();
            const auto& compMap = scene->objectManager->GetComponentCacheMap();
			ObjectId v = ObjectId::FromValue(objectId); // nullのときはInvalidを返す (例外が発生しない)
			ObjectId ownerV = ObjectId::Invalid();
            const auto& vComponent = compMap.contains(v) ? compMap.at(v).lock() : nullptr;
            std::string buttonLabel = std::string(vComponent ? vComponent->GetOwner()->GetName() : "None") + "(" + typeName + ")";
			ImGui::Button(buttonLabel.c_str());

			// ドラッグアンドドロップでターゲット設定
            if (ImGui::BeginDragDropTarget())
            {
				bool isComponentPayload = false;
				ObjectId droppedId = ObjectId::Invalid();

				// ドロップされたペイロードが、このフィールドが参照する Component と同じ型かどうかをチェックするために、ペイロードのタイプを typeName として受け入れる
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload((typeName).c_str())) {
                    IM_ASSERT(payload->DataSize == sizeof(ObjectId*));
                    if (ObjectId* pId = static_cast<ObjectId*>(payload->Data))
                    {
						droppedId = *pId;
						isComponentPayload = true;
                    }
                }
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
                    IM_ASSERT(payload->DataSize == sizeof(ObjectId*));
                    if (ObjectId* pId = static_cast<ObjectId*>(payload->Data))
                    {
                        // GameObject がドロップされた場合、その GameObject にアタッチされている同じ型の Component を探す
                        ObjectId gameObjectId = *pId;
                        if (auto gameObject = ObjectManager::Find(gameObjectId))
                        {
                            for (const auto& comp : gameObject->GetAllComponents())
                            {
								if (comp->GetTypeName() == typeName) // ComponentのGetName()は型名を返す想定
                                {
                                    droppedId = comp->GetId();
									isComponentPayload = true;
                                    break;
                                }
                            }
						}
                    }
                }
                ImGui::EndDragDropTarget();

				if (isComponentPayload)
                {
                    const auto& droppedComponent = compMap.contains(droppedId) ? compMap.at(droppedId).lock() : nullptr;
                    ObjectId droppedOwnerId = droppedComponent ? droppedComponent->GetOwner()->GetId() : ObjectId::Invalid();
                    if (droppedId != v) // ドロップされた Component が現在の値と異なる場合のみ更新する
                    {
                        CurryEngine::History::ExecuteCommand(
                            std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, std::pair<ObjectId, ObjectId>>>>(
                                "Set " + name + " Component reference",
                                [this](const std::pair<std::string, std::pair<ObjectId, ObjectId>>& pair) {
                                    // コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
                                    std::string valueStr = "Component(objectId: " + std::to_string(pair.second.first.Value()) + ", ownerId: " + std::to_string(pair.second.second.Value()) + ")";
                                    ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(),
                                        valueStr.c_str());
                                },
                                std::make_pair(name, std::make_pair(v, ownerV)), // 変更前の値
                                std::make_pair(name, std::make_pair(droppedId, droppedOwnerId)) // 変更後の値
                            )
                        );
                        // スクリプトフィールドを更新する
                        std::string valueStr = "Component(objectId: " + std::to_string(droppedId.Value()) + ", ownerId: " + std::to_string(droppedOwnerId.Value()) + ")";
                        ScriptSystem::SetScriptField(m_gcHandle, name.c_str(),
                            valueStr.c_str());
                    }
                    v = droppedId;
                    ownerV = droppedOwnerId;
                }
            }

			ImGui::SameLine();

            // クリアボタン
            if (ImGui::Button("X")) {
                CurryEngine::History::ExecuteCommand(
                    std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, std::pair<ObjectId, ObjectId>>>>(
                        "Clear " + name + " Component reference",
                        [this](const std::pair<std::string, std::pair<ObjectId, ObjectId>>& pair) {
							std::string valueStr = "Component(objectId: " + std::to_string(pair.second.first.Value()) + ", ownerId: " + std::to_string(pair.second.second.Value()) + ")";
                            // コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
                            ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(),
                                valueStr.c_str());
                        },
                        std::make_pair(name, std::make_pair(v, ownerV)), // 変更前の値
                        std::make_pair(name, std::make_pair(ObjectId::Invalid(), ObjectId::Invalid())) // 変更後の値
                    )
                );
				v = ObjectId::Invalid();
				ownerV = ObjectId::Invalid();
            }
			// ... ボタン (シーン内のオブジェクトを選択するためのもの)
            ImGui::SameLine();
            if (ImGui::Button("...")) {
                // シーン内のオブジェクトを選択するためのポップアップを表示
                ImGui::OpenPopup(("Select Component##" + name).c_str());
            }
            if (ImGui::BeginPopup(("Select Component##" + name).c_str()))
            {
                Scene* scene = GetOwner()->GetScene();
                for (const auto& obj : scene->GetObjectManager()->GetAll())
                {
                    if (!obj) continue;
                    for (const auto& comp : obj->GetAllComponents())
                    {
						if (comp->GetTypeName() == typeName) // ComponentのGetName()は型名を返す想定
                        {
                            std::string compLabel = obj->GetName() + " - " + comp->GetTypeName();
                            if (ImGui::Selectable(compLabel.c_str()))
                            {
                                ObjectId newValue = comp->GetId();
                                ObjectId newOwnerId = comp->GetOwner()->GetId();
                                if (newValue != v) // 値が変更された場合のみコマンドを追加
                                {
                                    CurryEngine::History::ExecuteCommand(
                                        std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, std::pair<ObjectId, ObjectId>>>>(
                                            "Set " + name + " Component reference",
                                            [this](const std::pair<std::string, std::pair<ObjectId, ObjectId>>& pair) {
                                                std::string valueStr = "Component(objectId: " + std::to_string(pair.second.first.Value()) + ", ownerId: " + std::to_string(pair.second.second.Value()) + ")";
                                                // コマンドの実行とUndoの両方で呼び出されるラムダ関数。スクリプトフィールドを更新する。
                                                ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(),
                                                    valueStr.c_str());
                                            },
                                            std::make_pair(name, std::make_pair(v, ownerV)), // 変更前の値
                                            std::make_pair(name, std::make_pair(newValue, newOwnerId)) // 変更後の値
                                        )
                                    );
                                    // スクリプトフィールドを更新する
                                    std::string valueStr = "Component(objectId: " + std::to_string(newValue.Value()) + ", ownerId: " + std::to_string(newOwnerId.Value()) + ")";
                                    ScriptSystem::SetScriptField(m_gcHandle, name.c_str(),
                                        valueStr.c_str());
                                }
                                v = newValue;
                                ownerV = newOwnerId;
                            }
                        }
                    }
                }
                ImGui::EndPopup();
            }
        }

        // Tooltip 表示
        if (!tooltip.empty() && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", tooltip.c_str());


        ImGui::PopID();
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