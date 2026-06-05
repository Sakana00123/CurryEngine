using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurryEngine
{
    /// <summary>
    /// ゲームの時間に関する情報を提供するクラス。
    /// </summary>
    public static class Time
    {
        /// <summary>
        /// 前のフレームからの経過時間を秒単位で返します。ゲームの時間スケールの影響を受けます。
        /// </summary>
        public static float deltaTime => Interop.NativeMethods.Time_GetDeltaTime();


        //public static float time { get; internal set; }

        /// <summary>
        /// 前のフレームからの経過時間を秒単位で返します。ゲームの時間スケールの影響を受けません。
        /// </summary>
        public static float unscaledDeltaTime => Interop.NativeMethods.Time_GetUnscaledDeltaTime();


        //public static float fixedDeltaTime { get; internal set; }

        /// <summary>
        /// ゲームの時間スケールを取得または設定します。時間スケールは、ゲームの時間の進み方を制御するために使用されます。例えば、時間スケールを0に設定すると、ゲームの時間が停止し、すべての動きやアニメーションが停止します。逆に、時間スケールを2に設定すると、ゲームの時間が通常の2倍の速さで進みます。
        /// </summary>
        public static float timeScale
        {
            get => Interop.NativeMethods.Time_GetTimeScale();
            set => Interop.NativeMethods.Time_SetTimeScale(value);
        }
    }
}
