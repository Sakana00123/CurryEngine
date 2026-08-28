#include "pch.h"
#include "ScriptSystem.h"
#include "ScriptWatcher.h"

#include "ScriptHost.h"
#include "Engine/Core/ScriptComponent.h"
#include "Engine/Editor/Console.h"
#include "Engine/Factory/ScriptFactory.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Physics/Collider.h"
#include "Engine/ProjectSettings.h"

#include "Engine/Scripting/Exports/ScriptBridgeTypes.h"
#include "ScriptFieldSerializer.h"

extern "C" void __stdcall OnScriptClassRegistered(const ScriptClassDesc* desc)
{
	ClassMeta meta;
	meta.name = desc->name;
	if (desc->baseClass) {
		meta.bases.push_back(desc->baseClass->name);
	}
	meta.isScript = true; // スクリプトクラスであることをマーク

	for (int i = 0; i < desc->propertyCount; ++i) {
		const ScriptPropertyDesc& propDesc = desc->properties[i];
		PropertyInfo propInfo;
		propInfo.name = propDesc.name;
		propInfo.type = propDesc.type;

		propInfo.getter = [name = std::string(propDesc.name)](void* instance) -> std::any {
			// C# 側のスクリプトインスタンスからプロパティの値を取得するためのロジックをここに実装
			// 例えば、ScriptSystem を通じて C# 側に値の取得をリクエストするなど
			auto* sc = static_cast<ScriptComponent*>(instance);
			if (auto* fieldsJson = static_cast<char*>(ScriptSystem::GetScriptFields(sc->GetGCHandle())))
			{
				std::string jsonStr = fieldsJson;
				CoTaskMemFree(fieldsJson); // C# 側で StringToCoTaskMemUTF8 で確保したメモリを解放

				json fields = json::parse(jsonStr, nullptr, false);
				if (!fields.is_discarded())
				{
					json fieldJson;
					for (auto& field : fields)
					{
						if (field.contains("name") && field["name"] == name)
						{
							fieldJson = field;
							break;
						}
					}

					if (fieldJson.is_object())
					{
						return CurryEngine::ScriptFieldSerializer::FromJson(fieldJson["type"], fieldJson["value"]);
					}
					else
					{
						LOG_ERROR("Property '" + name + "' not found in script fields.");
						return std::any();
					}
				}
				else
				{
					LOG_ERROR("Failed to parse script fields JSON: " + jsonStr);
					return std::any();
				}
			}
			else
			{
				LOG_ERROR("Failed to get script fields for property '" + name + "'.");
				return std::any();
			}
			};
		propInfo.setter = [name = std::string(propDesc.name)](void* instance, std::any value) {
			// C# 側のスクリプトインスタンスにプロパティの値を設定するためのロジックをここに実装
			// 例えば、ScriptSystem を通じて C# 側に値の設定をリクエストするなど
			auto* sc = static_cast<ScriptComponent*>(instance);
			std::string valueStr;
			try
			{
				valueStr = CurryEngine::ScriptFieldSerializer::ToJson(value);
			}
			catch (const std::bad_any_cast& e)
			{
				LOG_ERROR("Failed to cast property value to string: " + std::string(e.what()));
				return;
			}
			ScriptSystem::SetScriptField(sc->GetGCHandle(), name.c_str(), valueStr.c_str());
			};

		meta.properties.push_back(std::move(propInfo));
	}
	// ReflectionRegistry に登録
	ReflectionRegistry::Register(meta);
}

void ScriptSystem::Initialize()
{
	// exe のディレクトリを取得
	char exePath[MAX_PATH];
	GetModuleFileNameA(NULL, exePath, MAX_PATH);
	std::string exeDir(exePath);
	// パスの区切り文字を統一
	std::replace(exeDir.begin(), exeDir.end(), '/', '\\');
	// ディレクトリ部分だけを抽出
	exeDir = exeDir.substr(0, exeDir.rfind("\\"));
	
	// プロジェクト設定の読み込み
	ProjectSettings::Load(exeDir);
	ProjectSettingsData settings = ProjectSettings::Get();

	// スクリプトホストの初期化
	s_scriptHost = new ScriptHost();
	if (!s_scriptHost->Initialize()) {
		LOG_ERROR("[ScriptSystem] Failed to initialize the script host.");
		delete s_scriptHost;
		s_scriptHost = nullptr;
		return;
	}

#ifdef _DEBUG
	// スクリプトウォッチャーの初期化
	s_scriptWatcher = new ScriptWatcher();

	// TODO: あとでここを修正する
	//std::vector<BuildCommand> commands = {
	//	/*{".\\CurryEngine.API\\CurryEngine.API.csproj", "-c Debug --nologo -v q 2>&1" },
	//	{ ".\\CurryEngine.Runtime\\CurryEngine.Runtime.csproj", "-c Debug --nologo -v q 2>&1" },*/
	//	{ settings.scriptProjectPath, "-c Release --nologo -v q 2>&1" }
	//};
	std::vector<BuildCommand> commands = {
		{ settings.scriptProjectPath, "-c Release --nologo -v q 2>&1" } //-o \"" + settings.scriptOutputPath + "\" 
	};

	s_scriptWatcher->Start(
		settings.scriptWatchDirectory,
		commands,
		[]() {
			ScriptSystem::Reload(); // ビルド成功時にスクリプトをリロードするコールバック
		}
	);
	s_scriptWatcher->RequestBuild(); // 起動時に一度ビルドを要求して最新のスクリプトを読み込む  
#endif // _DEBUG

	LOG_INFO("[ScriptSystem] Script host initialized successfully.");
	return;
}

void ScriptSystem::Update()
{
	if (!s_scriptHost) return;
	s_scriptHost->Update();
}

void ScriptSystem::Shutdown()
{
#ifdef _DEBUG
	// スクリプトウォッチャーの終了処理
	if (s_scriptWatcher) {
		s_scriptWatcher->Stop();
		delete s_scriptWatcher;
		s_scriptWatcher = nullptr;
	}
#endif // _DEBUG

	if (!s_scriptHost) {
		LOG_ERROR("[ScriptSystem] Cannot shutdown scripts because the script host is not initialized.");
		return;
	}
	// スクリプトホストの終了処理
	s_scriptHost->Shutdown();
	delete s_scriptHost;
	s_scriptHost = nullptr;
}

void ScriptSystem::Reload()
{
	if (!s_scriptHost) {
		LOG_ERROR("[ScriptSystem] Cannot reload scripts because the script host is not initialized.");
		return;
	}

	auto* scene = SceneManager::GetCurrentScene();

	if (scene)
	{
		for (auto& object : scene->GetAllSceneObjects())
		{
			for (auto* scriptComponent : object->GetComponents<ScriptComponent>())
			{
				if (scriptComponent)
				{
					scriptComponent->OnPreScriptReload();
				}
			}
		}
	}

	// C#スクリプトのメタ情報をクリア
	ReflectionRegistry::UnregisterScriptClasses();

	// Assembly-CSharp.dll をリロード
	s_scriptHost->GetCallbacks().ReloadScripts("");

	// C#側に今すぐ全クラスの登録を要求して、スクリプトクラスのメタデータを更新する
	//s_scriptHost->GetCallbacks().RegisterAllScriptMeta(&OnScriptClassRegistered);
	// すべてのスクリプトクラスの登録を要求する
	for (const auto& className : GetRegisteredScriptNames())
	{
		auto* jsonPtr = static_cast<char*>(ScriptSystem::GetScriptMeta(className));
		if (!jsonPtr) continue;
		std::string jsonStr = jsonPtr;
		CoTaskMemFree(jsonPtr); // C# 側で StringToCoTaskMemUTF8 で確保したメモリを解放

		// Json をパースして ImGui で描画
		json j = json::parse(jsonStr, nullptr, false);
		if (j.is_discarded())
		{
			LOG_ERROR("[ScriptComponent] Failed to parse script fields JSON: " + jsonStr);
			continue;
		}
		// フィールド値をマップに格納
		ClassMeta meta;
		meta.name = className;
		meta.isScript = true; // スクリプトクラスであることをマーク
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
						std::string valueStr = std::to_string(v);
						ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(), valueStr);
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
						std::string valueStr = std::to_string(v);
						ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(), valueStr);
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
						std::string valueStr = "{\"x\":" + std::to_string(v.x) + ",\"y\":" + std::to_string(v.y) + ",\"z\":" + std::to_string(v.z) + "}";
						ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(),
							valueStr);
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
						std::string valueStr = "{\"x\":" + std::to_string(v.x) + ",\"y\":" + std::to_string(v.y) + ",\"z\":" + std::to_string(v.z) + ",\"w\":" + std::to_string(v.w) + "}";
						ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(), valueStr);
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
						std::string valueStr = "{\"r\":" + std::to_string(v.r) + ",\"g\":" + std::to_string(v.g) + ",\"b\":" + std::to_string(v.b) + ",\"a\":" + std::to_string(v.a) + "}";
						ScriptSystem::SetScriptField(comp->GetGCHandle(), name.c_str(),
							valueStr);
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
							valueStr); // GameObject の参照を文字列として渡す
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
							valueStr); // Component の参照を文字列として渡す
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
			meta.properties.push_back(std::move(prop));
		}
		// ReflectionRegistry に登録
		ReflectionRegistry::Register(meta);
	}

	// すべてのスクリプトコンポーネントに対してスクリプトリロード処理を呼び出す
	if (scene)
	{
		for (auto& object : scene->GetAllSceneObjects())
		{
			for (auto* scriptComponent : object->GetComponents<ScriptComponent>())
			{
				if (scriptComponent)
				{
					scriptComponent->OnScriptReload();
				}
			}
		}
	}
}

void ScriptSystem::RequestScriptBuildAndReload()
{
	// スクリプトのビルドを要求する
	if (s_scriptWatcher)
	{
		s_scriptWatcher->RequestBuild();
	}
	else
	{
		LOG_ERROR("[ScriptSystem] Cannot request script build because the script watcher is not initialized.");
	}
}

void* ScriptSystem::CreateScript(const std::string& typeName, uint64_t ownerId, uint64_t componentId)
{
	if (!s_scriptHost) return nullptr;
	return s_scriptHost->GetCallbacks().CreateScript(typeName.c_str(), ownerId, componentId);
}

void ScriptSystem::ReleaseScript(void* gcHandle)
{
	if (!s_scriptHost || !gcHandle) return;
	s_scriptHost->GetCallbacks().ReleaseScript(gcHandle);
}

void ScriptSystem::AwakeScript(void* gcHandle)
{
	if (!s_scriptHost || !gcHandle) return;
	s_scriptHost->GetCallbacks().AwakeScript(gcHandle);
}

void ScriptSystem::StartScript(void* gcHandle)
{
	if (!s_scriptHost || !gcHandle) return;
	s_scriptHost->GetCallbacks().StartScript(gcHandle);
}

void ScriptSystem::UpdateScript(void* gcHandle)
{
	if (!s_scriptHost || !gcHandle) return;
	s_scriptHost->GetCallbacks().UpdateScript(gcHandle);
}

void ScriptSystem::OnDestroyScript(void* gcHandle)
{
	if (!s_scriptHost || !gcHandle) return;
	s_scriptHost->GetCallbacks().OnDestroyScript(gcHandle);
}

void ScriptSystem::OnEnableScript(void* gcHandle)
{
	if (!s_scriptHost || !gcHandle) return;
	s_scriptHost->GetCallbacks().OnEnableScript(gcHandle);
}

void ScriptSystem::OnDisableScript(void* gcHandle)
{
	if (!s_scriptHost || !gcHandle) return;
	s_scriptHost->GetCallbacks().OnDisableScript(gcHandle);
}

void* ScriptSystem::HotSwapScript(void* gcHandle, uint64_t ownerId, uint64_t componentId)
{
	if (!s_scriptHost || !gcHandle) return nullptr;

	const auto hotSwap = s_scriptHost->GetCallbacks().HotSwapScript;
	return hotSwap ? hotSwap(gcHandle, ownerId, componentId) : nullptr;
}

void* ScriptSystem::GetScriptFields(void* gcHandle)
{
	if (!s_scriptHost || !gcHandle) return nullptr;
	return s_scriptHost->GetCallbacks().GetScriptFields(gcHandle);
}

void ScriptSystem::SetScriptField(void* gcHandle, const std::string& fieldName, const std::string& value)
{
	if (!s_scriptHost || !gcHandle) return;

	const auto setField = s_scriptHost->GetCallbacks().SetScriptField;
	if (!setField)
	{
		LOG_ERROR("[ScriptSystem] SetScriptField callback is not initialized.");
		return;
	}

	setField(gcHandle, fieldName.c_str(), value.c_str());
}

static const CollisionInfoDto& ConvertCollisionInfoToPrimitiveData(const CollisionInfo& info)
{
	// CollisionInfo をスクリプト側で扱いやすい形式に変換する
	CollisionInfoDto dto{};
	dto.selfId = info.self ? info.self->GetId().Value() : 0;
	dto.selfColliderId = info.selfCollider ? info.selfCollider->GetId().Value() : 0;
	dto.otherId = info.other ? info.other->GetId().Value() : 0;
	dto.otherColliderId = info.otherCollider ? info.otherCollider->GetId().Value() : 0;
	dto.impulseX = info.impulse.x;
	dto.impulseY = info.impulse.y;
	dto.impulseZ = info.impulse.z;
	dto.contactCount = static_cast<uint32_t>(info.contacts.size());
	// 接触点の情報をコピーする（最大数は MAX_CONTACTS_PER_PAIR で制限）
	for (size_t i = 0; i < dto.contactCount && i < MAX_CONTACTS_PER_PAIR; ++i)
	{
		const auto& src = info.contacts[i];
		auto& dst = dto.contacts[i];
		dst.pointX = src.point.x;
		dst.pointY = src.point.y;
		dst.pointZ = src.point.z;
		dst.normalX = src.normal.x;
		dst.normalY = src.normal.y;
		dst.normalZ = src.normal.z;
		dst.separation = src.separation;
		dst.thisId = src.thisCollider ? src.thisCollider->GetOwner()->GetId().Value() : 0;
		dst.thisColliderId = src.thisCollider ? src.thisCollider->GetId().Value() : 0;
		dst.otherId = src.otherCollider ? src.otherCollider->GetOwner()->GetId().Value() : 0;
		dst.otherColliderId = src.otherCollider ? src.otherCollider->GetId().Value() : 0;
	}
	return dto;
}

static const TriggerInfoDto& ConvertTriggerInfoToPrimitiveData(const TriggerInfo& info)
{
	// TriggerInfo をスクリプト側で扱いやすい形式に変換する
	TriggerInfoDto dto{};
	dto.selfId = info.self ? info.self->GetId().Value() : 0;
	dto.selfColliderId = info.selfCollider ? info.selfCollider->GetId().Value() : 0;
	dto.otherId = info.other ? info.other->GetId().Value() : 0;
	dto.otherColliderId = info.otherCollider ? info.otherCollider->GetId().Value() : 0;
	return dto;
}

void ScriptSystem::OnCollisionEnterScript(void* gcHandle, const CollisionInfo& info)
{
	if (!s_scriptHost || !gcHandle) return;
	CollisionInfoDto dto = ConvertCollisionInfoToPrimitiveData(info);
	s_scriptHost->GetCallbacks().OnCollisionEnter(gcHandle, &dto);
}

void ScriptSystem::OnCollisionStayScript(void* gcHandle, const CollisionInfo& info)
{
	if (!s_scriptHost || !gcHandle) return;
	CollisionInfoDto dto = ConvertCollisionInfoToPrimitiveData(info);
	s_scriptHost->GetCallbacks().OnCollisionStay(gcHandle, &dto);
}

void ScriptSystem::OnCollisionExitScript(void* gcHandle, const CollisionInfo& info)
{
	if (!s_scriptHost || !gcHandle) return;
	CollisionInfoDto dto = ConvertCollisionInfoToPrimitiveData(info);
	s_scriptHost->GetCallbacks().OnCollisionExit(gcHandle, &dto);
}

void ScriptSystem::OnTriggerEnterScript(void* gcHandle, const TriggerInfo& info)
{
	if (!s_scriptHost || !gcHandle) return;
	TriggerInfoDto dto = ConvertTriggerInfoToPrimitiveData(info);
	s_scriptHost->GetCallbacks().OnTriggerEnter(gcHandle, &dto);
}

void ScriptSystem::OnTriggerStayScript(void* gcHandle, const TriggerInfo& info)
{
	if (!s_scriptHost || !gcHandle) return;
	TriggerInfoDto dto = ConvertTriggerInfoToPrimitiveData(info);
	s_scriptHost->GetCallbacks().OnTriggerStay(gcHandle, &dto);
}

void ScriptSystem::OnTriggerExitScript(void* gcHandle, const TriggerInfo& info)
{
	if (!s_scriptHost || !gcHandle) return;
	TriggerInfoDto dto = ConvertTriggerInfoToPrimitiveData(info);
	s_scriptHost->GetCallbacks().OnTriggerExit(gcHandle, &dto);
}

std::vector<std::string> ScriptSystem::GetRegisteredScriptNames()
{
	return s_tempNames;
}

void ScriptSystem::ClearScriptNames()
{
	s_tempNames.clear();
}

void ScriptSystem::AddTempScriptName(const std::string& name)
{
	// ここでスクリプト名をキャッシュに追加する
	s_tempNames.push_back(name);
}

void* ScriptSystem::GetScriptMeta(const std::string& scriptName)
{
	if (!s_scriptHost) return nullptr;
	return s_scriptHost->GetCallbacks().GetScriptMeta(scriptName.c_str());
}
