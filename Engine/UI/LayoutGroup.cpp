#include "pch.h"
#include "LayoutGroup.h"
#include "Engine/Core/GameObject.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
//REGISTER_COMPONENT(LayoutGroup, "UserScripts")
REGISTER_COMPONENT_WITH_ATTRIBUTES(LayoutGroup, "UI", ComponentAttributes::ExecuteInEditMode | ComponentAttributes::HideInAddComponentMenu, {})


void LayoutGroup::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void LayoutGroup::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	// TODO: レイアウトの更新が必要な場合にのみ `UpdateLayout()` を呼び出すように、`m_layoutDirty` フラグを使用して最適化することができます。
	//if (m_layoutDirty)
	{
		UpdateLayout(); // レイアウトを更新
		m_layoutDirty = false; // 更新後はフラグをリセット
	}
}

#ifdef USE_IMGUI
void LayoutGroup::DrawProperty(const PropertyDrawContext& context)
{
	// エディタでプロパティを描画するための処理をここに実装します。
	Component::DrawProperty(context); // 基底クラスのプロパティ描画を呼び出す

	IMGUI_PROPERTY_BEGIN();

	// alignment のプロパティを描画
	const char* alignmentOptions[] = { "Left", "Center", "Right" };
	int oldAlignment = layoutAlignment;
	IMGUI_PROPERTY_ENUM("layoutAlignment", layoutAlignment, alignmentOptions, _countof(alignmentOptions));
	if (oldAlignment != layoutAlignment)
	{
		SetLayoutDirty(); // 配置方法が変更されたらレイアウトを更新する必要があることを示す
	}

	IMGUI_PROPERTY_END();

}
#endif // USE_IMGUI

std::vector<std::shared_ptr<RectTransform>> LayoutGroup::GetChildRects() const
{
	std::vector<std::shared_ptr<RectTransform>> childRects;
	if (auto owner = GetOwner())
	{
		for (const auto& child : owner->GetChildren())
		{
			if (!child->IsActive())
			{
				continue; // 非アクティブな子はスキップ
			}
			if (auto rect = child->GetComponentShared<RectTransform>())
			{
				childRects.push_back(rect);
			}
		}
	}
	return childRects;
}

std::shared_ptr<RectTransform> LayoutGroup::GetRectTransform() const
{
	if (auto owner = GetOwner())
	{
		return owner->GetComponentShared<RectTransform>();
	}
	return nullptr;
}