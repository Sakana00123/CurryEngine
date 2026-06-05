using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CurryEngine.Interop;

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

[StructLayout(LayoutKind.Sequential)]
public struct TriggerInfoDto
{
    public ulong selfId;
    public ulong selfColliderId;
    public ulong otherId;
    public ulong otherColliderId;
}