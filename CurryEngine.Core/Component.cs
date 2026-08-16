
using CurryEngine.Interfaces;
using System;

namespace CurryEngine;

/// <summary>
/// 全コンポーネントの基底クラス。
/// データは持たず、entityId をキーにC++メモリへアクセスする。
/// </summary>
public abstract class Component : Object
{
    internal static IComponentAccessor? Accessor { get; set; }

    internal ulong ownerId { get; private set; }

    protected Component()
    { 
    }
    internal void Setup(ulong ownerId, ulong objectId)
    {
        this.ownerId = ownerId;
        SetObjectIdInternal(objectId);
    }

    /// <summary>
    /// 同一エンティティ上の他のコンポーネントを取得する。
    /// </summary>
    /// <typeparam name="T"> 取得したいコンポーネントの型 </typeparam>
    /// <returns> コンポーネントが存在すればそのインスタンス、存在しなければ null </returns>
    public T? GetComponent<T>() where T : Component
        => Accessor?.Get<T>(ownerId);

    /// <summary>
    /// 同一エンティティ上の他のコンポーネントを取得する。
    /// </summary>
    /// <typeparam name="T"> 取得したいコンポーネントの型 </typeparam>
    /// <returns> コンポーネントが存在すればそのインスタンス </returns>
    /// <exception cref="InvalidOperationException"> コンポーネントが存在しない場合 </exception>
    public T GetRequiredComponent<T>() where T : Component
        => Accessor?.Get<T>(ownerId)
           ?? throw new InvalidOperationException(
               $"Component {typeof(T).Name} not found on entity {ownerId}");

    /// <summary>
    /// 同一エンティティ上の他のコンポーネントを取得する。
    /// </summary>
    /// <typeparam name="T"> 取得したいコンポーネントの型 </typeparam>
    /// <param name="component"> コンポーネントが存在すればそのインスタンス、存在しなければ null </param>
    /// <returns> コンポーネントが存在すれば true、存在しなければ false </returns>
    public bool TryGetComponent<T>(out T component) where T : Component
    {
        component = Accessor?.Get<T>(ownerId)!;
        return component != null;
    }

    /// <summary>
    /// このコンポーネントがアタッチされている GameObject を取得する。
    /// </summary>
    public GameObject gameObject
        => GameObject.Accessor?.GetOrCreate(ownerId)
           ?? throw new InvalidOperationException($"GameObject not found for Entity {ownerId}");

    /// <summary>
    /// このコンポーネントがアタッチされている GameObject の Transform コンポーネントを取得する。
    /// </summary>
    public Transform transform
    {
        get
        {
            var transform = Accessor?.Get<Transform>(ownerId);
            return transform ?? throw new InvalidOperationException($"Transform component not found on Entity {ownerId}");
        }
    }

    /// <summary>
    /// このコンポーネントのエンティティが存在するか。
    /// </summary>
    public bool IsValid
        => Accessor?.IsValid(objectId) ?? false;

    /// <summary>
    /// このコンポーネントが有効か (エンティティが存在し、かつこの型のコンポーネントがアタッチされているか)。
    /// </summary>
    public bool Enabled
    {
        get => Accessor?.IsEnabled(objectId, ownerId) ?? false;
        set => Accessor?.SetEnabled(objectId, ownerId, value);
    }

    public static bool operator ==(Component? a, Component? b)
    {
        if (ReferenceEquals(a, b)) return true;
        // isValid=false は null と同等に扱う
        uint bitA = (uint)a;// 有効かどうかの判定をビット演算で行うために uint に変換
        uint bitB = (uint)b;// 有効かどうかの判定をビット演算で行うために uint に変換
        if ((bitA ^ bitB) == 1) return false; // どちらか一方が有効で、もう一方が無効の場合は等しくない
        if (bitA == 0 && bitB == 0) return true; // 両方とも無効の場合は等しいとみなす
        if (a is null || b is null) return false; // どちらか一方が null の場合は等しくない
        return a.objectId == b.objectId && a.ownerId == b.ownerId; // 同じ型で objectId と ownerId が同じなら等しいとみなす
    }

    public static bool operator !=(Component? a, Component? b)
        => !(a == b);

    public override bool Equals(object? obj)
    {
        if (obj is Component other)
            return this == other;
        return false;
    }

    public override int GetHashCode()
        => HashCode.Combine(objectId, ownerId);

    /// <summary>
    /// このコンポーネントが有効かを bool 型として評価する。
    /// </summary>
    /// <param name="component"> 評価するコンポーネント </param>
    public static implicit operator bool(Component? component)
        => component != null && component.IsValid;




#if false // TODO: 今後実装するか検討中。
    // ----- ライフサイクルコールバック (override して使う) -----

    internal void InvokeOnAttached(ulong ownerId, ulong objectId)
    {
        if (IsValid)
        {
            this.ownerId = ownerId;
            SetObjectIdInternal(objectId);
            OnAttached();
        }
    }
    internal virtual void OnAttached() { }
    internal virtual void OnDetached() { } 
#endif

    public override string ToString()
        => $"{GetType().Name}(entity={ownerId})";
}
