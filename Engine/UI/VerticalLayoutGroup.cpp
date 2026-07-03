#include "pch.h"
#include "VerticalLayoutGroup.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
//REGISTER_COMPONENT(VerticalLayoutGroup, "UserScripts")
REGISTER_COMPONENT_WITH_ATTRIBUTES(VerticalLayoutGroup, "UI", ComponentAttributes::ExecuteInEditMode, {})


void VerticalLayoutGroup::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void VerticalLayoutGroup::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	UpdateLayout();
}

void VerticalLayoutGroup::UpdateLayout()
{
	// 垂直方向のレイアウトを更新する処理をここに実装します。
	// 子要素の RectTransform を取得して、padding や spacing を考慮して配置を計算します。
	auto rectTransform = GetRectTransform();
	if (!rectTransform) return;

	auto childRects = GetChildRects();
	if (childRects.empty()) return;

	RectTransform* selfRect = rectTransform.get();
	const float innerWidth = selfRect->GetWorldSize().x - paddingLeft - paddingRight;
	const float innerHeight = selfRect->GetWorldSize().y - paddingTop - paddingBottom;
	const int childCount = static_cast<int>(childRects.size());
	const float totalSpacing = spacing * (childCount - 1);

	// 子要素の高さを計算
	std::vector<float> childHeights(childCount);
	float fixedTotal = 0.0f;
	for (int i = 0; i < childCount; ++i)
	{
		float preferredHeight = childRects[i]->size.y;
		childHeights[i] = preferredHeight;
		fixedTotal += preferredHeight;
	}
	// ForceExpand: 余白を均等に分配
	if (childForceExpandHeight)
	{
		float extraSpace = (std::max)(0.0f, innerHeight - fixedTotal - totalSpacing);
		float extraPerChild = extraSpace / childCount;
		for (int i = 0; i < childCount; ++i)
		{
			childHeights[i] += extraPerChild;
		}
	}

	// 主軸の開始位置を計算 (上揃え、中央揃え、下揃え)
	float totalUsed = totalSpacing;
	for (float h : childHeights) totalUsed += h;

	float startY = paddingTop;

	if (layoutAlignment == 1) // 中央揃え
	{
		startY = paddingTop + (innerHeight - totalUsed) * 0.5f;
	}
	else if (layoutAlignment == 2) // 下揃え
	{
		startY = paddingTop + (innerHeight - totalUsed);
	}

	// 子要素を配置
	float cursor = startY;
	for (int i = 0; i < childCount; ++i)
	{
		auto* rect = childRects[i].get();
		float childWidth = childControlWidth ? innerWidth : rect->size.x;
		float childHeight = childControlHeight ? childHeights[i] : rect->size.y;

		// 交差軸の配置
		float localX = paddingLeft;
		if (layoutAlignment == 1) // 中央揃え
		{
			localX = paddingLeft + (innerWidth - childWidth) * 0.5f;
		}
		else if (layoutAlignment == 2) // 右揃え
		{
			localX = paddingLeft + (innerWidth - childWidth);
		}
		
		// 子要素の位置を設定
		rect->SetAnchorMin({ 0, 0 });
		rect->SetAnchorMax({ 0, 0 });
		rect->SetPivot({ 0, 0 }); // 左上を基準に配置
		rect->SetAnchoredPosition({ localX, cursor });
		rect->SetSize({ childWidth, childHeight });

		cursor += childHeight + spacing;
	}
}