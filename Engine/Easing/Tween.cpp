#include "pch.h"
#include "Tween.h"
#include "Easing.h"
#include "Engine/Core/Transform.h"


void ITween::Update(float deltaTime)
{
	if (m_killed || m_completed)
		return;
	// 遅延時間の処理
	if (m_delay > 0.0f)
	{
		m_delay -= deltaTime;
		if (m_delay > 0.0f)
			return;
		deltaTime = -m_delay; // 遅延が終わった分の時間を更新に使う
		m_delay = 0.0f;
	}
	// 開始時のコールバック
	if (!m_started)
	{
		m_started = true;
		if (m_onStart)
			m_onStart();
	}
	// 経過時間の更新
	m_elapsedTime += deltaTime;
	// 内部更新処理
	OnUpdateInternal(deltaTime);
	// 更新時のコールバック
	if (m_onUpdate)
		m_onUpdate();
	// 完了判定
	if (m_elapsedTime >= m_duration)
	{
		m_completed = true;
		if (m_onComplete)
			m_onComplete();
	}
}
void ITween::Kill()
{
	m_killed = true;
}
ITween& ITween::SetUpdate(UpdateType type)
{
	m_updateType = type;
	return *this;
}
ITween& ITween::OnStart(std::function<void()> func)
{
	m_onStart = func;
	return *this;
}
ITween& ITween::OnUpdate(std::function<void()> func)
{
	m_onUpdate = func;
	return *this;
}
ITween& ITween::OnComplete(std::function<void()> func)
{
	m_onComplete = func;
	return *this;
}
ITween& ITween::SetLoop(int loop, LoopType type)
{
	m_loopCount = loop;
	m_loopType = type;
	return *this;
}
ITween& ITween::SetDelay(float delay)
{
	m_delay = delay;
	return *this;
}


template <typename T>
Tween<T>::Tween(T* target, T from, T to, float duration)
{
	m_target = target;
	m_from = from;
	m_to = to;
	m_duration = duration;
	m_ease = Ease::Linear;
}

template <typename T>
void Tween<T>::OnUpdateInternal(float deltaTime)
{
	float t = m_elapsedTime / m_duration;
	if (t > 1.0f) t = 1.0f;
	// イージング計算
	
	//float easedT = Easing::Ease(t, m_ease);
	float easedT = t;
	// 補間値の計算
	T value = m_from + (m_to - m_from) * easedT;
	// 対象オブジェクトに値を設定
	*m_target = value;
}

// 明示的なインスタンス化
template class Tween<float>;
template class Tween<int>;
template class Tween<Vector2>;
template class Tween<Vector3>;