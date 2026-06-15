#include "pch.h"
#include "Component.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Utils/JsonFileHandler.h"

#ifdef USE_IMGUI
#include "Engine/EditorSupport/EditorSelection.h"
#include "Engine/EditorSupport/PropertyDrawer/PropertyDrawHelper.h"
#include "Engine/Editor/PropertyEditor.h"
#endif // USE_IMGUI


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

#ifdef USE_IMGUI
void Component::DrawProperty(const PropertyDrawContext& context)
{
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
				CurryEngine::PropertyEditor::DrawProperty(&prop, &context);
			}

			// クラスの境界
			ImGui::Separator(); // プロパティの区切り線

		}

		// プロパティ描画の終了
		IMGUI_PROPERTY_END();


#if 0
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
#endif // 0


	}

}
#endif // USE_IMGUI
