using System.Runtime.InteropServices;

namespace CurryEngine.Runtime.Native;

/// <summary>
/// 衝突情報を表すデータ転送オブジェクト (Data Transfer Object)。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct ContactDto
{
    public float pointX, pointY, pointZ;
    public float normalX, normalY, normalZ;
    public float separation;
    public ulong selfId;
    public ulong selfColliderId;
    public ulong otherId;
    public ulong otherColliderId;
}

/// <summary>
/// 衝突情報を表すデータ転送オブジェクト (Data Transfer Object)。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct CollisionInfoDto
{
    public ulong selfId;
    public ulong selfColliderId;
    public ulong otherId;
    public ulong otherColliderId;
    public float impulseX, impulseY, impulseZ;
    public uint contactCount;
    public fixed byte contacts[8 * (7 * sizeof(float) + 4 * sizeof(ulong))]; // 8 contacts, each contact has 7 floats (point and normal) and 4 ulongs (ids)
}

/// <summary>
/// トリガー情報を表すデータ転送オブジェクト (Data Transfer Object)。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct TriggerInfoDto
{
    public ulong selfId;
    public ulong selfColliderId;
    public ulong otherId;
    public ulong otherColliderId;
}
