
using System.Runtime.InteropServices;

namespace CurryEngine;

/// <summary>
/// 接触点を表す構造体。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct ContactPoint
{
    public Vector3 position;    // 接触点の位置
    public Vector3 normal;      // 接触面の法線ベクトル
    public float separation;   // 接触点の分離距離（負なら貫通している）
    public ulong thisColliderId; // 接触している自分のコライダーのID
    public ulong otherColliderId; // 接触している相手のコライダーのID
}

/// <summary>
/// 衝突情報を表す構造体。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Collision
{
    public ulong thisColliderId;          // 衝突した自分のコライダーのID
    public ulong otherColliderId;         // 衝突した相手のコライダーのID
    public ContactPoint[] contacts;          // 接触点の配列
    public Vector3 impulse;      // 衝突の衝撃力
}

/// <summary>
/// トリガー情報を表す構造体。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Trigger
{
    public ulong thisColliderId;    // トリガーに入った自分のコライダー
    public ulong otherColliderId;   // トリガーに入った相手のコライダー
}
