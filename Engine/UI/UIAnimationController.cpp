#include "pch.h"
#include "UIAnimationController.h"

void UIAnimationController::SetTransition(State state, const Transition& transition)
{
	transitions[static_cast<size_t>(state)] = transition;
}


void UIAnimationController::Update(float elapsedTime)
{
	RectTransform* rect = GetRectTransform();
	if (Selectable* selectable = gameObject->GetComponent<Selectable>()) {
		//ステートの更新
		if (!selectable->IsInteractable()) {
			SetState(State::Disabled);
		}
		else if (selectable->isPressed) {
			SetState(State::Pressed);
		}
		else if (selectable->isHovered) {
			SetState(State::Hovered);
		}
		else if (selectable->isSelected) {
			SetState(State::Selected);
		}
		else {
			SetState(State::Idle);
		}


		if (transitionProgress >= 1.0f || handler.GetSequenceCount() == 0) return;

		//ハンドラ更新
		handler.Update(transitionProgress, elapsedTime);

		//座標の更新
		if (transitions[static_cast<size_t>(currentState)].enableTranslate)
		{
			XMVECTOR FromPosition = XMLoadFloat2(&fromPosition);
			XMVECTOR ToPosition = XMLoadFloat2(&toPosition);
			Vector2 pos;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&pos), XMVectorLerp(FromPosition, ToPosition, transitionProgress));
			rect->SetAnchoredPosition(pos);
		}

		//サイズの更新
		if (transitions[static_cast<size_t>(currentState)].enableSizing)
		{
			XMVECTOR FromSize = XMLoadFloat2(&fromSize);
			XMVECTOR ToSize = XMLoadFloat2(&toSize);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&rect->size), XMVectorLerp(FromSize, ToSize, transitionProgress));
		}
	}
}

void UIAnimationController::SetState(State newState)
{
	if (newState == targetState) return;
	RectTransform* rect = GetRectTransform();
	const auto& newTrans = transitions[static_cast<size_t>(newState)];
	const auto& oldTrans = transitions[static_cast<size_t>(currentState)];

	fromPosition = rect->GetAnchoredPosition();
	toPosition = newTrans.position;

	fromSize = rect->size;
	toSize = newTrans.size;

	transitionProgress = 0.0f;

	handler.Clear();
	handler.AddEasing(newTrans.type, 0.0f, 1.0f, newTrans.duration);

	currentState = targetState;
	targetState = newState;
}