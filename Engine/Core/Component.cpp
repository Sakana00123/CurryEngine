#include "pch.h"
#include "Component.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Utils/JsonFileHandler.h"
#include "Engine/EditorSupport/EditorSelection.h"

Transform* Component::GetTransform() const
{
	// 所属する GameObject が有効な状態であれば Transform を返す。無効な状態なら nullptr を返す。
	if (GetOwner())
	{
		return GetOwner()->GetTransform();
	}
	return nullptr;
}

Scene* Component::GetScene() const
{
	// 所属する GameObject が有効な状態であれば Scene を返す。無効な状態なら nullptr を返す。
	if (GetOwner())
	{
		return GetOwner()->GetScene();
	}
	return nullptr;
}

void Component::SetEnabled(bool set)
{
	// 自身の有効状態を更新
	enabledSelf = set;
	// 所属する GameObject に有効状態の変更を通知
	if (GetOwner())
	{
		GetOwner()->RefreshComponentActive(this);
	}
}

bool Component::IsEnabled() const
{
	// 現在の有効状態を返す
	return enabledInGame;
}

bool Component::IsEnabledSelf() const
{
	// 自身の有効状態を返す
	return enabledSelf;
}

void Component::Destroy() {
	GetOwner()->Destroy(this);
}

void Component::Destroy(GameObject* obj) {
	obj->Destroy();
}

static GameObject* InstantiateInternal(const std::string& prefabPath, Transform* parent, const Vector3& position, const Quaternion& rotation)
{
	Scene* currentScene = SceneManager::GetLoadingSceneOrCurrentScene();
	if (currentScene)
	{
		json prefabData;
		if (JsonFileHandler::LoadJsonFromFile(prefabData, prefabPath, JsonIOFormat::Binary))
		{
			GameObject* newObject = currentScene->GetObjectManager()->Instantiate(prefabData);
			if (newObject)
			{
				if (newObject->transform)
				{
					newObject->transform->SetPosition(position);
					newObject->transform->SetRotation(rotation);
				}
				if (parent)
				{
					newObject->SetParent(parent->GetOwner());
				}
				return newObject;
			}
		}
	}
	
	// 失敗した場合は nullptr を返す
	return nullptr;
}

static GameObject* InstantiateInternal(GameObject* prefabObject, Transform* parent, const Vector3& position, const Quaternion& rotation)
{
	Scene* currentScene = SceneManager::GetLoadingSceneOrCurrentScene();
	if (currentScene)
	{
		if (prefabObject)
		{
			GameObject* newObject = currentScene->objectManager->Duplicate(prefabObject);
			if (newObject)
			{
				newObject->transform->SetPosition(position);
				newObject->transform->SetRotation(rotation);
				if (parent)
				{
					newObject->SetParent(parent->GetOwner());
				}
				return newObject;
			}
		}
	}

	// 失敗した場合は nullptr を返す
	return nullptr;
}

GameObject* Component::Instantiate(const std::string& prefabPath, Transform* parent, const Vector3& position, const Quaternion& rotation)
{
	return InstantiateInternal(prefabPath, parent, position, rotation);
}

GameObject* Component::Instantiate(const std::string& prefabPath, const Vector3& position, const Quaternion& rotation)
{
	return InstantiateInternal(prefabPath, nullptr, position, rotation);
}

GameObject* Component::Instantiate(GameObject* prefab, Transform* parent, const Vector3& position, const Quaternion& rotation)
{
	return InstantiateInternal(prefab, parent, position, rotation);
}

GameObject* Component::Instantiate(GameObject* prefab, const Vector3& position, const Quaternion& rotation)
{
	return InstantiateInternal(prefab, nullptr, position, rotation);
}

// --- デフォルトプロパティ描画（リフレクションで取得したフィールドを描画） ---

void Component::DrawProperty()
{
#ifdef USE_IMGUI
	//auto& selectAll = this->GetScene()->objectManager->GetEditorSelection()->GetAll();

	//// 選択中のすべてのオブジェクトに対して処理を適用するヘルパー関数
	//auto applyToSelectedObjects = [&](const std::function<void(Component*)>& func) {
	//	bool applied = false;
	//	if (this->GetScene() && this->GetScene()->objectManager && this->GetScene()->objectManager->GetEditorSelection()) {
	//		if (!selectAll.empty()) {
	//			for (auto& obj : selectAll) {
	//				if (auto go = obj.get()) {
	//					if (auto comp = go->GetComponentByTypeName(this->GetTypeName()).get()) {
	//						func(comp);
	//					}
	//				}
	//			}
	//			applied = true;
	//		}
	//	}
	//	if (!applied) {
	//		func(this);
	//	}
	//	};


	// リフレクションで取得したフィールドを描画
	std::vector<std::string> typeNames; // クラス階層の型名を格納するベクター
	std::vector<const ClassMeta*> classMetas; // クラス階層のメタデータを格納するベクター
	{
		const ClassMeta* meta = GetClassMeta();
		while (meta)
		{
			typeNames.push_back(meta->name); // 型名を追加
			classMetas.push_back(meta); // メタデータを追加
			if (meta->bases.empty())
			{
				break; // 継承元がない場合は終了
			}
			meta = ReflectionRegistry::FindClass(meta->bases.front()); // 最初の継承元を取得してループを続ける
		}
	}

	{
		// プロパティ描画の開始
		IMGUI_PROPERTY_BEGIN();

		for (int i = static_cast<int>(classMetas.size()) - 1; i >= 0; --i) // クラス階層の順序で描画するために逆順でループ
		{
			const ClassMeta* meta = classMetas[i];

			for (const auto& prop : meta->properties)
			{
				if (prop.GetAttribute("HideInInspector") != nullptr)
				{
					continue; // HideInInspector 属性がある場合は描画しない
				}
				ImGui::PushID(prop.name.c_str());

				// フィールドの値を取得
				std::any propertyAny = prop.getter(this);

				// ラベルの取得
				const char* label = prop.name.c_str();

				// ツールチップの取得
				const char* tooltip = nullptr;
				if (auto* tooltipAttr = prop.GetAttribute("Tooltip")) // Tooltip 属性があれば、引数からツールチップを取得
				{
					if (!tooltipAttr->args.empty())
					{
						tooltip = tooltipAttr->args[0].c_str();
					}
				}

				// 属性の取得
				bool readOnly = prop.GetAttribute("ReadOnly") != nullptr; // ReadOnly 属性があるかどうか

				// int/float の描画に必要な変数の初期値
				float speed = 1.0f;
				ImGuiSliderFlags sliderFlags = 0;
				// int/float の描画に必要な属性の取得
				{
					if (auto* speedAttr = prop.GetAttribute("Speed")) // Speed 属性があれば、引数から speed を取得
					{
						if (!speedAttr->args.empty())
						{
							speed = std::stof(speedAttr->args[0]);
						}
					}
					if (readOnly) // ReadOnly 属性があれば、flags に ImGuiSliderFlags_ReadOnly をセット
					{
						sliderFlags |= ImGuiSliderFlags_NoInput; // ImGui には ReadOnly フラグがないため、入力を無効化するフラグを使用
						sliderFlags |= ImGuiSliderFlags_AlwaysClamp; // 入力を無効化する場合でも、ドラッグでの編集は可能にするために常にクランプするフラグをセット
					}
				}


				// フィールドの型に応じて描画
				if (prop.type == "int")
				{
					static int prevValue; /* 前回の値を保持する静的変数 */
					int value = std::any_cast<int>(propertyAny);
					IMGUI_PROPERTY_EX(label, tooltip);
					if (readOnly) ImGui::BeginDisabled(); // ReadOnly 属性がある場合は描画を無効化
					int min = 0, max = 0;
					if (auto* rangeAttr = prop.GetAttribute("Range")) // Range 属性があれば、引数から min, max, (speed) を取得
					{
						if (rangeAttr->args.size() >= 2)
						{
							min = std::stoi(rangeAttr->args[0]);
							max = std::stoi(rangeAttr->args[1]);
						}
					}
					const char* format = "%d";
					if (auto* formatAttr = prop.GetAttribute("Format")) // Format 属性があれば、引数から format を取得
					{
						if (!formatAttr->args.empty())
						{
							format = formatAttr->args[0].c_str();
						}
					}

					// GUI のドラッグ操作で値を編集
					ImGui::DragInt("##int", &value, speed, min, max, format, sliderFlags);
					if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
					{
						prevValue = value;
					}
					if (ImGui::IsItemDeactivatedAfterEdit()) /* 編集終了後にコマンドを発行 */
					{
						int newValue = value; /* 現在の値を取得 */
						if (newValue != prevValue)
						{
							IMGUI_PROPERTY_COMMAND_INT(label, newValue, prevValue);
						}
						prevValue = newValue; /* 前回の値を更新 */
					}
					if (readOnly) ImGui::EndDisabled(); // ReadOnly 属性がある場合は描画の無効化を終了
				}
				else if (prop.type == "float")
				{
					static float prevValue; /* 前回の値を保持する静的変数 */
					float value = std::any_cast<float>(propertyAny);
					IMGUI_PROPERTY_EX(label, tooltip);
					if (readOnly) ImGui::BeginDisabled(); // ReadOnly 属性がある場合は描画を無効化
					float min = 0.0f, max = 0.0f;
					if (auto* rangeAttr = prop.GetAttribute("Range")) // Range 属性があれば、引数から min, max, (speed) を取得
					{
						if (rangeAttr->args.size() >= 2)
						{
							min = std::stof(rangeAttr->args[0]);
							max = std::stof(rangeAttr->args[1]);
						}
					}
					const char* format = "%.3f";
					if (auto* formatAttr = prop.GetAttribute("Format")) // Format 属性があれば、引数から format を取得
					{
						if (!formatAttr->args.empty())
						{
							format = formatAttr->args[0].c_str();
						}
					}

					// GUI のドラッグ操作で値を編集
					ImGui::DragFloat("##float", &value, speed, min, max, format, sliderFlags);
					if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
					{
						prevValue = value;
					}
					if (ImGui::IsItemDeactivatedAfterEdit()) /* 編集終了後にコマンドを発行 */
					{
						float newValue = value; /* 現在の値を取得 */
						if (newValue != prevValue)
						{
							IMGUI_PROPERTY_COMMAND_FLOAT(label, newValue, prevValue);
						}
						prevValue = newValue; /* 前回の値を更新 */
					}
					if (readOnly) ImGui::EndDisabled(); // ReadOnly 属性がある場合は描画の無効化を終了
				}
				else if (prop.type == "bool")
				{
					static bool prevValue; /* 前回の値を保持する静的変数 */
					bool value = std::any_cast<bool>(propertyAny);
					IMGUI_PROPERTY_EX(label, tooltip);
					if (readOnly) ImGui::BeginDisabled(); // ReadOnly 属性がある場合は描画を無効化
					ImGui::Checkbox("##bool", &value);
					if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
					{
						prevValue = value;
					}
					if (ImGui::IsItemDeactivatedAfterEdit()) /* 編集終了後にコマンドを発行 */
					{
						bool newValue = value; /* 現在の値を取得 */
						if (newValue != prevValue)
						{
							IMGUI_PROPERTY_COMMAND_BOOL(label, newValue, prevValue);
						}
						prevValue = newValue; /* 前回の値を更新 */
					}
					if (readOnly) ImGui::EndDisabled(); // ReadOnly 属性がある場合は描画の無効化を終了
				}
				else if (prop.type == "std::string" || prop.type == "string")
				{
					std::string value = std::any_cast<std::string>(propertyAny);
					std::string oldValue = value;
					IMGUI_PROPERTY_EX(label, tooltip);
					if (readOnly) ImGui::BeginDisabled(); // ReadOnly 属性がある場合は描画を無効化
					char buffer[256];
					strncpy_s(buffer, value.c_str(), sizeof(buffer));
					buffer[sizeof(buffer) - 1] = '\0'; // バッファの最後を null で終端
					static std::string prevValue = value; /* 前回の値を保持する静的変数 */
					bool valueChanged = false; // 値が変更されたかを追跡するフラグ
					valueChanged |= ImGui::InputText("##string", buffer, sizeof(buffer));
					if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
					{
						prevValue = value;
					}
					if (ImGui::IsItemDeactivatedAfterEdit()) /* 編集終了後にコマンドを発行 */
					{
						std::string newValue(buffer); /* 現在の値を取得 */
						if (newValue != prevValue)
						{
							IMGUI_PROPERTY_COMMAND_STRING(label, newValue, oldValue);
						}
						prevValue = newValue; /* 前回の値を更新 */
					}
					if (valueChanged) // 値が変更された場合のみ std::string を更新
					{
						value = buffer;
						if (value.length() > 255)
						{
							value.resize(255); // 256 - 1 (null terminator)
						}
					}
					if (readOnly) ImGui::EndDisabled(); // ReadOnly 属性がある場合は描画の無効化を終了
				}
				else if (prop.type == "Vector2" || (prop.type.find("XMFLOAT2") != std::string::npos))
				{
					static Vector2 prevValue; /* 前回の値を保持する静的変数 */
					Vector2 value = std::any_cast<Vector2>(propertyAny);
					IMGUI_PROPERTY_EX(label, tooltip);
					if (readOnly) ImGui::BeginDisabled(); // ReadOnly 属性がある場合は描画を無効化
					bool valueChanged = false; // 値が変更されたかを追跡するフラグ
					valueChanged |= ImGui::DragFloat2("##vector2", &value.x);
					if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
					{
						prevValue = value;
					}
					if (ImGui::IsItemDeactivatedAfterEdit()) /* 編集終了後にコマンドを発行 */
					{
						Vector2 newValue = value; /* 現在の値を取得 */
						if (!Vector2::Equal(newValue, prevValue))
						{
							IMGUI_PROPERTY_COMMAND_VECTOR2(label, newValue, prevValue);
						}
						prevValue = newValue; /* 前回の値を更新 */
					}
					if (readOnly) ImGui::EndDisabled(); // ReadOnly 属性がある場合は描画の無効化を終了
				}
				else if (prop.type == "Vector3" || (prop.type.find("XMFLOAT3") != std::string::npos))
				{
					static Vector3 prevValue; /* 前回の値を保持する静的変数 */
					Vector3 value = std::any_cast<Vector3>(propertyAny);
					IMGUI_PROPERTY_EX(label, tooltip);
					if (readOnly) ImGui::BeginDisabled(); // ReadOnly 属性がある場合は描画を無効化
					bool valueChanged = false; // 値が変更されたかを追跡するフラグ
					valueChanged |= ImGui::DragFloat3("##vector3", &value.x);
					if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
					{
						prevValue = value;
					}
					if (ImGui::IsItemDeactivatedAfterEdit()) /* 編集終了後にコマンドを発行 */
					{
						Vector3 newValue = value; /* 現在の値を取得 */
						if (!Vector3::Equal(newValue, prevValue))
						{
							IMGUI_PROPERTY_COMMAND_VECTOR3(label, newValue, prevValue);
						}
						prevValue = newValue; /* 前回の値を更新 */
					}
					if (readOnly) ImGui::EndDisabled(); // ReadOnly 属性がある場合は描画の無効化を終了
				}
				else if (prop.type == "Quaternion" || (prop.type.find("XMFLOAT4") != std::string::npos))
				{
					if (name == "Transform" && prop.name == "rotation")
					{
						static Quaternion prevValue; /* 前回の値を保持する静的変数 */
						static Vector3 editorEuler; /* 編集中のオイラー角を保持する静的変数 */
						static bool isEditing = false; /* 編集中かどうかを追跡するフラグ */
						Quaternion value = std::any_cast<Quaternion>(propertyAny);
						if (!isEditing) /* 編集開始前に現在の値をオイラー角に変換して保存 */
						{
							editorEuler = Transform::QuaternionToEuler(value);
						}

						IMGUI_PROPERTY_EX(label, tooltip);
						if (readOnly) ImGui::BeginDisabled(); // ReadOnly 属性がある場合は描画を無効化
						bool valueChanged = false; // 値が変更されたかを追跡するフラグ
						valueChanged |= ImGui::DragFloat3("##rotation", &editorEuler.x);
						if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
						{
							prevValue = value;
						}
						if (ImGui::IsItemDeactivatedAfterEdit()) /* 編集終了後にコマンドを発行 */
						{
							// 変更されたオイラー角と前回のオイラー角の差分を計算してクォータニオンに変換
							Quaternion newValue = Transform::EulerToQuaternion(editorEuler);
							{
								IMGUI_PROPERTY_COMMAND(label, Quaternion, newValue, prevValue,
									"(" + std::to_string(newValue.x) + ", " + std::to_string(newValue.y) + ", " + std::to_string(newValue.z) + ", " + std::to_string(newValue.w) + ")",
									"(" + std::to_string(prevValue.x) + ", " + std::to_string(prevValue.y) + ", " + std::to_string(prevValue.z) + ", " + std::to_string(prevValue.w) + ")");
							}
							prevValue = newValue; /* 前回の値を更新 */
							GetTransform()->SetRotation(newValue); /* Transform の回転を更新 */
							isEditing = false; /* 編集終了 */
						}
						if (valueChanged) /* 値が変更された場合は Transform の回転を更新 */
						{
							Quaternion newValue = Transform::EulerToQuaternion(editorEuler);
							GetTransform()->SetRotation(newValue);
							isEditing = true; /* 編集中 */
						}
					}
					else // Transform の rotation 以外の Quaternion フィールドはそのまま編集できるようにする
					{
						static Quaternion prevValue; /* 前回の値を保持する静的変数 */
						Quaternion value = std::any_cast<Quaternion>(propertyAny);
						IMGUI_PROPERTY_EX(label, tooltip);
						if (readOnly) ImGui::BeginDisabled(); // ReadOnly 属性がある場合は描画を無効化
						bool valueChanged = false; // 値が変更されたかを追跡するフラグ
						valueChanged |= ImGui::DragFloat4("##quaternion", &value.x);
						if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
						{
							prevValue = value;
						}
						if (ImGui::IsItemDeactivatedAfterEdit()) /* 編集終了後にコマンドを発行 */
						{
							Quaternion newValue = value; /* 現在の値を取得 */
							if (newValue.x != prevValue.x || newValue.y != prevValue.y || newValue.z != prevValue.z || newValue.w != prevValue.w)
							{
								IMGUI_PROPERTY_COMMAND(label, Quaternion, newValue, prevValue,
									"(" + std::to_string(newValue.x) + ", " + std::to_string(newValue.y) + ", " + std::to_string(newValue.z) + ", " + std::to_string(newValue.w) + ")",
									"(" + std::to_string(prevValue.x) + ", " + std::to_string(prevValue.y) + ", " + std::to_string(prevValue.z) + ", " + std::to_string(prevValue.w) + ")");
							}
							prevValue = newValue; /* 前回の値を更新 */
						}
					}
					if (readOnly) ImGui::EndDisabled(); // ReadOnly 属性がある場合は描画の無効化を終了
				}
				else if (prop.type == "Color")
				{
					static Color prevValue; /* 前回の値を保持する静的変数 */
					Color value = std::any_cast<Color>(propertyAny);
					IMGUI_PROPERTY_EX(label, tooltip);
					if (readOnly) ImGui::BeginDisabled(); // ReadOnly 属性がある場合は描画を無効化
					bool valueChanged = false; // 値が変更されたかを追跡するフラグ
					valueChanged |= ImGui::ColorEdit4("##color", &value.r);
					if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
					{
						prevValue = value;
					}
					if (ImGui::IsItemDeactivatedAfterEdit()) /* 編集終了後にコマンドを発行 */
					{
						Color newValue = value; /* 現在の値を取得 */
						if (newValue.r != prevValue.r || newValue.g != prevValue.g || newValue.b != prevValue.b || newValue.a != prevValue.a)
						{
							IMGUI_PROPERTY_COMMAND_COLOR(label, newValue, prevValue);
						}
						prevValue = newValue; /* 前回の値を更新 */
					}
					if (readOnly) ImGui::EndDisabled(); // ReadOnly 属性がある場合は描画の無効化を終了
				}
				else if (prop.type == "ObjectId")
				{
					if (auto referenceAttr = prop.GetAttribute("ObjectReference")) // ObjectReference 属性がある場合のみドロップを受け入れる
					{
						if (referenceAttr->args.empty())
						{
							Console::LogWarning("ObjectReference attribute on property: " + prop.name + " in component: " + name + " is missing the reference type argument.");
							ImGui::PopID();
							continue;
						}
						static ObjectId prevValue; /* 前回の値を保持する静的変数 */
						ObjectId value = std::any_cast<ObjectId>(propertyAny);
						IMGUI_PROPERTY_EX(label, tooltip);
						if (readOnly) ImGui::BeginDisabled(); // ReadOnly 属性がある場合は描画を無効化
						std::string refTypeName = referenceAttr->args[0]; // Reference 属性の引数は参照先の型名 (例: "Transform", "GameObejct") を想定
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
							const auto& componentCacheMap = SceneManager::GetLoadingSceneOrCurrentScene()->objectManager->GetComponentCacheMap();
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
											const auto& prevObj = ObjectManager::Find(prevValue);
											const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
											std::string newValueLog = newComponentId.IsValid() ? (std::to_string(newComponentId.Value()) + "(" + refTypeDisplay + ": " + droppedObjName + ")") : "None";
											std::string prevValueLog = prevValue.IsValid() ? (std::to_string(prevValue.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")") : "None";
											IMGUI_PROPERTY_COMMAND(label, ObjectId, newComponentId, prevValue,
												newValueLog,
												prevValueLog);
											prevValue = newComponentId; // 前回の値を更新
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
										const auto& prevObj = ObjectManager::Find(prevValue);
										const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
										std::string newValueLog = droppedId.IsValid() ? (std::to_string(droppedId.Value()) + "(" + refTypeDisplay + ": " + droppedObjName + ")") : "None";
										std::string prevValueLog = prevValue.IsValid() ? (std::to_string(prevValue.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")") : "None";

										IMGUI_PROPERTY_COMMAND(label, ObjectId, droppedId, prevValue,
											newValueLog,
											prevValueLog);
										prevValue = droppedId; // 前回の値を更新
									}
								}
							}
							ImGui::EndDragDropTarget(); // ドロップ操作の受け入れを終了
						}
						if (readOnly) ImGui::EndDisabled(); // ReadOnly 属性がある場合は描画の無効化を終了
						else // ReadOnly 属性がない場合は、編集用の追加 UI を表示
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

								IMGUI_PROPERTY_COMMAND(label, ObjectId, ObjectId::Invalid(), oldValue,
									newValueLog,
									prevValueLog);
								prevValue = ObjectId::Invalid(); // 前回の値を更新
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
									const auto& allObjects = currentScene->objectManager->GetAll();
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
													ObjectId oldValue = value;
													value = obj->id; // フィールドに選択されたオブジェクトの ID を設定

													std::string refTypeDisplay = "GameObject";
													const auto& prevObj = ObjectManager::Find(oldValue);
													const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
													const auto& newValueLog = std::to_string(obj->id.Value()) + "(" + refTypeDisplay + ": " + obj->GetName() + ")";
													const auto& prevValueLog = oldValue.IsValid() ? (std::to_string(oldValue.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")") : "None";

													IMGUI_PROPERTY_COMMAND(label, ObjectId, obj->id, oldValue,
														std::to_string(obj->id.Value()),
														std::to_string(oldValue.Value()));
													prevValue = obj->id; // 前回の値を更新
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
																ObjectId oldValue = value;
																value = refComponent->id; // フィールドに選択されたオブジェクトの ID を設定
																std::string refTypeDisplay = "Component(" + refTypeName + ")";
																const auto& prevObj = ObjectManager::Find(oldValue);
																const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
																const auto& newValueLog = std::to_string(refComponent->id.Value()) + "(" + refTypeDisplay + ": " + obj->GetName() + ")";
																const auto& prevValueLog = oldValue.IsValid() ? (std::to_string(oldValue.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")") : "None";
																IMGUI_PROPERTY_COMMAND(label, ObjectId, refComponent->id, oldValue,
																	newValueLog,
																	prevValueLog);
																prevValue = refComponent->id; // 前回の値を更新
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
																	ObjectId oldValue = value;
																	value = comp->id; // フィールドに選択されたオブジェクトの ID を設定

																	std::string refTypeDisplay = "Component(" + refTypeName + ")";
																	const auto& prevObj = ObjectManager::Find(oldValue);
																	const auto& prevObjName = prevObj ? prevObj->GetName() : "Unknown";
																	const auto& newValueLog = std::to_string(comp->id.Value()) + "(" + refTypeDisplay + ": " + obj->GetName() + ")";
																	const auto& prevValueLog = oldValue.IsValid() ? (std::to_string(oldValue.Value()) + "(" + refTypeDisplay + ": " + prevObjName + ")") : "None";

																	IMGUI_PROPERTY_COMMAND(label, ObjectId, comp->id, oldValue,
																		newValueLog,
																		prevValueLog);
																	prevValue = comp->id; // 前回の値を更新
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
				}
				else
				{
					Console::LogWarning("Unsupported property type: " + prop.type + " for property: " + prop.name + " in component: " + name);
				}

				ImGui::PopID();
			}

			// クラスの境界
			ImGui::Separator(); // プロパティの区切り線

		}

		// プロパティ描画の終了
		IMGUI_PROPERTY_END();


		// テスト用で関数呼び出しのImGui::Buttonを追加
		if (ImGui::CollapsingHeader("Test Functions"))
		{
			for (const auto& meta : classMetas)
			{
				if (!meta)
					continue;

				for (const auto& func : meta->methods)
				{
					if (!func.invoker)
						continue; // 関数が呼び出せない場合はスキップ
					ImGui::PushID(func.name.c_str());
					if (ImGui::Button(func.name.c_str()))
					{
						func.InvokeVoid(this, {}); // 引数なしで関数を呼び出す
					}
					ImGui::PopID();
				}
			}
		}


	}

#endif // USE_IMGUI
}
