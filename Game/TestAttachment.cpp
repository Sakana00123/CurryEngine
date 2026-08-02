#include "pch.h"
#include "TestAttachment.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Core/ObjectManager.h"
#include "Engine/Utils/JsonFileHandler.h"
#include "PreserveValue.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Physics/Physics.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Rendering/Camera/CameraSystem.h"
#include "Engine/Rendering/Camera/CameraComponent.h"
#include "Engine/Physics/Collider.h"
#include "Flipper.h"
#include "ChangeballColor.h"

// スクリプトのクラス名とコンポーネントカテゴリを登録します。必要に応じて追加の属性を指定できます。
REGISTER_COMPONENT(TestAttachment, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(TestAttachment, "UserScripts", ComponentAttributes::None, {})

void TestAttachment::Start()
{
	// コンポーネントが開始されたときの初期化処理を実装します。

}

void TestAttachment::Update(float deltaTime)
{
	// フレームごとの更新処理を実装します。
}

//void TestAttachment::DrawProperty()
//{
//	// エディタでプロパティを描画するための初期化処理を実装します。
//#ifdef USE_IMGUI
//
//	Component::DrawProperty(); // 基底クラスのプロパティ描画を呼び出す (必要に応じて追加)
//
//	// ここに TestAttachment 特有のプロパティ描画コードを追加します。
//	ImGui::Separator();
//
//	if (ImGui::Button("Attach"))
//	{
//		GameObject* pinObject = ObjectManager::Find(pinObj);
//		GameObject* generatedObject = Instantiate(prefabPath, Vector3::Zero);
//		
//		if (pinObject && generatedObject)
//		{
//			if (Pin* pin = pinObject->GetComponent<Pin>())
//			{
//				// Pin コンポーネントが存在する場合の初期化処理を実装する
//				if (AttachToPin(generatedObject, pin))
//				{
//					Console::Log("Object attached successfully via ImGui button.");
//				}
//				else
//				{
//					Console::LogWarning("Failed to attach object via ImGui button.");
//					generatedObject->Destroy(); // 失敗した場合は生成したオブジェクトを破壊する
//				}
//			}
//		}
//	}
//
//	if (ImGui::Button("Detach"))
//	{
//		// ボタンがクリックされたときの初期化処理を実装します。
//		GameObject* pinObject = ObjectManager::Find(pinObj);
//		if (pinObject)
//		{
//			if (Pin* pin = pinObject->GetComponent<Pin>())
//			{
//				// Pin コンポーネントが存在する場合の初期化処理を実装します。
//				if (GameObject* unequippedObject = pin->Unequip())
//				{
//					// 装備解除されたオブジェクトが存在する場合の初期化処理を実装します。
//					unequippedObject->Destroy(); // 例: 装備解除オブジェクトを破壊する場合
//				}
//			}
//		}
//	}
//
//#endif // USE_IMGUI
//}

bool TestAttachment::AttachToPin(std::string& path, Pin* hitPin) const
{

	if (path.empty()|| hitPin == nullptr) 
	{
		Console::LogWarning("AttachToPin: Prefab path is empty or hitPin is null.");
		return false;
	}

	//アタッチ処理
	{
		if (hitPin != nullptr)
		{
			// すでにアタッチされている場合は置けない
			if (hitPin->IsEquipped())
			{
				Console::LogWarning("Pin is already equipped. Cannot attach.");
				return false;
			}

			//ゴールデンピンにも置けないようにする
			if (hitPin->IsGoldenPin())
			{
				Console::LogWarning("Cannot attach to a Golden Pin.");
				return false;
			}

			// 引数で受け取ったパスを使ってInstantiateする
			GameObject* generatedObject = Instantiate(path, Vector3::Zero);
			if (!generatedObject) return false;

			Quaternion spawnRotation = generatedObject->GetTransform()->GetRotation();
			Vector3 spawnScale = generatedObject->GetTransform()->GetScale();

			Vector3 spawnPosition = Vector3(0, -0.25f, 0); // オブジェクトの中心

			//changeballcolorは原点に配置
			if (generatedObject->GetComponent<ChangeballColor>())
			{
				spawnPosition = Vector3::Zero;
			}

			hitPin->Equip(generatedObject, spawnPosition, spawnRotation, spawnScale);

			Console::Log("Attached successfully to " + hitPin->GetOwner()->GetName());
			return true;
		}
		else
		{
			Console::LogWarning("Clicked object is not a Pin. Name: " + hitPin->GetOwner()->GetName());
			return false;
		}
	}

	Console::LogWarning("Raycast did not hit anything.");
	return false;
}

bool TestAttachment::AttachToPin(GameObject* prefab, Pin* hitPin) const
{
	if (prefab == nullptr || hitPin == nullptr)
	{
		Console::LogWarning("AttachToPin: Prefab is null or hitPin is null.");
		return false;
	}
	//アタッチ処理
	{
		if (hitPin != nullptr)
		{
			// すでにアタッチされている場合は置けない
			if (hitPin->IsEquipped())
			{
				Console::LogWarning("Pin is already equipped. Cannot attach.");
				return false;
			}
			//ゴールデンピンにも置けないようにする
			if (hitPin->IsGoldenPin())
			{
				Console::LogWarning("Cannot attach to a Golden Pin.");
				return false;
			}
			Vector3 spawnPosition = Vector3(0, -0.25f, 0); // オブジェクトの中心
			Quaternion spawnRotation = prefab->GetTransform()->GetRotation();
			Vector3 spawnScale = prefab->GetTransform()->GetScale();
			hitPin->Equip(prefab, spawnPosition, spawnRotation, spawnScale);
			Console::Log("Attached successfully to " + hitPin->GetOwner()->GetName());
			return true;
		}
		else
		{
			Console::LogWarning("Clicked object is not a Pin. Name: " + hitPin->GetOwner()->GetName());
			return false;
		}
	}
	Console::LogWarning("Raycast did not hit anything.");
	return false;
}
