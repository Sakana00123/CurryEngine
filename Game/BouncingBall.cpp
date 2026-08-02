#include "pch.h"
#include "BouncingBall.h"
#include <cmath>

REGISTER_COMPONENT(BouncingBall, "UserScripts")

void BouncingBall::Start()
{
	m_timer = 0.0f;

	if (RectTransform* rect = GetRectTransform())
	{
		m_startX = rect->anchoredPosition.x;
		m_groundY = rect->anchoredPosition.y;
	}
}

void BouncingBall::Update(float deltaTime)
{
	// 時間を進行させる
	m_timer += deltaTime * m_speedFactor;

	// タイマーが 2.0 に達したら 0.0 に戻してループ（往路1.0 ＋ 復路1.0 ＝ 計2.0）
	if (m_timer >= 2.0f)
	{
		m_timer = 0.0f;
	}

	RectTransform* rect = GetRectTransform();
	if (rect)
	{
		float newX = m_startX;
		float newY = m_groundY;

		// ---------- 【前半：0.0 〜 1.0】 右から左へジャンプ ----------
		if (m_timer < 1.0f)
		{
			float progress = m_timer; // 0.0 〜 1.0

			// X座標：右端から左へ進む
			newX = m_startX - (progress * m_jumpDistanceX);

			// Y座標：サイン波で1回ジャンプする（0 〜 π の空気感を作るため M_PI を掛ける）
			const float M_PI_F = 3.141592f;
			float jumpCurve = std::sin(progress * M_PI_F);
			newY = m_groundY - (jumpCurve * m_jumpHeight);
		}
		// ---------- 【後半：1.0 〜 2.0】 左から右へ滑らかに戻る ----------
		else
		{
			// 後半の進行度を 0.0 〜 1.0 に変換
			float progress = m_timer - 1.0f;

			// X座標：左端（m_startX - m_jumpDistanceX）から、初期位置（m_startX）まで戻る
			// progressが1.0に近づくほど、引き算される値が減るので右に戻っていきます
			newX = (m_startX - m_jumpDistanceX) + (progress * m_jumpDistanceX);

			// Y座標：地面を滑るため、初期位置（地面）のまま固定
			newY = m_groundY;
		}

		// 計算した座標を適用
		rect->SetAnchoredPosition({ newX, newY });
		//rect->SetRotation(m_timer * 360.0f); // 回転はなし
		rect->SetAngle(-m_timer * 360.0f); // 回転はなし
	}
}