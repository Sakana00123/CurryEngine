#pragma once

#include <functional>
#include <memory>
#include <vector>

enum class Ease : uint8_t
{
	Linear,
	InQuad,
	OutQuad,
	InOutQuad,
	InCubic,
	OutCubic,
	InOutCubic,
	InQuart,
	OutQuart,
	InOutQuart,
	InQuint,
	OutQuint,
	InOutQuint,
	InSine,
	OutSine,
	InOutSine,
	InExp,
	OutExp,
	InOutExp,
	InCirc,
	OutCirc,
	InOutCirc,
	InBounce,
	OutBounce,
	InOutBounce,
	InBack,
	OutBack,
	InOutBack
};

enum class UpdateType : uint8_t
{
	Update,
	FixedUpdate,
	LateUpdate
};

enum class LoopType : uint8_t
{
	Restart,
	Yoyo
};

class ITween
{
public:
	virtual ~ITween() = default;
	void Update(float deltaTime);
	bool IsComplete() const { return m_completed; }
	void Kill();

	ITween& SetUpdate(UpdateType type);
	ITween& OnStart(std::function<void()> func);
	ITween& OnUpdate(std::function<void()> func);
	ITween& OnComplete(std::function<void()> func);
	ITween& SetLoop(int loop, LoopType type);
	ITween& SetDelay(float delay);
protected:

	virtual void OnUpdateInternal(float deltaTime) = 0;

	float m_delay = 0.0f; // 遅延時間
	float m_duration = 0.0f; // 継続時間
	float m_elapsedTime = 0.0f; // 経過時間

	bool m_started = false; // 開始済みフラグ
	bool m_completed = false; // 完了済みフラグ
	bool m_killed = false; // 強制終了済みフラグ

	UpdateType m_updateType = UpdateType::Update; // 更新タイプ
	int m_loopCount = 0; // ループ回数
	LoopType m_loopType = LoopType::Restart; // ループタイプ

	std::function<void()> m_onStart; // 開始時に実行する関数
	std::function<void()> m_onUpdate; // 更新時に実行する関数
	std::function<void()> m_onComplete; // 完了時に実行する関数
};

template<typename T>
class Tween : public ITween
{
public:
	Tween(T* target, T from, T to, float duration);
	~Tween() override = default;

	Tween<T>& SetEase(Ease ease)
	{
		m_ease = ease;
		return *this;
	}

protected:
	void OnUpdateInternal(float deltaTime) override;

private:
	T* m_target; // 対象オブジェクト
	T m_from; // 開始値
	T m_to; // 終了値
	Ease m_ease; // イージングタイプ
};

class Sequence : public ITween
{
public:
	Sequence();
	~Sequence() override = default;
	Sequence& Append(std::shared_ptr<ITween> tween);
	Sequence& Join(std::shared_ptr<ITween> tween);
	Sequence& Prepend(std::shared_ptr<ITween> tween);
private:
	std::vector<std::shared_ptr<ITween>> tweens;
	size_t currentIndex;
	UpdateType updateType;
	bool isCompleted;
	std::function<void()> completeFunction;
};


struct TweenHandle
{
	std::shared_ptr<ITween> tween;
	bool IsComplete() const { return tween->IsComplete(); }
	void Kill() const { tween->Kill(); }
	TweenHandle& SetUpdate(UpdateType type) { tween->SetUpdate(type); return *this; }
	TweenHandle& OnStart(std::function<void()> func) { tween->OnStart(func); return *this; }
	TweenHandle& OnUpdate(std::function<void()> func) { tween->OnUpdate(func); return *this; }
	TweenHandle& OnComplete(std::function<void()> func) { tween->OnComplete(func); return *this; }
	TweenHandle& SetLoop(int loop, LoopType type) { tween->SetLoop(loop, type); return *this; }
	TweenHandle& SetDelay(float delay) { tween->SetDelay(delay); return *this; }
};
