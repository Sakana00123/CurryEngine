#include "pch.h"
#include "GadgetItemView.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Core/GameObject.h"
#include "ItemTooltipController.h"
#include <Engine\UI\Text.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(GadgetItemView, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(GadgetItemView, "UserScripts", ComponentAttributes::None, {})


void GadgetItemView::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void GadgetItemView::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void GadgetItemView::ShowTooltip(Transform* attachPoint, const GadgetItemData& data)
{
	// ツールチップを表示する処理をここに実装します。
	if (GameObject* tooltipObject = GetScene()->FindGameObjectById(tooltipReference))
	{
		tooltipObject->SetActive(true); // ツールチップオブジェクトをアクティブにする
	}
	else
	{
		Console::LogError("Tooltip GameObject not found for the given tooltipReference in GadgetItemView.");
	}

	//if (auto* backgroundImage = GetScene()->FindComponentById<Image>(backgroundImageReference))
	//{
	//	if (!data.backgroundImagePath.empty())
	//	{
	//		backgroundImage->SetSource(StringToWstring(data.backgroundImagePath).c_str()); // 背景画像を設定
	//	}
	//	else
	//	{
	//		Console::LogError("Background image path is empty in GadgetItemData for GadgetItemView.");
	//	}
	//}
	//else
	//{
	//	Console::LogError("Background Image component not found for the given backgroundImageReference in GadgetItemView.");
	//}

	if (nameTextReference.IsValid())
	{
		if (auto* nameText = GetScene()->FindComponentById<Text>(nameTextReference))
		{
			nameText->SetText(StringToWstring(data.name)); // アイテム名を設定
		}
		else
		{
			Console::LogError("Name Text component not found for the given nameTextReference in GadgetItemView.");
		}
	}
	else
	{
		Console::LogError("Invalid tooltipControllerReference in GadgetItemView.");
	}
	if (descriptionTextReference.IsValid())
	{
		if (auto* descriptionText = GetScene()->FindComponentById<Text>(descriptionTextReference))
		{
			descriptionText->SetText(StringToWstring(data.description)); // アイテム説明を設定
		}
		else
		{
			Console::LogError("Description Text component not found for the given descriptionTextReference in GadgetItemView.");
		}
	}
	else
	{
		Console::LogError("Invalid descriptionTextReference in GadgetItemView.");
	}

	// attachPoint の位置にツールチップを表示する処理
	if (attachPoint)
	{
		// ツールチップの位置を attachPoint に合わせる
		if (RectTransform* rectTransform = GetRectTransform())
		{
			rectTransform->SetAnchoredPositionByTransform(attachPoint);
		}
		else
		{
			Console::LogError("RectTransform not found for GadgetItemView when trying to show tooltip.");
		}
	}
	else
	{
		Console::LogError("Invalid attachPoint provided to ShowTooltip in GadgetItemView.");
	}

}

void GadgetItemView::HideTooltip()
{
	// ツールチップを非表示にする処理をここに実装します。
	if (GameObject* tooltipObject = GetScene()->FindGameObjectById(tooltipReference))
	{
		tooltipObject->SetActive(false); // ツールチップオブジェクトを非アクティブにする
	}
	else
	{
		Console::LogError("Tooltip GameObject not found for the given tooltipReference in GadgetItemView.");
	}
}