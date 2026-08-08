using System;

namespace CurryEngine.Math
{
    public static class Mathf
    {
        // ---- 定数 ----
        public const float PI       = MathF.PI;
        public const float Deg2Rad  = PI / 180f;
        public const float Rad2Deg  = 180f / PI;
        public const float Epsilon   = float.Epsilon;
        public const float Infinity  = float.PositiveInfinity;

        // ---- 基本演算 ----
        public static float Abs(float f) => MathF.Abs(f);
        public static float Sign(float f) => MathF.Sign(f);
        public static float Floor(float f) => MathF.Floor(f);
        public static float Ceil(float f) => MathF.Ceiling(f);
        public static float Round(float f) => MathF.Round(f);
        public static float Sqrt(float f) => MathF.Sqrt(f);
        public static float Pow(float f, float p) => MathF.Pow(f, p);
        public static float Log(float f) => MathF.Log(f);
        public static float Log(float f, float b) => MathF.Log(f, b);
        public static float Exp(float f) => MathF.Exp(f);

        // ---- 三角関数 ----
        public static float Sin(float f) => MathF.Sin(f);
        public static float Cos(float f) => MathF.Cos(f);
        public static float Tan(float f) => MathF.Tan(f);
        public static float Asin(float f) => MathF.Asin(f);
        public static float Acos(float f) => MathF.Acos(f);
        public static float Atan(float f) => MathF.Atan(f);
        public static float Atan2(float y, float x) => MathF.Atan2(y, x);

        // ---- 補間 ----
        public static float Lerp(float a, float b, float t)
            => a + (b - a) * Clamp01(t);

        public static float LerpUnclamped(float a, float b, float t)
            => a + (b - a) * t;

        public static float InverseLerp(float a, float b, float value)
            => MathF.Abs(b - a) > Epsilon ? Clamp01((value - a) / (b - a)) : 0f;

        public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
        {
            t = Clamp01(t);
            return new Vector3(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t);
        }

        public static Vector3 LerpUnclamped(Vector3 a, Vector3 b, float t)
            => new(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t);

        // ---- クランプ ----
        public static float Clamp(float value, float min, float max)
            => Mathf.Min(Mathf.Max(value, min), max);

        public static int Clamp(int value, int min, int max)
            => Mathf.Min(Mathf.Max(value, min), max);

        public static float Clamp01(float value)
            => Clamp(value, 0f, 1f);

        // ---- Min / Max ----
        public static float Min(float a, float b) => MathF.Min(a, b);
        public static float Max(float a, float b) => MathF.Max(a, b);
        public static int Min(int a, int b) => Min(a, b);
        public static int Max(int a, int b) => Max(a, b);

        // ---- 近似比較 ----

        /// <summary>
        /// 2つの浮動小数点数がほぼ等しいかどうかを判断します。これは、両方の値が非常に小さい場合や、両方の値が非常に大きい場合でも機能するように設計されています。
        /// </summary>
        /// <param name="a"></param>
        /// <param name="b"></param>
        /// <returns></returns>
        public static bool Approximately(float a, float b)
            => Abs(a - b) < MathF.Max(1E-6f * MathF.Max(Abs(a), Abs(b)), Epsilon * 8f);

        // ---- スムーズステップ ----
        public static float SmoothStep(float from, float to, float t)
        {
            t = Clamp01(t);
            t = t * t * (3f - 2f * t);
            return to * t + from * (1f - t);
        }

        // ---- 角度 ----
        public static float DeltaAngle(float current, float target)
        {
            float delta = Repeat(target - current, 360f);
            if (delta > 180f) delta -= 360f;
            return delta;
        }

        public static float Repeat(float t, float length)
            => Clamp(t - Floor(t / length) * length, 0f, length);

        public static float PingPong(float t, float length)
        {
            t = Repeat(t, length * 2f);
            return length - Abs(t - length);
        }

        // ---- MoveTowards ----
        public static float MoveTowards(float current, float target, float maxDelta)
        {
            if (Abs(target - current) <= maxDelta) return target;
            return current + Sign(target - current) * maxDelta;
        }

        public static Vector3 MoveTowards(Vector3 current, Vector3 target, float maxDelta)
        {
            var diff = target - current;
            float dist = Vector3.Magnitude(diff);
            if (dist <= maxDelta || dist < Epsilon) return target;
            return current + diff * (maxDelta / dist);
        }
    }
}
