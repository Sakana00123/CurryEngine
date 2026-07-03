#pragma once

enum BeatResult : uint8_t
{
	Perfect,
	Good,
	Miss
};


class BeatManager
{
public:

	/**
	 * @brief ビートマネージャを初期化します。
	 */
	static void Initialize();

	/**
	 * @brief 現在のビートタイミングかを返します。
	 */
	static bool IsBeatTiming();

	/**
	 * @brief ちょうどビートを刻んだ瞬間かを返します。
	 */
	static bool IsJustBeat(float offset = -0.05f)
	{
		// ビートカウントの更新
		int previousBeatCount = static_cast<int>((previousSongTime + offset) / beatInterval);
		int beatCount = static_cast<int>((songTime + offset) / beatInterval);
		bool isBeatTiming = false;

		// 前回のビートカウントと比較して、ビートが変わったかをチェック
		if (beatCount != previousBeatCount)
		{
			// ビートが変わったときの処理
			isBeatTiming = true;
		}
		return isBeatTiming;
	}

	/**
	 * @brief 現在のビート内の時間を返します。
	 * @param offset オフセット時間（秒）。デフォルトは -0.05 秒。
	 * @return 現在のビート内の時間（秒）。
	 */
	static float GetTimeInCurrentBeat(float offset = -0.05f)
	{
		return fmod(songTime + offset, beatInterval);
	}


	/**
	 * @brief ビートタイミングの評価を返します。
	 * @param offset 評価のオフセット時間（秒）。デフォルトは -0.05 秒。
	 * @return ビートタイミングの評価結果。
	 */
	static BeatResult CheckBeatTiming(float offset = -0.05f) 
	{
		constexpr float perfectThreshold = 0.1f;
		constexpr float goodThreshold = 0.2f;
		float timeInCurrentBeat = GetTimeInCurrentBeat(offset);
		if (timeInCurrentBeat <= perfectThreshold || timeInCurrentBeat >= (beatInterval - perfectThreshold)) {
			return BeatResult::Perfect;
		}
		else if (timeInCurrentBeat <= goodThreshold || timeInCurrentBeat >= (beatInterval - goodThreshold)) {
			return BeatResult::Good;
		}
		else {
			return BeatResult::Miss;
		}
	}

	/**
	 * @brief ビートタイミングを更新します。
	 * @param deltaTime 前フレームからの経過時間（秒）。
	 */
	static void Update(float deltaTime);
private:
	/** @brief 前回のビートカウント。*/
	static inline int previousBeatCount = -1;
	/** @brief ビートカウント。*/
	static inline int beatCount = 0;
	/** @brief 現在ビートタイミングか。*/
	static inline bool isBeatTiming = false;
	/** @brief BPM（Beats Per Minute）。*/
	static inline float bpm = 128.0f;
	/** @brief ビート間隔（秒）。*/
	static inline float beatInterval = 60.0f / bpm;
	/** @brief 前回の曲の再生時間（秒）。*/
	static inline float previousSongTime = 0.0f;
	/** @brief 曲の再生時間（秒）。*/
	static inline float songTime = 0.0f;
	/** @brief ビートタイミングの許容誤差（秒）。*/
	static inline float beatTolerance = 0.1f;
};
