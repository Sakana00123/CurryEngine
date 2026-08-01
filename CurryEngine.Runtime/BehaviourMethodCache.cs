using CurryEngine.Interfaces;
using System.Reflection;

namespace CurryEngine.Runtime;

internal static class BehaviourMethodCache
{
    internal record MethodSet(
        MethodInfo? Awake,
        MethodInfo? Start,
        MethodInfo? Update,
        MethodInfo? OnEnable,
        MethodInfo? OnDisable,
        MethodInfo? OnDestroy,
        MethodInfo? OnCollisionEnter,
        MethodInfo? OnCollisionStay,
        MethodInfo? OnCollisionExit,
        MethodInfo? OnTriggerEnter,
        MethodInfo? OnTriggerStay,
        MethodInfo? OnTriggerExit
    );

    private static readonly Dictionary<Type, MethodSet> s_cache = new();

    /// <summary>
    /// 指定された型のコールバックメソッドをキャッシュから取得する。キャッシュに存在しない場合はリフレクションで取得してキャッシュに保存する。
    /// </summary>
    /// <param name="type">コールバックメソッドを取得する型</param>
    /// <returns>コールバックメソッドのセット</returns>
    internal static MethodSet Get(Type type)
    {
        if (s_cache.TryGetValue(type, out var set)) return set;

        set = new MethodSet(
            Lookup(type, nameof(Behaviour.Awake)),
            Lookup(type, nameof(Behaviour.Start)),
            Lookup(type, nameof(Behaviour.Update)),
            Lookup(type, nameof(Behaviour.OnEnable)),
            Lookup(type, nameof(Behaviour.OnDisable)),
            Lookup(type, nameof(Behaviour.OnDestroy)),
            Lookup(type, nameof(Behaviour.OnCollisionEnter)),
            Lookup(type, nameof(Behaviour.OnCollisionStay)),
            Lookup(type, nameof(Behaviour.OnCollisionExit)),
            Lookup(type, nameof(Behaviour.OnTriggerEnter)),
            Lookup(type, nameof(Behaviour.OnTriggerStay)),
            Lookup(type, nameof(Behaviour.OnTriggerExit))
        );
        s_cache[type] = set;
        return set;
    }

    /// <summary>
    /// ホットリロード時にキャッシュをクリアする。次回Get呼び出し時に新しいMethodInfoを取得するため。
    /// </summary>
    internal static void Clear() => s_cache.Clear();

    private static MethodInfo? Lookup(Type t, string name)
        => t.GetMethod(name,
            BindingFlags.Instance |
            BindingFlags.Public |
            BindingFlags.NonPublic);
}
