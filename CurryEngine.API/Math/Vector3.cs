using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using CurryEngine.Math;

namespace CurryEngine;

[StructLayout(LayoutKind.Sequential)]   // P/Invoke でそのまま渡せる
public struct Vector3
{
    public float x, y, z;

    public Vector3(float x, float y, float z) { this.x = x; this.y = y; this.z = z; }

    public static readonly Vector3 zero = new(0, 0, 0);
    public static readonly Vector3 one = new(1, 1, 1);
    public static readonly Vector3 up = new(0, 1, 0);
    public static readonly Vector3 down = new(0, -1, 0);
    public static readonly Vector3 forward = new(0, 0, 1);
    public static readonly Vector3 backward = new(0, 0, -1);
    public static readonly Vector3 right = new(1, 0, 0);
    public static readonly Vector3 left = new(-1, 0, 0);

    // ---- 算術演算子 ----
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector3 operator +(Vector3 a, Vector3 b)
        => new(a.x + b.x, a.y + b.y, a.z + b.z);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector3 operator -(Vector3 a, Vector3 b)
        => new(a.x - b.x, a.y - b.y, a.z - b.z);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector3 operator *(Vector3 a, Vector3 b)
        => new(a.x * b.x, a.y * b.y, a.z * b.z);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector3 operator *(Vector3 v, float s)
        => new(v.x * s, v.y * s, v.z * s);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector3 operator *(float s, Vector3 v) => v * s;

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector3 operator -(Vector3 v) => new(-v.x, -v.y, -v.z);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector3 operator /(Vector3 v, float s)
        => new(v.x / s, v.y / s, v.z / s);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector3 operator /(float s, Vector3 v)
        => new(s / v.x, s / v.y, s / v.z);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector3 operator %(float s, Vector3 v)
        => new(s % v.x, s % v.y, s % v.z);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector3 operator %(Vector3 v, float s)
        => new(v.x % s, v.y % s, v.z % s);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(Vector3 a, Vector3 b)
        => Mathf.Approximately(a.x, b.x) && Mathf.Approximately(a.y, b.y) && Mathf.Approximately(a.z, b.z);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(Vector3 a, Vector3 b) => !(a == b);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public override int GetHashCode() => HashCode.Combine(x, y, z);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public override bool Equals(object? obj)
        => obj is Vector3 v && this == v;
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator >(Vector3 a, Vector3 b)
        => a.x > b.x && a.y > b.y && a.z > b.z;
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator <(Vector3 a, Vector3 b)
        => a.x < b.x && a.y < b.y && a.z < b.z;
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator >=(Vector3 a, Vector3 b)
        => a.x >= b.x && a.y >= b.y && a.z >= b.z;
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator <=(Vector3 a, Vector3 b)
        => a.x <= b.x && a.y <= b.y && a.z <= b.z;

    // ---- 便利メソッド ----

    public static float Magnitude(Vector3 v)
        => MathF.Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    public readonly float magnitude
        => Magnitude(this);


    public static Vector3 Normalize(Vector3 v)
    {
        float mag = Magnitude(v);
        return mag > Mathf.Epsilon ? v * (1f / mag) : zero;
    }

    public Vector3 normalized
    {
        get
        {
            float mag = magnitude;
            return mag > Mathf.Epsilon ? this * (1f / mag) : zero;
        }
    }

    public static float Dot(Vector3 a, Vector3 b)
        => a.x * b.x + a.y * b.y + a.z * b.z;

    public static Vector3 Cross(Vector3 a, Vector3 b) => new(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);

    public static float Distance(Vector3 a, Vector3 b)
        => Magnitude(a - b);

    public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
        => a + (b - a) * t;

    public readonly bool Equals(Vector3 other)
        => x == other.x && y == other.y && z == other.z;

    public override readonly string ToString() => $"({x:F3}, {y:F3}, {z:F3})";
}
