using CurryEngine.Components;
using CurryEngine.Interop;

namespace CurryEngine;

internal static class ComponentAccess
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
            return null;
        return (T)GetOrCreate(ownerId, ids[0], typeof(T));
    }

    /// 同型の全インスタンスを返す
    public static T[] GetAll<T>(ulong ownerId) where T : Component
    {
        var ids = NativeMethods.GameObject_GetComponentIdsHelper(ownerId, typeof(T).Name);
        return [.. ids.Select(id => (T)GetOrCreate(ownerId, id, typeof(T)))];
    }

    public static Transform GetTransform(ulong ownerId)
    {
        var ids = NativeMethods.GameObject_GetComponentIdsHelper(ownerId, "Transform");
        if (ids.Length == 0)
            throw new InvalidOperationException($"Entity {ownerId} has no Transform");
        return (Transform)GetOrCreate(ownerId, ids[0], typeof(Transform));
    }

    /// コンポーネント単体の破棄（Detach時など）
    internal static void RemoveComponent(ulong componentId)
        => s_cache.Remove(componentId);

    // ---- 内部実装 ----

    private static Component GetOrCreate(ulong ownerId, ulong componentId, Type type)
    {
        //if (!s_cache.TryGetValue(componentId, out var component))
        //{
        //    component = CreateInstance(type, ownerId, componentId);
        //    s_cache[componentId] = component;
        //}
        var component = CreateInstance(type, ownerId, componentId);

        return component;
    }

    internal static Component CreateInstance(Type type, ulong ownerId, ulong componentId)
    {
        //if (!s_factories.TryGetValue(type, out var factory))
        //{
        //    var ctor = type.GetConstructor(
        //        System.Reflection.BindingFlags.NonPublic |
        //        System.Reflection.BindingFlags.Instance,
        //        [typeof(ulong)])
        //        ?? throw new InvalidOperationException(
        //            $"{type.Name} has no internal ctor(ulong)");

        //    var setup = type.GetMethod(
        //        "Setup",
        //        System.Reflection.BindingFlags.NonPublic |
        //        System.Reflection.BindingFlags.Instance,
        //        [typeof(ulong), typeof(ulong)]);

        //    factory = id => (Component)ctor.Invoke([id]);
        //    s_factories[type] = factory;
        //}
        var component = Activator.CreateInstance(type) as Component
            ?? throw new InvalidOperationException($"Failed to create instance of {type.Name}");
        component.Setup(ownerId, componentId);
        return component;
    }
}
