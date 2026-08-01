using CurryEngine.Math;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace CurryEngine;

/// <summary>
/// RGBA各成分を0.0～1.0の範囲で表す構造体。
/// 内部的にはfloat4と同じレイアウトで、C++側とP/Invokeでやり取りするために定義されている。
/// ユーザーコードで直接インスタンスを作成することは想定されておらず、Colorプロパティを通じて取得・設定される。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Color
{
    public float r, g, b, a;

    public Color(float r, float g, float b, float a = 1f)
    {
        this.r = r;
        this.g = g;
        this.b = b;
        this.a = a;
    }
    public Color(Vector4 v)
    {
        this.r = v.x;
        this.g = v.y;
        this.b = v.z;
        this.a = v.w;
    }
    public Color(string hex) : this(FromHex(hex)) { }
    public Color(uint hex) : this(FromHex(hex)) { }

    public static readonly Color White = new(1f, 1f, 1f, 1f);
    public static readonly Color Black = new(0f, 0f, 0f, 1f);
    public static readonly Color Gray = new(0.5f, 0.5f, 0.5f, 1f);
    public static readonly Color Red = new(1f, 0f, 0f, 1f);
    public static readonly Color Green = new(0f, 1f, 0f, 1f);
    public static readonly Color Blue = new(0f, 0f, 1f, 1f);
    public static readonly Color Clear = new(0f, 0f, 0f, 0f);
    public static readonly Color Gold = new(1f, 0.843137f, 0f, 1f);
    public static readonly Color Yellow = new(1f, 1f, 0f, 1f);
    public static readonly Color Purple = new(0.5f, 0f, 0.5f, 1f);
    public static readonly Color Cyan = new(0f, 1f, 1f, 1f);
    public static readonly Color Magenta = new(1f, 0f, 1f, 1f);
    public static readonly Color Pink = new(1f, 0.75f, 0.8f, 1f);
    public static readonly Color Orange = new(1f, 0.647059f, 0f, 1f);
    public static readonly Color Lime = new(0f, 1f, 0f, 1f);

    public static Color FromHex(string v)
    {
        if (v.StartsWith("#"))
            v = v[1..];
        if (v.Length == 6)
            v += "FF"; // alphaが指定されていない場合は不透明とみなす
        if (v.Length != 8)
            throw new ArgumentException("Hex color string must be in the format #RRGGBB or #RRGGBBAA");
        
        byte r = Convert.ToByte(v.Substring(0, 2), 16);
        byte g = Convert.ToByte(v.Substring(2, 2), 16);
        byte b = Convert.ToByte(v.Substring(4, 2), 16);
        byte a = Convert.ToByte(v.Substring(6, 2), 16);

        return new Color(r / 255f, g / 255f, b / 255f, a / 255f);
    }

    public static Color FromHex(uint hex)
    {
        byte r = (byte)((hex >> 24) & 0xFF);
        byte g = (byte)((hex >> 16) & 0xFF);
        byte b = (byte)((hex >> 8) & 0xFF);
        byte a = (byte)(hex & 0xFF);
        return new Color(r / 255f, g / 255f, b / 255f, a / 255f);
    }

    public uint ToHex()
    {
        byte r = (byte)(Mathf.Clamp((int)(this.r * 255), 0, 255));
        byte g = (byte)(Mathf.Clamp((int)(this.g * 255), 0, 255));
        byte b = (byte)(Mathf.Clamp((int)(this.b * 255), 0, 255));
        byte a = (byte)(Mathf.Clamp((int)(this.a * 255), 0, 255));
        return ((uint)r << 24) | ((uint)g << 16) | ((uint)b << 8) | a;
    }

    public static Color FromHSV(float h, float s, float v, float a = 1f)
    {
        h = h % 360f;
        if (h < 0) h += 360f;
        s = Mathf.Clamp01(s);
        v = Mathf.Clamp01(v);
        a = Mathf.Clamp01(a);
        float c = v * s;
        float x = c * (1 - Mathf.Abs((h / 60f) % 2 - 1));
        float m = v - c;
        float r1, g1, b1;
        if (h < 60)
            (r1, g1, b1) = (c, x, 0);
        else if (h < 120)
            (r1, g1, b1) = (x, c, 0);
        else if (h < 180)
            (r1, g1, b1) = (0, c, x);
        else if (h < 240)
            (r1, g1, b1) = (0, x, c);
        else if (h < 300)
            (r1, g1, b1) = (x, 0, c);
        else
            (r1, g1, b1) = (c, 0, x);
        return new Color(r1 + m, g1 + m, b1 + m, a);
    }

    public void ToHSV(out float h, out float s, out float v)
    {
        float max = Mathf.Max(r, Mathf.Max(g, b));
        float min = Mathf.Min(r, Mathf.Min(g, b));
        v = max;
        float delta = max - min;
        if (max == 0)
        {
            s = 0;
            h = 0; // 色相は定義されないが、便宜上0にする
            return;
        }
        s = delta / max;
        if (delta == 0)
        {
            h = 0; // 色相は定義されないが、便宜上0にする
        }
        else if (max == r)
        {
            h = 60 * (((g - b) / delta) % 6);
        }
        else if (max == g)
        {
            h = 60 * (((b - r) / delta) + 2);
        }
        else // max == b
        {
            h = 60 * (((r - g) / delta) + 4);
        }
        if (h < 0)
            h += 360;
    }



    public static Color Lerp(Color a, Color b, float t)
    {
        t = Mathf.Clamp01(t);
        return new Color(
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        );
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Color operator *(Color c, float f)
        => new(c.r * f, c.g * f, c.b * f, c.a * f);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Color operator *(float f, Color c) => c * f;

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Color operator +(Color a, Color b)
        => new(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Color operator -(Color a, Color b)
        => new(a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Color operator /(Color c, float f)
        => new(c.r / f, c.g / f, c.b / f, c.a / f);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Color operator /(float f, Color c)
        => new(f / c.r, f / c.g, f / c.b, f / c.a);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Color operator -(Color c)
        => new(-c.r, -c.g, -c.b, -c.a);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Color operator *(Color a, Color b)
        => new(a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Color operator /(Color a, Color b)
        => new(a.r / b.r, a.g / b.g, a.b / b.b, a.a / b.a);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(Color a, Color b)
        => Mathf.Approximately(a.r, b.r) && Mathf.Approximately(a.g, b.g) && Mathf.Approximately(a.b, b.b) && Mathf.Approximately(a.a, b.a);
    
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(Color a, Color b) => !(a == b);
    
    public override bool Equals(object? obj)
        => obj is Color other && this == other;
    
    public override int GetHashCode()
        => HashCode.Combine(r.GetHashCode(), g.GetHashCode(), b.GetHashCode(), a.GetHashCode());
    
    public override string ToString()
        => $"Color(r={r}, g={g}, b={b}, a={a})";

    // 便利な暗黙の型変換 (float4と相互に変換可能)
    public static implicit operator Vector4(Color c)
        => new(c.r, c.g, c.b, c.a);
}
