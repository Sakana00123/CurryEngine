#include "pch.h"
#include "Gadget.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Pin.h"
#include <Engine/UI/Text.h>
#include <Engine/Factory/GameObjectFactory.h>
#include <Engine/Easing/EasingComponent.h>
#include <Engine/Scenes/SceneManager.h>
#include "Engine/Audio/Audio.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
//REGISTER_COMPONENT(Gadget, "UserScripts")
REGISTER_COMPONENT_WITH_ATTRIBUTES(Gadget, "UserScripts", ComponentAttributes::HideInAddComponentMenu, {})


void Gadget::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	//Activate();
}

void Gadget::Finalize()
{
	//Deactivate();
}

void Gadget::LateUpdate(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	if (!IsDisabled())
	{
		// ガジェットが有効な場合の処理をここに実装します。例えば、特定の条件でアクションを実行するなどの処理を行うことができます。
		if (durabilityRectTransform)
		{
			durabilityRectTransform->SetAnchoredPositionByTransform(GetTransform());
			Vector2 anchoredPos = durabilityRectTransform->GetAnchoredPosition();
			anchoredPos.x += 30.0f; // 少し右にオフセット
			//anchoredPos.y += 30.0f; // 少し下にオフセット
			durabilityRectTransform->SetAnchoredPosition(anchoredPos);
		}
	}
}

void Gadget::OnAttachment()
{
	// ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
	// 例えば、ガジェットがアタッチされたオブジェクトがピンであれば、そのピンのガジェットとして登録するなどの処理を行うことができます。
	GameObject* parentPinObj = GetOwner()->GetParent();
	if (parentPinObj)
	{
		// ピンのコンポーネントをすべて無効にする
		for (auto& component : parentPinObj->GetAllComponents())
		{
			if (component && component->GetTypeName() != "BoxCollider" && component->GetTypeName() != "ParticleComponent")
			{
				component->SetEnabled(false);
			}
		}
	}

	// 耐久値を表示するテキストオブジェクトを生成して設定します。
	Scene* currentScene = SceneManager::GetCurrentScene();
	if (!currentScene)
	{
		return; // シーンが取得できない場合は処理を中断
	}
	GameObject* canvasObj = currentScene->GetObjectManager()->Find("DurabilityCanvas");
	GameObject* backgroundObj = GameObjectFactory::CreateImage(currentScene, "DurabilityBackground", canvasObj, L"./Assets/Texture/durabilitycontainer.png");
	durabilityRectTransform = backgroundObj->GetComponent<RectTransform>();
	if (durabilityRectTransform)
	{
		durabilityRectTransform->SetSize(Vector2(70, 70)); // 背景のサイズを設定
		durabilityRectTransform->SetAnchoredPositionByTransform(GetTransform()); // ボールの上に配置
		Vector2 anchoredPos = durabilityRectTransform->GetAnchoredPosition();
		anchoredPos.x += 30.0f; // 少し右にオフセット
		//anchoredPos.y += 30.0f; // 少し下にオフセット
		durabilityRectTransform->SetAnchoredPosition(anchoredPos);
		if (gadgetItemData.isDurabilityRoundDecrease)
		{
			durabilityRectTransform->SetAngle(45.0f); // 角度を45度に設定
		}
	}

	Image* backgroundImage = backgroundObj->GetComponent<Image>();

	GameObject* valueTextObj = GameObjectFactory::CreateText(currentScene, "DurabilityText", canvasObj);
	valueTextObj->SetParent(backgroundObj); // テキストを背景の子オブジェクトにする
	
	Text* durabilityText = valueTextObj->GetComponent<Text>();
	if (durabilityText)
	{
		std::wstring displayText = std::to_wstring(durability);
		durabilityText->SetText(displayText); // 耐久値を表示
		Color textColor = Color::White; // 白色
		durabilityText->SetColor(textColor); // 色を設定
		durabilityText->SetFontSize(36); // フォントサイズを設定
		durabilityText->SetHorizontalOverflow(Text::HorizontalOverflow::Overflow); // はみ出しを許可
		durabilityText->SetAlignment(Text::Alignment::MiddleCenter); // 中央揃え
		this->durabilityText = durabilityText; // 後で耐久値を更新するために保持

		if (gadgetItemData.isDurabilityRoundDecrease)
		{
			durabilityText->GetRectTransform()->SetAngle(-45.0f); // テキストの角度を-45度に設定して背景と合わせる
		}
	}
}

bool Gadget::CanActivate() const
{
	// ガジェットがアクティブ化できるかどうかを判断するための条件をここに実装します。例えば、耐久値が0以上であることなど。
	return gadgetActive;
}

void Gadget::OnBreak()
{
	// ガジェットが壊れたときの処理をここに実装します。
	// 親オブジェクト(ピン)を取得
	GameObject* parentPinObj = GetOwner()->GetParent();
	if (parentPinObj)
	{
		Pin* pin = parentPinObj->GetComponent<Pin>();
		if (pin && pin->IsEquipped())
		{
			// ピンのガジェットを外して、ピン側のコンポーネントを有効化する
			pin->PlayBreakEffect(); // ガジェットが壊れるエフェクトを再生
			pin->Unequip();
		}
	}

	// 破壊する
	{
		if (durabilityRectTransform)
		{
			durabilityRectTransform->GetOwner()->Destroy(); // 耐久値テキストを破壊
			durabilityRectTransform = nullptr; // RectTransformへの参照をクリア
			durabilityText = nullptr; // テキストへの参照をクリア
		}
		GetOwner()->Destroy();
	}

	// 破壊の効果音を再生
	Audio::PlayOneShot(L"./Assets/Sounds/SE/gadgetBreak.wav", 0.5f);
}

void Gadget::Activate()
{
	OnActivate();
	gadgetActive = true;
}

void Gadget::Deactivate()
{
	OnDeactivate();
	gadgetActive = false;
}

void Gadget::SetDurability(int newDurability)
{
	durability = newDurability;
	if (durabilityText)
	{
		durabilityText->SetText(std::to_wstring(durability));
	}
	if (durability <= 0)
	{
		OnBreak();
	}
}

void Gadget::PerformAction()
{
	//if (CanActivate())
	{
		OnAction();
	}
}

void Gadget::DecreaseDurability()
{
	durability--;
	if (durabilityText)
	{
		durabilityText->SetText(std::to_wstring(durability));
	}
	if (durability <= 0)
	{
		OnBreak();
	}
}
