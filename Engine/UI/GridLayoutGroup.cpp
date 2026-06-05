#include "pch.h"
#include "GridLayoutGroup.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
//REGISTER_COMPONENT(GridLayoutGroup, "UserScripts")
REGISTER_COMPONENT_WITH_ATTRIBUTES(GridLayoutGroup, "UI", ComponentAttributes::ExecuteInEditMode, {})


void GridLayoutGroup::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void GridLayoutGroup::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	UpdateLayout();
}

void GridLayoutGroup::UpdateLayout()
{
	// 子要素の位置を計算して配置する処理をここに実装します。
	// 例えば、子要素をグリッド状に配置する場合の例を以下に示します。
	auto rectTransform = GetRectTransform();
	if (!rectTransform) return;
	auto childRects = GetChildRects();
	if (childRects.empty()) return;

	// セルのサイズとスペーシングを考慮して、子要素の位置を計算します。
	int columns = static_cast<int>((rectTransform->GetWorldSize().x - paddingLeft - paddingRight + spacing) / (cellSize.x + spacing));
	int row = 0, column = 0;
	
	// 子要素をグリッド状に配置(左上から右下へ)します。
	for (size_t i = 0; i < childRects.size(); ++i)
	{
		auto& childRect = childRects[i];
		float x = paddingLeft + column * (cellSize.x + spacing);
		float y = paddingTop + row * (cellSize.y + spacing);
		childRect->SetAnchoredPosition(Vector2(x, y));
		childRect->SetSize(cellSize);
		column++;
		if (column >= columns)
		{
			column = 0;
			row++;
		}
	}
}