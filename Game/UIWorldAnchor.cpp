#include "pch.h"
#include "UIWorldAnchor.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Core/GameObject.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
//REGISTER_COMPONENT(UIWorldAnchor, "UserScripts")
REGISTER_COMPONENT_WITH_ATTRIBUTES(UIWorldAnchor, "UserScripts", ComponentAttributes::ExecuteInEditMode, {})


void UIWorldAnchor::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void UIWorldAnchor::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	if (targetTransformReference.IsValid())
	{
		if (Transform* targetTransform = GetScene()->FindComponentById<Transform>(targetTransformReference))
		{

			if (RectTransform* rectTransform = GetRectTransform())
			{
				rectTransform->SetAnchoredPositionByTransform(targetTransform);
			}
			else
			{
				Console::Log("Error: RectTransform not found for UIWorldAnchor.");
			}
		}
		else
		{
			Console::Log("Error: Target Transform not found for UIWorldAnchor.");
		}
	}
}