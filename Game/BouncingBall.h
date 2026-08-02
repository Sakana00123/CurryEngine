#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/UI/UIComponent.h"

class BouncingBall : public UIComponent
{
	C_REFLECT(BouncingBall)

public:
	BouncingBall() = default;
	~BouncingBall() = default;

	void Start() override;
	void Update(float deltaTime) override;

private:
	float m_timer = 0.0f;

	// --- 初期位置（右端の待機場所） ---
	float m_startX = 0.0f;
	float m_groundY = 0.0f;

	// --- モーションの設定 ---
	float m_jumpDistanceX = 150.0f; // ジャンプで左に進む距離
	float m_jumpHeight = 100.0f;    // ジャンプの高さ
	float m_speedFactor = 1.5f;     // ループ全体の速度（大きいほどキビキビ動く）
};