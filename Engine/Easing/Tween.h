#pragma once

#include <functional>
#include <memory>
#include <vector>

/** @brief Ease を表す列挙型です。 */
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

/** @brief UpdateType を表す列挙型です。 */
enum class UpdateType : uint8_t
{
	Update,
	FixedUpdate,
	LateUpdate
};

/** @brief LoopType を表す列挙型です。 */
enum class LoopType : uint8_t
{
	Restart,
	Yoyo
};

/** @brief ITween を表すクラスです。 */
class ITween
{
public:
	/**
	 * @brief ITween を破棄します。
	 */
	virtual ~ITween() = default;
	/**
	 * @brief 状態を更新します。
	 */
	void Update(float deltaTime);
	bool IsComplete() const { return m_completed; }
	/**
	 * @brief Kill の処理を行います。
	 */
	void Kill();

	/**
	 * @brief Update を設定します。
	 */
	ITween& SetUpdate(UpdateType type);
	/**
	 * @brief Start イベントを処理します。
	 */
	ITween& OnStart(std::function<void()> func);
	/**
	 * @brief Update イベントを処理します。
	 */
	ITween& OnUpdate(std::function<void()> func);
	/**
	 * @brief Complete イベントを処理します。
	 */
	ITween& OnComplete(std::function<void()> func);
	/**
	 * @brief Loop を設定します。
	 */
	ITween& SetLoop(int loop, LoopType type);
	/**
	 * @brief Delay を設定します。
	 */
	ITween& SetDelay(float delay);
protected:

	/**
	 * @brief UpdateInternal イベントを処理します。
	 */
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
/** @brief Tween を表すクラスです。 */
class Tween : public ITween
{
public:
	/**
	 * @brief Tween を構築します。
	 */
	Tween(T* target, T from, T to, float duration);
	/**
	 * @brief Tween を破棄します。
	 */
	~Tween() override = default;

	Tween<T>& SetEase(Ease ease)
	{
		m_ease = ease;
		return *this;
	}

protected:
	/**
	 * @brief UpdateInternal イベントを処理します。
	 */
	void OnUpdateInternal(float deltaTime) override;

private:
	T* m_target; // 対象オブジェクト
	T m_from; // 開始値
	T m_to; // 終了値
	Ease m_ease; // イージングタイプ
};

/** @brief Sequence を表すクラスです。 */
class Sequence : public ITween
{
public:
	/**
	 * @brief Sequence を構築します。
	 */
	Sequence();
	/**
	 * @brief Sequence を破棄します。
	 */
	~Sequence() override = default;
	/**
	 * @brief Append の処理を行います。
	 */
	Sequence& Append(std::shared_ptr<ITween> tween);
	/**
	 * @brief Join の処理を行います。
	 */
	Sequence& Join(std::shared_ptr<ITween> tween);
	/**
	 * @brief Prepend の処理を行います。
	 */
	Sequence& Prepend(std::shared_ptr<ITween> tween);
private:
	std::vector<std::shared_ptr<ITween>> tweens;
	size_t currentIndex;
	UpdateType updateType;
	bool isCompleted;
	std::function<void()> completeFunction;
};


/** @brief TweenHandle を表す構造体です。 */
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
