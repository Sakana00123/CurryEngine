using CurryEngine.Interfaces;
using CurryEngine.Math;
using System;
using System.ComponentModel;

namespace CurryEngine;

/// <summary>
/// C++側のTransformデータへの薄いラッパー。
/// インスタンスはデータを持たず、全アクセスはP/Invoke経由。
/// </summary>
public sealed class Transform : Component
{
    internal static ITransformAccessor? TransformAccessor { get; set; }

    // ---- Position ----

    public Vector3 position
    {
        get => TransformAccessor?.GetPosition(ownerId) ?? throw new InvalidOperationException("TransformAccessor is not set.");
        set => TransformAccessor?.SetPosition(ownerId, value);
    }

    public Vector3 localPosition
    {
        get => TransformAccessor?.GetLocalPosition(ownerId) ?? throw new InvalidOperationException("TransformAccessor is not set.");
        set => TransformAccessor?.SetLocalPosition(ownerId, value);
    }

    /// <summary>現在位置から相対移動</summary>
    public void Translate(Vector3 delta)
    {
        TransformAccessor?.Translate(ownerId, delta);
    }

    // ---- Rotation ----

    public Quaternion rotation
    {
        get => TransformAccessor?.GetRotation(ownerId) ?? throw new InvalidOperationException("TransformAccessor is not set.");
        set => TransformAccessor?.SetRotation(ownerId, value);
    }

    public Vector3 eulerAngles
    {
        get => rotation.eulerAngles;
        set => rotation = Quaternion.Euler(value.x, value.y, value.z);
    }

    public void Rotate(Vector3 axis, float degrees, Space space = Space.Self)
    {
        var delta = Quaternion.AxisAngle(axis, degrees * Mathf.PI / 180f);
        rotation = space == Space.Self
            ? rotation * delta
            : delta * rotation;
    }

    public void LookAt(Vector3 target, Vector3? up = null)
    {
        var upVec = up ?? Vector3.up;
        TransformAccessor?.LookAt(ownerId, target, upVec);
    }

    // ---- Scale ----

    public Vector3 scale
    {
        get => TransformAccessor?.GetScale(ownerId) ?? throw new InvalidOperationException("TransformAccessor is not set.");
        set => TransformAccessor?.SetScale(ownerId, value);
    }

    // ---- 方向ベクトル (読み取り専用) ----

    public Vector3 forward => TransformAccessor?.GetForward(ownerId) ?? throw new InvalidOperationException("TransformAccessor is not set.");
    public Vector3 right => TransformAccessor?.GetRight(ownerId) ?? throw new InvalidOperationException("TransformAccessor is not set.");
    public Vector3 up => TransformAccessor?.GetUp(ownerId) ?? throw new InvalidOperationException("TransformAccessor is not set.");

    // ---- 親子関係 ----

    public Transform? Parent
    {
        get
        {
            ulong parentId = TransformAccessor?.GetParent(ownerId) ?? 0;
            return Component.Accessor?.Get<Transform>(parentId);
        }
        set => TransformAccessor?.SetParent(ownerId, value?.ownerId ?? 0);
    }

    public int ChildCount
        => TransformAccessor?.GetChildCount(ownerId) ?? throw new InvalidOperationException("TransformAccessor is not set.");

    public Transform GetChild(int index)
    {
        ulong childId = TransformAccessor?.GetChild(ownerId, index) ?? 0;
        if (childId == 0)
            throw new IndexOutOfRangeException($"No child at index {index}");
        return Component.Accessor?.Get<Transform>(childId) ?? throw new InvalidOperationException($"Child ID {childId} is not a Transform");
    }

    public Transform[] GetChildren()
    {
        int count = ChildCount;
        Transform[] children = new Transform[count];
        for (int i = 0; i < count; i++)
        {
            children[i] = GetChild(i);
        }
        return children;
    }
}

public enum Space { World, Self }
