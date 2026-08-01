using System;
using System.Runtime.InteropServices;
using CurryEngine.Math;
namespace CurryEngine;

[StructLayout(LayoutKind.Sequential)]
public struct Quaternion
{
    public float x, y, z, w;

    public Quaternion(float x, float y, float z, float w)
    { this.x = x; this.y = y; this.z = z; this.w = w; }

    public static readonly Quaternion identity = new(0, 0, 0, 1);

    // ---- 生成 ----

    /// <summary>オイラー角(度)からクォータニオンを生成</summary>
    public static Quaternion Euler(float pitch, float yaw, float roll)
    {
        float toRad = Mathf.Deg2Rad * 0.5f;
        float cx = MathF.Cos(pitch * toRad), sx = MathF.Sin(pitch * toRad);
        float cy = MathF.Cos(yaw * toRad), sy = MathF.Sin(yaw * toRad);
        float cz = MathF.Cos(roll * toRad), sz = MathF.Sin(roll * toRad);
        return new(
            sx * cy * cz + cx * sy * sz,
            cx * sy * cz - sx * cy * sz,
            cx * cy * sz + sx * sy * cz,
            cx * cy * cz - sx * sy * sz);
    }

    public static Quaternion Euler(Vector3 euler)
        => Euler(euler.x, euler.y, euler.z);


    /// <summary>軸 + 角度(ラジアン) から生成</summary>
    /// <param name="axis">回転軸 (正規化されている必要あり)</param>
    /// <param name="angle">回転角度 (度)</param>
    /// <returns>回転を表すクォータニオン</returns>
    public static Quaternion AxisAngle(Vector3 axis, float angle)
    {
        float halfAngle = angle * 0.5f;
        float s = MathF.Sin(halfAngle);
        return new Quaternion(
            axis.x * s,
            axis.y * s,
            axis.z * s,
            MathF.Cos(halfAngle));
    }

    /// <summary>fromからtoへの回転を表すクォータニオンを生成</summary>
    public static Quaternion FromToRotation(Vector3 from, Vector3 to)
    {
        from = from.normalized;
        to = to.normalized;
        float dot = Vector3.Dot(from, to);

        if (dot >= 1f - Mathf.Epsilon)
            return identity;

        if (dot <= -1f + Mathf.Epsilon)
        {
            // 180度回転 — 任意の垂直軸を使う
            var axis = Vector3.Cross(Vector3.right, from);
            if (axis.magnitude < Mathf.Epsilon)
                axis = Vector3.Cross(Vector3.up, from);
            return AxisAngle(axis.normalized, 180f);
        }

        var cross = Vector3.Cross(from, to);
        float s = MathF.Sqrt((1f + dot) * 2f);
        return new(cross.x / s, cross.y / s, cross.z / s, s * 0.5f);
    }

    /// <summary>指定方向を向くクォータニオンを生成</summary>
    public static Quaternion LookRotation(Vector3 forward, Vector3 up)
    {
        forward = forward.normalized;
        var right = Vector3.Cross(up, forward).normalized;
        up = Vector3.Cross(forward, right);

        float m00 = right.x, m01 = right.y, m02 = right.z;
        float m10 = up.x, m11 = up.y, m12 = up.z;
        float m20 = forward.x, m21 = forward.y, m22 = forward.z;
        float trace = m00 + m11 + m22;

        if (trace > 0f)
        {
            float s = 0.5f / MathF.Sqrt(trace + 1f);
            return new((m12 - m21) * s, (m20 - m02) * s, (m01 - m10) * s, 0.25f / s);
        }
        if (m00 > m11 && m00 > m22)
        {
            float s = 2f * MathF.Sqrt(1f + m00 - m11 - m22);
            return new(0.25f * s, (m01 + m10) / s, (m20 + m02) / s, (m12 - m21) / s);
        }
        if (m11 > m22)
        {
            float s = 2f * MathF.Sqrt(1f + m11 - m00 - m22);
            return new((m01 + m10) / s, 0.25f * s, (m12 + m21) / s, (m20 - m02) / s);
        }
        else
        {
            float s = 2f * MathF.Sqrt(1f + m22 - m00 - m11);
            return new((m20 + m02) / s, (m12 + m21) / s, 0.25f * s, (m01 - m10) / s);
        }
    }

    public static Quaternion LookRotation(Vector3 forward)
        => LookRotation(forward, Vector3.up);

    // ---- 演算 ----

    public static Quaternion operator *(Quaternion a, Quaternion b) => new(
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z);

    /// <summary>クォータニオンでベクトルを回転</summary>
    public static Vector3 operator *(Quaternion q, Vector3 v)
    {
        var u = new Vector3(q.x, q.y, q.z);
        var uv = Vector3.Cross(u, v);
        var uuv = Vector3.Cross(u, uv);
        return v + 2f * (q.w * uv + uuv);
    }

    // ---- プロパティ ----

    public Vector3 eulerAngles
    {
        get
        {
            float toDeg = Mathf.Rad2Deg;
            float sinP = 2f * (w * x - y * z);
            float pitch = MathF.Abs(sinP) >= 1f
                ? MathF.CopySign(90f, sinP)
                : MathF.Asin(sinP) * toDeg;
            float yaw = MathF.Atan2(
                            2f * (w * y + x * z),
                            1f - 2f * (x * x + y * y)) * toDeg;
            float roll = MathF.Atan2(
                            2f * (w * z + x * y),
                            1f - 2f * (x * x + z * z)) * toDeg;
            return new(pitch, yaw, roll);
        }
        set => this = Euler(value);
    }

    public Quaternion normalized
    {
        get
        {
            float mag = MathF.Sqrt(x * x + y * y + z * z + w * w);
            if (mag < Mathf.Epsilon) return identity;
            return new(x / mag, y / mag, z / mag, w / mag);
        }
    }

    // ---- 補間 ----

    /// <summary>球面線形補間</summary>
    public static Quaternion Slerp(Quaternion a, Quaternion b, float t)
    {
        t = Mathf.Clamp01(t);
        return SlerpUnclamped(a, b, t);
    }

    public static Quaternion SlerpUnclamped(Quaternion a, Quaternion b, float t)
    {
        float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

        // 最短経路を保証
        if (dot < 0f) { b = new(-b.x, -b.y, -b.z, -b.w); dot = -dot; }

        if (dot > 1f - Mathf.Epsilon)
            return Lerp(a, b, t);  // 角度が小さい場合は線形補間

        float angle = MathF.Acos(dot);
        float sinA = MathF.Sin(angle);
        float wa = MathF.Sin((1f - t) * angle) / sinA;
        float wb = MathF.Sin(t * angle) / sinA;
        return new(
            wa * a.x + wb * b.x,
            wa * a.y + wb * b.y,
            wa * a.z + wb * b.z,
            wa * a.w + wb * b.w);
    }

    /// <summary>線形補間 (正規化あり)</summary>
    public static Quaternion Lerp(Quaternion a, Quaternion b, float t)
    {
        t = Mathf.Clamp01(t);
        return LerpUnclamped(a, b, t);
    }

    public static Quaternion LerpUnclamped(Quaternion a, Quaternion b, float t)
    {
        float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        float s = dot >= 0f ? 1f : -1f;
        var q = new Quaternion(
            a.x + (b.x * s - a.x) * t,
            a.y + (b.y * s - a.y) * t,
            a.z + (b.z * s - a.z) * t,
            a.w + (b.w * s - a.w) * t);
        return q.normalized;
    }

    /// <summary>一定角速度で回転 (度/秒)</summary>
    public static Quaternion RotateTowards(
        Quaternion from, Quaternion to, float maxDegreesDelta)
    {
        float angle = Angle(from, to);
        if (angle < Mathf.Epsilon) return to;
        return SlerpUnclamped(from, to,
            Mathf.Min(1f, maxDegreesDelta / angle));
    }

    // ---- ユーティリティ ----

    /// <summary>2つのクォータニオン間の角度(度)を返す</summary>
    public static float Angle(Quaternion a, Quaternion b)
    {
        float dot = MathF.Abs(a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
        return dot >= 1f - Mathf.Epsilon
            ? 0f
            : MathF.Acos(MathF.Min(dot, 1f)) * 2f * Mathf.Rad2Deg;
    }

    public static Quaternion Inverse(Quaternion q)
        => new(-q.x, -q.y, -q.z, q.w);

    public override string ToString()
        => $"({x:F3}, {y:F3}, {z:F3}, {w:F3})";
}