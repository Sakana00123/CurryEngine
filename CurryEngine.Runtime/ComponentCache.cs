using CurryEngine.Interfaces;
using CurryEngine.Runtime.Native;

namespace CurryEngine.Runtime;

internal static class ComponentCache
{
    // キーを componentId に変更
    private static readonly Dictionary<ulong, Component> s_cache = new();

    //private static readonly Dictionary<Type, Func<Component>> s_factories = new();

    // ---- 外部API ----

    /// 最初の1つを返す（単一アタッチ前提の便利メソッド）
    public static T? Get<T>(ulong ownerId) where T : Component
    {
        var ids = NativeMethods.GameObject_GetComponentIdsHelper(ownerId, typeof(T).Name);
        if (ids.Length == 0)
            return default;
        return (T)GetOrCreate(ownerId, ids[0], typeof(T));
    }

    public static Component? Get(ulong ownerId, Type type)
    {
        var ids = NativeMethods.GameObject_GetComponentIdsHelper(ownerId, type.Name);
        if (ids.Length == 0)
            return default;
        return GetOrCreate(ownerId, ids[0], type);
    }

    /// 同型の全インスタンスを返す
    public static T[] GetAll<T>(ulong ownerId) where T : Component
    {
        var ids = NativeMethods.GameObject_GetComponentIdsHelper(ownerId, typeof(T).Name);
        return [.. ids.Select(id => (T)GetOrCreate(ownerId, id, typeof(T)))];
    }

    /// <summary>
    /// コンポーネントをキャッシュに登録します。
    /// </summary>
    /// <param name="component"> 登録するコンポーネント</param>
    /// <exception cref="ArgumentException"> component.objectId が 0 の場合にスローされます。</exception>
    internal static void Register(Component component)
    {
        if (component.objectId == 0)
            throw new ArgumentException("Component ID must not be zero.", nameof(component));

        s_cache[component.objectId] = component;
    }

    /// <summary>
    /// キャッシュからコンポーネントを削除します。
    /// </summary>
    /// <param name="component"> 削除するコンポーネント</param>
    internal static void Remove(Component component)
    {
        if (s_cache.TryGetValue(component.objectId, out var cached) &&
            ReferenceEquals(cached, component))
        {
            s_cache.Remove(component.objectId);
        }
    }

    /// <summary>
    /// キャッシュからコンポーネントを取得するか、存在しない場合は新しいインスタンスを作成して返します。
    /// </summary>
    /// <param name="ownerId"> 所属するGameObjectのID</param>
    /// <param name="componentId"> コンポーネントのID</param>
    /// <param name="type"> コンポーネントの型</param>
    /// <returns> キャッシュされたコンポーネント、または新しく作成されたコンポーネント</returns>
    internal static Component GetOrCreate(ulong ownerId, ulong componentId, Type type)
    {
        if (!s_cache.TryGetValue(componentId, out var component))
        {
            // CreateInstanceを呼び出して新しいインスタンスを作成し、キャッシュに追加
            component = CreateInstance(type, ownerId, componentId);
            s_cache[componentId] = component;
        }
        return component;
    }

    internal static Component CreateInstance(Type type, ulong ownerId, ulong componentId)
    {
        var component = Activator.CreateInstance(type) as Component
            ?? throw new InvalidOperationException($"Failed to create instance of {type.Name}");
        component.Setup(ownerId, componentId);
        return component;
    }

    internal static void Replace(Component previous, Component replacement)
    {
        if (previous.objectId != replacement.objectId)
            throw new InvalidOperationException("Cannot replace components with different IDs.");

        s_cache[replacement.objectId] = replacement;
    }
}
