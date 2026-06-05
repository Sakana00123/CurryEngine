using CurryEngine.Math;
using CurryEngine.Interop;
using System;

namespace CurryEngine;

/// <summary>
/// C++側のTransformデータへの薄いラッパー。
/// インスタンスはデータを持たず、全アクセスはP/Invoke経由。
/// </summary>
public sealed class Transform : Component
{
    // ---- Position ----

    public Vector3 position
    {
        get => NativeMethods.Transform_GetPosition(ownerId);
        set => NativeMethods.Transform_SetPosition(ownerId, value);
    }

    public Vector3 localPosition
    {
        get => NativeMethods.Transform_GetLocalPosition(ownerId);
        set => NativeMethods.Transform_SetLocalPosition(ownerId, value);
    }

    /// <summary>現在位置から相対移動</summary>
    public void Translate(Vector3 delta)
    {
        NativeMethods.Transform_Translate(ownerId, delta);
    }

    // ---- Rotation ----

    public Quaternion rotation
    {
        get => NativeMethods.Transform_GetRotation(ownerId);
        set => NativeMethods.Transform_SetRotation(ownerId, value);
    }

    public Vector3 eulerAngles
    {
        get => rotation.eulerAngles;
        set => rotation = Quaternion.Euler(value.x, value.y, value.z);
    }

    public void Rotate(Vector3 axis, float degrees, Space space = Space.Self)
    {
        var delta = Quaternion.AxisAngle(axis, degrees * MathF.PI / 180f);
        rotation = space == Space.Self
            ? rotation * delta
            : delta * rotation;
    }

    public void LookAt(Vector3 target, Vector3? up = null)
    {
        var upVec = up ?? Vector3.up;
        NativeMethods.Transform_LookAt(ownerId, target, upVec);
    }

    // ---- Scale ----

    public Vector3 scale
    {
        get => NativeMethods.Transform_GetScale(ownerId);
        set => NativeMethods.Transform_SetScale(ownerId, value);
    }

    // ---- 方向ベクトル (読み取り専用) ----

    public Vector3 forward => NativeMethods.Transform_GetForward(ownerId);
    public Vector3 right => NativeMethods.Transform_GetRight(ownerId);
    public Vector3 up => NativeMethods.Transform_GetUp(ownerId);

    // ---- 親子関係 ----

    public Transform? parent
    {
        get
        {
            ulong parentId = NativeMethods.Transform_GetParent(ownerId);
            return Accessor?.GetTransform(parentId);
        }
        set => NativeMethods.Transform_SetParent(
                   ownerId, value?.ownerId ?? 0);
    }

    public int ChildCount
        => NativeMethods.Transform_GetChildCount(ownerId);

    public Transform GetChild(int index)
    {
        ulong childId = NativeMethods.Transform_GetChild(ownerId, index);
        if (childId == 0)
            throw new IndexOutOfRangeException($"No child at index {index}");
        return Accessor?.GetTransform(childId) ?? throw new InvalidOperationException($"Child ID {childId} is not a Transform");
    }
}

public enum Space { World, Self }