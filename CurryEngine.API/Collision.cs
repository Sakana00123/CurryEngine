using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

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
    public Collider? thisCollider;  // 接触している自分のコライダー
    public Collider? otherCollider; // 接触相手のコライダー
}

/// <summary>
/// 衝突情報を表す構造体。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Collision
{
    public Collider? thisCollider;    // 衝突している自分のコライダー
    public Collider? otherCollider;   // 衝突相手のコライダー
    public ContactPoint[] contacts;          // 接触点の配列
    public Vector3 impulse;      // 衝突の衝撃力
}

/// <summary>
/// トリガー情報を表す構造体。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Trigger
{
    public Collider? thisCollider;    // トリガーに入った自分のコライダー
    public Collider? otherCollider;   // トリガーに入った相手のコライダー
}