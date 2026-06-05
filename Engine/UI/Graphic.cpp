#include "pch.h"
#include "Graphic.h"
#include "Canvas.h"
#include "Mask.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(Graphic, "UI", ComponentAttributes::HideInAddComponentMenu, {})


void Graphic::OnEnable()
{
	if (GetOwner() == nullptr) return; // 所属 GameObject が無効な場合は処理しない

	if (Canvas* canvas = GetOwner()->GetComponentInParent<Canvas>()) {
		canvas->RegisterGraphic(this);
	}
}

void Graphic::OnDisable()
{
	if (GetOwner() == nullptr) return; // 所属 GameObject が無効な場合は処理しない

	if (Canvas* canvas = GetOwner()->GetComponentInParent<Canvas>()) {
		canvas->UnregisterGraphic(this);
	}
}

Canvas* Graphic::GetCanvas() const
{
	if (GetOwner() == nullptr) return nullptr; // 所属 GameObject が無効な場合は nullptr を返す
	return GetOwner()->GetComponentInParent<Canvas>();
}

bool Graphic::Raycast(const Vector2& position)
{
	if (isRaycastTarget) {
		if (!GetRectTransform()) return false; // RectTransform がない場合は当たり判定できない
		if (!GetRectTransform()->Contains(position))
			return false; // 矩形内にない場合は当たり判定なし

		// 祖先のMaskでクリッピング
		auto* parent = GetOwner()->GetParent();

		while (parent) {
			if (Mask* mask = parent->GetComponent<Mask>()) {
				D3D11_RECT scissorRect = mask->GetScissorRect();
				if (position.x < scissorRect.left || position.x > scissorRect.right ||
					position.y < scissorRect.top || position.y > scissorRect.bottom) {
					return false; // マスクの矩形外にある場合は当たり判定なし
				}
			}
			parent = parent->GetParent();
		}

		return true; // 当たり判定あり
	}
	return false;
}