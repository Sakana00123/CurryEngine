#include "pch.h"
#include "HorizontalLayoutGroup.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
//REGISTER_COMPONENT(HorizontalLayoutGroup, "UserScripts")
REGISTER_COMPONENT_WITH_ATTRIBUTES(HorizontalLayoutGroup, "UI", ComponentAttributes::ExecuteInEditMode, {})


void HorizontalLayoutGroup::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}



void HorizontalLayoutGroup::UpdateLayout()
{
	// 水平方向のレイアウトを更新する処理をここに実装します。
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

	// 子要素の幅を計算
	std::vector<float> childWidths(childCount);
	float fixedTotal = 0.0f;
	for (int i = 0; i < childCount; ++i)
	{
		float preferredWidth = childRects[i]->size.x;
		childWidths[i] = preferredWidth;
		fixedTotal += preferredWidth;
	}

	// ForceExpand: 余白を均等に分配
	if (childForceExpandWidth)
	{
		float extraSpace = (std::max)(0.0f, innerWidth - fixedTotal - totalSpacing);
		float extraPerChild = extraSpace / childCount;
		for (int i = 0; i < childCount; ++i)
		{
			childWidths[i] += extraPerChild;
		}
	}

	// 主軸の開始位置を計算 (左揃え、中央揃え、右揃え)
	float totalUsed = totalSpacing;
	for (float w : childWidths) totalUsed += w;

	float startX = paddingLeft;
	if (layoutAlignment == 1) // 中央揃え
	{
		startX = paddingLeft + (innerWidth - totalUsed) * 0.5f;
	}
	else if (layoutAlignment == 2) // 右揃え
	{
		startX = selfRect->GetWorldSize().x - paddingRight - totalUsed;
	}

	// 子要素を配置
	float cursor = startX;
	for (int i = 0; i < childCount; ++i)
	{
		auto* rect = childRects[i].get();
		float childWidth = childControlWidth ? childWidths[i] : rect->size.x;
		float childHeight = childControlHeight ? innerHeight : rect->size.y;

		// 交差軸(Y)のalignment
		float localY = paddingTop;
		if (layoutAlignment == 1) // 中央揃え
		{
			localY = paddingTop + (innerHeight - childHeight) * 0.5f;
		}
		else if (layoutAlignment == 2) // 右揃え
		{
			localY = selfRect->GetWorldSize().y - paddingBottom - childHeight;
		}

		// 子要素のアンカーポジションを設定
		rect->SetAnchorMin({ 0, 0 });
		rect->SetAnchorMax({ 0, 0 });
		rect->SetPivot({ 0, 0 }); // 左上を基準に配置
		rect->SetAnchoredPosition({ cursor, localY });
		rect->SetSize({ childWidth, childHeight });

		cursor += childWidth + spacing;
	}
}