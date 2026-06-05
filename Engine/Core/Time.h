#pragma once

/**
 * @file
 * @brief 高精度タイマーによる時間管理ユーティリティ。
 * @details QueryPerformanceCounter/Frequency を用いて、
 *          経過時間、デルタタイム、スケール済み時間の取得を提供します。
 */
class Time 
{
public:
	/**
	 * @brief コンストラクタ。高精度タイマーを初期化します。
	 * @details 周波数から 1 カウントあたりの秒数を求め、基準時刻を記録します。
	 */
	Time();
	/** @brief デストラクタ。*/
	~Time() = default;
	Time(const Time&) = delete;
	Time& operator=(const Time&) = delete;
	Time(Time&&) noexcept = delete;
	Time& operator=(Time&&) noexcept = delete;

	/**
	 * @brief タイマーをリセットします。
	 * @details 現在時刻を基準に取り直し、停止状態を解除します。
	 */
	void Reset();

	/**
	 * @brief 停止中のタイマーを再開します。
	 * @details 停止中に経過した時間を `pausedTime` に加算します。
	 */
	void Start();

	/**
	 * @brief タイマーを停止します。
	 */
	void Stop();

	/**
	 * @brief 1フレーム分の時間を更新します。
	 * @details `deltaTime` と `unscaledDeltaTime` を計算し、負値の場合は 0 に丸めます。
	 */
	void Tick();

	/**
	 * @brief アプリケーション開始からの経過時間（秒）を取得します。
	 * @return 経過時間（秒）。
	 */
	float TimeStamp() const;

public:
	/** @brief スケール適用後のデルタタイム（秒）。*/
	static float DeltaTime() { return static_cast<float>(deltaTime); }
	/** @brief スケール非適用のデルタタイム（秒）。*/
	static float UnscaledDeltaTime() { return static_cast<float>(unscaledDeltaTime); }
	/**
	 * @brief 時間スケール。
	 * @details 1.0 が等速、2.0 が 2 倍速、0.5 が半速、0.0 で停止。
	 */
	static inline float timeScale{ 1.0f };
private:
	/** @brief 前フレームからの経過時間（秒、スケール適用）。*/
	static inline double deltaTime{ 0.0f };
	/** @brief 前フレームからの経過時間（秒、スケール非適用）。*/
	static inline double unscaledDeltaTime{ 0.0f };
private:
	/** @brief 1カウントあたりの秒数。*/
	double secondsPerCount{ 0.0 };

	LONGLONG baseTime{ 0LL };   //!< 基準時刻
	LONGLONG pausedTime{ 0LL }; //!< 停止中の累積時間
	LONGLONG stopTime{ 0LL };   //!< 停止した時刻
	LONGLONG lastTime{ 0LL };   //!< 前フレームの時刻
	LONGLONG thisTime{ 0LL };   //!< 現在の時刻

	bool stopped{ false };      //!< 停止中フラグ
};