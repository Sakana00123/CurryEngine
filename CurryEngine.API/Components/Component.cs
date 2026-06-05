using CurryEngine.Interop;
using System;
namespace CurryEngine;

/// <summary>
/// 全コンポーネントの基底クラス。
/// データは持たず、entityId をキーにC++メモリへアクセスする。
/// </summary>
public abstract class Component : Object
{
    // C++側のコンポーネントアクセス用インターフェース (実装は EngineCore)
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
        => new(ownerId);

    /// <summary>
    /// このコンポーネントがアタッチされている GameObject の Transform コンポーネントを取得する。
    /// </summary>
    public Transform transform
    {
        get
        {
            if (Component.Accessor == null)
                throw new InvalidOperationException("Component accessor is not set. This likely means that the engine is not properly initialized.");
            var transform = Accessor.GetTransform(ownerId);
            return transform ?? throw new InvalidOperationException($"Transform component not found on Entity {ownerId}");
        }
    }

    /// <summary>
    /// このコンポーネントのエンティティが存在するか。
    /// </summary>
    public bool IsValid
        => NativeMethods.Entity_IsValid(ownerId);

    /// <summary>
    /// このコンポーネントが有効か (エンティティが存在し、かつこの型のコンポーネントがアタッチされているか)。
    /// </summary>
    public bool enabled
    {
        get => NativeMethods.Component_GetEnabled(ownerId, objectId) != 0;
        set => NativeMethods.Component_SetEnabled(ownerId, objectId, value ? 1 : 0);
    }

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

    public override string ToString()
        => $"{GetType().Name}(entity={ownerId})";
}
