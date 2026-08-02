#include "pch.h"
#include "PinLayoutGroup.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
//REGISTER_COMPONENT(PinLayoutGroup, "UserScripts")
REGISTER_COMPONENT_WITH_ATTRIBUTES(PinLayoutGroup, "UserScripts", ComponentAttributes::None, {})


void PinLayoutGroup::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void PinLayoutGroup::UpdateLayout()
{
	// 毎フレームの更新処理をここに実装します。

	// レイアウトの更新処理をここに実装します。例えば、子オブジェクトを取得して、maxPerRow と spacing に基づいて配置する処理など。
	int childCount = GetOwner()->GetChildren().size();

	int maxPerRow1 = isFirstRowLonger ? maxPerRow : minPerRow; // 一列目の最大数
	int maxPerRow2 = isFirstRowLonger ? minPerRow : maxPerRow; // 二列目の最大数

	// 行ごとにじゃばらになるように配置する
	// z座標を行数に応じて少しずつ下げることで、ピンが重ならないようにする
	int currentRow = 0;
	int currentCol = 0;
	for (int i = 0; i < childCount; ++i)
	{
		GameObject* child = GetOwner()->GetChildren()[i];
		Vector3 newPosition;
		if (currentRow % 2 == 0) // 一列目
		{
			newPosition = startPosition + Vector3(currentCol * spacing, 0, -currentRow * spacing);
			currentCol++;
			if (currentCol >= maxPerRow1)
			{
				currentCol = 0;
				currentRow++;
			}
		}
		else // 二列目
		{
			newPosition = startPosition + Vector3((currentCol + 0.5f) * spacing, 0, -currentRow * spacing);
			currentCol++;
			if (currentCol >= maxPerRow2)
			{
				currentCol = 0;
				currentRow++;
			}
		}
		child->GetTransform()->SetPosition(newPosition);
	}
	
}

//void PinLayoutGroup::DrawProperty()
//{
//#ifdef USE_IMGUI
//	// エディタでプロパティを描画するための処理をここに実装します。  
//	Component::DrawProperty(); // 基底クラスのプロパティ描画を呼び出す
//
//	if (ImGui::Button("Update Layout"))
//	{
//		UpdateLayout(); // ボタンが押されたときにレイアウトを更新する
//	}
//
//
//#endif // USE_IMGUI
//
//}
