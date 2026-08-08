using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurryEngine
{
    /// <summary>
    /// ゲームの時間に関する情報を格納する構造体。フレームごとの時間の変化や、ゲームの時間スケールを保持します。
    /// </summary>
    internal readonly struct TimeData
    {
        public readonly float DeltaTime;
        public readonly float UnscaledDeltaTime;
        public readonly float TimeScale;
        public TimeData(float deltaTime, float unscaledDeltaTime, float timeScale)
        {
            DeltaTime = deltaTime;
            UnscaledDeltaTime = unscaledDeltaTime;
            TimeScale = timeScale;
        }
    }

    /// <summary>
    /// ゲームの時間に関する情報を提供するクラス。
    /// </summary>
    public static class Time
    {
        internal static Action<float>? OnTimeScaleChanged { get; set; }

        internal static TimeData CurrentTimeData { get; private set; }

        internal static void UpdateTimeData(TimeData timeData)
        {
            CurrentTimeData = timeData;
        }


        /// <summary>
        /// 前のフレームからの経過時間を秒単位で返します。ゲームの時間スケールの影響を受けます。
        /// </summary>
        public static float DeltaTime => CurrentTimeData.DeltaTime;


        /// <summary>
        /// 前のフレームからの経過時間を秒単位で返します。ゲームの時間スケールの影響を受けません。
        /// </summary>
        public static float UnscaledDeltaTime => CurrentTimeData.UnscaledDeltaTime;


        //public static float fixedDeltaTime { get; internal set; }

        /// <summary>
        /// ゲームの時間スケールを取得または設定します。時間スケールは、ゲームの時間の進み方を制御するために使用されます。例えば、時間スケールを0に設定すると、ゲームの時間が停止し、すべての動きやアニメーションが停止します。逆に、時間スケールを2に設定すると、ゲームの時間が通常の2倍の速さで進みます。
        /// </summary>
        public static float TimeScale 
        {
            get => CurrentTimeData.TimeScale;
            set
            {
                if (value < 0)
                    throw new ArgumentOutOfRangeException(nameof(value), "TimeScale cannot be negative.");
                if (CurrentTimeData.TimeScale != value)
                {
                    OnTimeScaleChanged?.Invoke(value);
                    CurrentTimeData = new TimeData(CurrentTimeData.DeltaTime, CurrentTimeData.UnscaledDeltaTime, value);
                }
            }
        }
    }
}
