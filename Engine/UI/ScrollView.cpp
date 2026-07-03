#include "pch.h"
#include "ScrollView.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(ScrollView, "UI", ComponentAttributes::ExecuteInEditMode, {});

void ScrollView::Awake()
{
	UIComponent::Awake();
}

void ScrollView::Update(float deltaTime)
{
	UIComponent::Update(deltaTime);

	if (!isInputEnabled)
		return;

	if (!contentRef.IsValid())
		return;

	GameObject* contentObj = GetScene()->GetObjectManager()->FindInObjects(contentRef);
	if (!contentObj)
		return;

	auto contentRect = contentObj->GetComponent<RectTransform>();
	if (!contentRect)
		return;

	// ここまでの処理で contentRect が有効な状態であれば、スクロール処理を行います。
	// マウスホイールの入力を取得
	float scrollDelta = InputSystem::GetWheelDelta();
	// スクロールオフセットを更新
	m_scrollPosition.y += (-scrollDelta) * scrollSensitivity;
	// content の位置にスクロールオフセットを反映させる処理をここに実装します。
	// 例えば、content の RectTransform を取得して、アンカーやピボットに基づいて位置を調整することが考えられます。

	// スクロール位置の制限（例: 上限を 0、下限を content の高さ - viewport の高さ とする場合）
	float contentHeight = contentRect->GetWorldSize().y; // content の高さ
	float viewportHeight = GetRectTransform()->GetWorldSize().y; // ビューポートの高さ
	// スクロール位置が下限を超えないようにする（例: contentHeight - viewportHeight 以下）
	float maxScrollY = (std::max)(0.f, contentHeight - viewportHeight);

	m_scrollPosition.y = std::clamp(m_scrollPosition.y, -maxScrollY, 0.0f); // スクロール位置が上限を超えないようにする（例: 0 以上）

	// Content に反映
	contentRect->SetAnchoredPosition(m_scrollPosition);
}