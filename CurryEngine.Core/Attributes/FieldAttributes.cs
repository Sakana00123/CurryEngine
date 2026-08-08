using System;

namespace CurryEngine;

/// <summary>
/// private フィールドをインスペクタに表示・シリアライズさせるための属性。
/// </summary>
[AttributeUsage(AttributeTargets.Field)]
public sealed class SerializeField : Attribute { }

/// <summary>
/// public フィールドをインスペクタに表示させないための属性。
/// </summary>
[AttributeUsage(AttributeTargets.Field)]
public sealed class HideInInspector : Attribute { }

/// <summary>
/// フィールドをシリアライズさせないための属性。
/// </summary>
[AttributeUsage(AttributeTargets.Field)]
public sealed class NonSerialized : Attribute { }

/// <summary>
/// フィールドをインスペクタでグループ化するための属性。
/// </summary>
[AttributeUsage(AttributeTargets.Field)]
public sealed class Header : Attribute
{
    public string Text { get; }
    public Header(string text)
    {
        Text = text;
    }
}

/// <summary>
/// フィールドの値を指定した範囲内に制限するための属性。
/// </summary>
[AttributeUsage(AttributeTargets.Field)]
public sealed class Range : Attribute
{
    public float Min { get; }
    public float Max { get; }
    public Range(float min, float max)
    {
        Min = min;
        Max = max;
    }
}

/// <summary>
/// フィールドの値を指定した最小値以上に制限するための属性。
/// </summary>
[AttributeUsage(AttributeTargets.Field)]
public sealed class Min : Attribute
{
    public float Value { get; }
    public Min(float value)
    {
        Value = value;
    }
}

/// <summary>
/// フィールドの値を指定した最大値以下に制限するための属性。
/// </summary>
[AttributeUsage(AttributeTargets.Field)]
public sealed class Max : Attribute
{
    public float Value { get; }
    public Max(float value)
    {
        Value = value;
    }
}

/// <summary>
/// フィールドにツールチップを表示するための属性。
/// </summary>
[AttributeUsage(AttributeTargets.Field)]
public sealed class Tooltip : Attribute
{
    public string Text { get; }
    public Tooltip(string text)
    {
        Text = text;
    }
}