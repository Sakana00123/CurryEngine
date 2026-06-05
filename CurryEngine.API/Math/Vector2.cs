using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using CurryEngine.Math;

namespace CurryEngine;

[StructLayout(LayoutKind.Sequential)]  // P/Invoke でそのまま渡せる
public struct Vector2
{
    public float x, y;
    public Vector2(float x, float y) { this.x = x; this.y = y; }
    public static readonly Vector2 zero = new(0, 0);
    public static readonly Vector2 one = new(1, 1);
    public static readonly Vector2 up = new(0, 1);
    public static readonly Vector2 down = new(0, -1);
    public static readonly Vector2 right = new(1, 0);
    public static readonly Vector2 left = new(-1, 0);

    // ---- 算術演算子 ----
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector2 operator +(Vector2 a, Vector2 b)
        => new(a.x + b.x, a.y + b.y);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector2 operator -(Vector2 a, Vector2 b)
        => new(a.x - b.x, a.y - b.y);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector2 operator *(Vector2 v, float s)
        => new(v.x * s, v.y * s);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector2 operator *(float s, Vector2 v) => v * s;
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector2 operator -(Vector2 v) => new(-v.x, -v.y);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector2 operator /(Vector2 v, float s)
        => new(v.x / s, v.y / s);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector2 operator /(float s, Vector2 v)
        => new(s / v.x, s / v.y);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector2 operator %(Vector2 v, float s)
        => new(v.x % s, v.y % s);
     [MethodImpl(MethodImplOptions.AggressiveInlining)]
     public static Vector2 operator %(float s, Vector2 v)
        => new(s % v.x, s % v.y);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(Vector2 a, Vector2 b)
        => Mathf.Approximately(a.x, b.x) && Mathf.Approximately(a.y, b.y);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(Vector2 a, Vector2 b) => !(a == b);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator <(Vector2 a, Vector2 b)
        => a.x < b.x && a.y < b.y;
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator >(Vector2 a, Vector2 b)
        => a.x > b.x && a.y > b.y;
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator <=(Vector2 a, Vector2 b)
        => a.x <= b.x && a.y <= b.y;
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator >=(Vector2 a, Vector2 b)
        => a.x >= b.x && a.y >= b.y;

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public override bool Equals(object? obj)
        => obj is Vector2 v && this == v;
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public override int GetHashCode() => HashCode.Combine(x, y);

    // ---- 便利メソッド ----
    public static float Magnitude(Vector2 v)
        => MathF.Sqrt(v.x * v.x + v.y * v.y);
    public readonly float magnitude
        => Magnitude(this);
    public static Vector2 Normalize(Vector2 v)
    {
        float mag = Magnitude(v);
        return mag > Mathf.Epsilon ? v * (1f / mag) : zero;
    }
    public Vector2 normalized
    {
        get
        {
            float mag = magnitude;
            return mag > Mathf.Epsilon ? this * (1f / mag) : zero;
        }
    }

    public override string ToString() => $"({x}, {y})";
}