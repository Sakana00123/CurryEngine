using CurryEngine.Interfaces;
using System;
using System.Runtime.InteropServices;

namespace CurryEngine;

[StructLayout(LayoutKind.Sequential)]
public struct LayerMask
{
    internal static ILayerMask? Accessor;

    public int value;
    public LayerMask(int value)
    {
        this.value = value;
    }

    /// <summary>
    /// LayerMaskをint型に暗黙的に変換します。
    /// </summary>
    /// <param name="mask"> 変換するLayerMask</param>
    public static implicit operator int(LayerMask mask) => mask.value;

    /// <summary>
    /// int型をLayerMaskに暗黙的に変換します。
    /// </summary>
    /// <param name="value"> 変換するint値</param>
    public static implicit operator LayerMask(int value) => new LayerMask(value);


    public static LayerMask operator |(LayerMask a, LayerMask b)
    {
        return new LayerMask(a.value | b.value);
    }

    public static LayerMask operator &(LayerMask a, LayerMask b)
    {
        return new LayerMask(a.value & b.value);
    }

    public static LayerMask operator ~(LayerMask a)
    {
        return new LayerMask(~a.value);
    }

    public static LayerMask operator ^(LayerMask a, LayerMask b)
    {
        return new LayerMask(a.value ^ b.value);
    }

    public static LayerMask operator <<(LayerMask a, int shift)
    {
        return new LayerMask(a.value << shift);
    }

    public static LayerMask operator >>(LayerMask a, int shift)
    {
        return new LayerMask(a.value >> shift);
    }

    public static LayerMask GetMask(params string[] layerNames)
    {
        if (Accessor == null)
        {
            Debug.LogError("LayerMask accessor is not implemented.");
            return new LayerMask(0);
        }
        int maskValue = 0;
        foreach (var layerName in layerNames)
        {
            maskValue |= Accessor.GetLayerMaskValue(layerName).value;
        }
        return new LayerMask(maskValue);
    }

    public static LayerMask NameToLayer(string layerName)
    {
        return Accessor?.GetLayerMaskValue(layerName) ?? throw new NotImplementedException("LayerMask accessor is not implemented.");
    }

    public static string LayerToName(int layer)
    {
        return Accessor?.GetLayerName(new LayerMask(layer)) ?? throw new NotImplementedException("LayerMask accessor is not implemented.");
    }


}
