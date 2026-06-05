#pragma once
#include "easing.h"
#include <functional>
#include <vector>

/**
 * @file
 * @brief イージングのシーケンスを管理し、値を時間的に補間するハンドラ。
 * @details 直列のイージング/待機を組み立て、`Update` により進行させます。
 *          進捗値や補間済み進捗、完了時コールバックの管理も行います。
 */

/**
 * @brief イージング種別。
 */
enum class EaseType
{
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
	InOutBack,
	Linear
};

/**
 * @brief イージングを制御するハンドラクラス。
 */
class EasingHandler
{
public:
	/**
	 * @brief 1 ステップ分のイージング要素。
	 * @details `easeData` に時刻や開始/終了値等を保持し、`function` または
	 *          `backFunction` を使って補間値を算出します。
	 */
	struct EaseItem
	{
		EaseData easeData{}; //!< 時刻・開始/終了値・バック値など
		std::function<float(float, float, float, float)> function; //!< e(t, b, c, d)
		std::function<float(float, float, float, float, float)> backFunction; //!< e(t, b, c, d, back)
	};

	/** @brief 既定コンストラクタ。*/
	EasingHandler() {}
	/** @brief デストラクタ。*/
	~EasingHandler() {}

	/**
	 * @brief イージング要素を追加します。
	 * @param type イージングタイプ。
	 * @param start 開始値。
	 * @param end 終了値。
	 * @param duration 補間時間（秒）。
	 * @param back バック系で用いる係数（未使用タイプでは無視）。
	 */
	void AddEasing(EaseType type, float start, float end, float duration = 1.0f, float back = 1.70158f);
	/** @brief 既製のイージング要素を追加します。*/
	void AddEasing(const EaseItem& item);

	/**
	 * @brief 待機（ディレイ）を追加します。
	 * @param waitTime 待機時間（秒）。
	 */
	void AddWait(float waitTime);

	/** @brief すべての要素をクリアします。*/
	void Clear();

	/**
	 * @brief 値を更新（補間）します。
	 * @param value イージング対象の値（参照）。
	 * @param deltaTime 経過時間（秒）。
	 */
	void Update(float& value, float deltaTime);

	/** @brief 現在の進捗（0～1）。*/
	float GetProgress() const { return progress; }

	/** @brief イージング済みの進捗（0～1）。*/
	float GetEasedProgress() const { return easedProgress; }

	/**
	 * @brief 線形補間を行います。
	 * @tparam T 任意型（演算子*と+が必要）。
	 * @param a 開始値。
	 * @param b 終了値。
	 * @param t 進捗（0～1）。
	 * @return 補間値。
	 */
	template <class T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a * (1.0f - t) + b * t;
	}
	/**
	 * @brief 任意型の補間値を返します。
	 * @tparam T 任意型。
	 * @param start 開始値。
	 * @param end 終了値。
	 * @return `easedProgress` に基づく補間結果。
	 */
	template<typename T>
	T GetValue(const T& start, const T& end) const {
		return Lerp(start, end, easedProgress);
	}

	/** @brief すべての処理が完了したか。*/
	bool IsCompleted() const { return isCompleted; }

	/**
	 * @brief 完了時に実行する関数を設定します。
	 * @param function 引数なしコールバック関数。
	 */
	void SetCompletedFunction(std::function<void()> function) { completeFunction = function; }

	/** @brief 現在のシーケンス長を返します。*/
	size_t GetSequenceCount() const { return sequence.size(); }

	/**
	 * @brief 列挙型をイージング関数に変換します。
	 * @param type イージングタイプ。
	 * @param func back 非対応関数の出力先。
	 * @param backFunc back 対応関数の出力先。
	 */
	static void ToEasingFunction(EaseType type, std::function<float(float, float, float, float)>& func, std::function<float(float, float, float, float, float)>& backFunc);

private:
	/** @brief 完了時コールバックがあれば実行します。*/
	void ExecuteCompletedFunction() { if (completeFunction != nullptr) completeFunction(); }

private:
	/** @brief イージング要素のシーケンス。*/
	std::vector<EaseItem> sequence;
	/** @brief 完了フラグ。*/
	bool isCompleted = false;
	/** @brief 現在の進捗（0～1）。*/
	float progress = 0.0f;
	/** @brief イージング後の進捗（0～1）。*/
	float easedProgress = 0.0f;
	/** @brief 完了時コールバック。*/
	std::function<void()> completeFunction;
};
