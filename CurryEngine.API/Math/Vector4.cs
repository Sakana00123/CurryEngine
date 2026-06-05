using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using CurryEngine.Math;

namespace CurryEngine;

[StructLayout(LayoutKind.Sequential)]
public class Vector4
{
    public float x, y, z, w;
    
    public Vector4(float x, float y, float z, float w) { this.x = x; this.y = y; this.z = z; this.w = w; }
    
    public static readonly Vector4 Zero = new(0, 0, 0, 0);
    public static readonly Vector4 One = new(1, 1, 1, 1);
    public static readonly Vector4 OneMinus = new(-1, -1, -1, -1);
    public static readonly Vector4 UnitX = new(1, 0, 0, 0);
    public static readonly Vector4 UnitY = new(0, 1, 0, 0);
    public static readonly Vector4 UnitZ = new(0, 0, 1, 0);
    public static readonly Vector4 UnitW = new(0, 0, 0, 1);
    public static readonly Vector4 Infinity = new(float.PositiveInfinity, float.PositiveInfinity, float.PositiveInfinity, float.PositiveInfinity);
    public static readonly Vector4 NegativeInfinity = new(float.NegativeInfinity, float.NegativeInfinity, float.NegativeInfinity, float.NegativeInfinity);
    public static readonly Vector4 NaN = new(float.NaN, float.NaN, float.NaN, float.NaN);
    public static readonly Vector4 Epsilon = new(float.Epsilon, float.Epsilon, float.Epsilon, float.Epsilon);
    public static readonly Vector4 MinValue = new(float.MinValue, float.MinValue, float.MinValue, float.MinValue);
    public static readonly Vector4 MaxValue = new(float.MaxValue, float.MaxValue, float.MaxValue, float.MaxValue);
    
    // ---- 算術演算子 ----
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 operator +(Vector4 a, Vector4 b)
        => new(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
    
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 operator -(Vector4 a, Vector4 b)
        => new(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
    
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 operator *(Vector4 a, Vector4 b)
        => new(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 operator *(Vector4 v, float s)
        => new(v.x * s, v.y * s, v.z * s, v.w * s);
    
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 operator *(float s, Vector4 v) => v * s;
    
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 operator -(Vector4 v) => new(-v.x, -v.y, -v.z, -v.w);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 operator /(Vector4 v, float s)
        => new(v.x / s, v.y / s, v.z / s, v.w / s);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 operator /(float s, Vector4 v)
        => new(s / v.x, s / v.y, s / v.z, s / v.w);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 operator %(float s, Vector4 v)
        => new(s % v.x, s % v.y, s % v.z, s % v.w);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 operator %(Vector4 v, float s)
        => new(v.x % s, v.y % s, v.z % s, v.w % s);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(Vector4 a, Vector4 b)
        => Mathf.Approximately(a.x, b.x) && Mathf.Approximately(a.y, b.y) && Mathf.Approximately(a.z, b.z) && Mathf.Approximately(a.w, b.w);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(Vector4 a, Vector4 b) => !(a == b);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public override bool Equals(object? obj)
        => obj is Vector4 other && this == other;
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public override int GetHashCode() => HashCode.Combine(x, y, z, w);
    
    public override string ToString()
         => $"Vector4(x={x}, y={y}, z={z}, w={w})";

    // 便利な暗黙の型変換 (Colorと相互に変換可能)
    public static implicit operator Color(Vector4 v)
        => new(v.x, v.y, v.z, v.w);
}
