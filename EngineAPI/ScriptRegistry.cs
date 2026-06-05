// EngineAPI/ScriptRegistry.cs
using CurryEngine.Components;
using CurryEngine.Interop;
using System.Reflection;

namespace CurryEngine;

/// <summary>
/// Behaviour サブクラスを名前で解決するレジストリ。
/// ・起動時 → RegisterAssembly でエンジンAPIをスキャン
/// ・ホットリロード時 → Refresh で差し替え
/// </summary>
public static class ScriptRegistry
{
    // 型名 → Behaviourコンストラクタ のキャッシュ
    private static readonly Dictionary<string, Func<object>> s_factories = new();
    private static readonly Dictionary<string, Type> s_types = new();

    // ---- 登録 ----

    /// <summary>
    /// アセンブリ内の全 Behaviour サブクラスを登録する。
    /// 起動時・ホットリロード時に呼ぶ。
    /// </summary>
    public static void RegisterAssembly(Assembly assembly)
    {
        //var behaviourType = typeof(Behaviour);

        // C++側に登録されてるスクリプト名リストをクリアさせる。
        NativeMethods.ScriptNames_Clear();

        // アセンブリ内の全型を走査
        foreach (var type in assembly.GetTypes())
        {
            // abstract は除外、Behaviour の具象サブクラスのみ
            if (type.IsAbstract) continue;
            //if (!behaviourType.IsAssignableFrom(type)) continue;

            // IsAssignableFrom の代わりに継承チェーンを名前で確認
            if (!InheritsFromBehaviour(type)) continue;

            RegisterType(type);
            NativeMethods.ScriptNames_Add(type.Name);   // C++側にも型名を登録
            Debug.Log($"登録完了: {type.Name}");
        }
    }

    /// <summary>型を個別登録する。テスト用途など。</summary>
    public static void RegisterType(Type type)
    {
        var ctor = type.GetConstructor(
            BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance,
            binder: null,
            types: Type.EmptyTypes,   // 引数なしコンストラクタ
            modifiers: null)
            ?? throw new InvalidOperationException(
                $"{type.Name} has no parameterless constructor");

        s_factories[type.Name] = () => ctor.Invoke(null);
        s_types[type.Name] = type;
        //s_factories[type.Name] = () => (Component)ctor.Invoke(null);
    }

    // ---- 解決 ----

    /// <summary>
    /// 型名から Behaviour インスタンスを生成して返す。
    /// 見つからなければ null。
    /// </summary>
    //public static Behaviour? Create(string typeName, ulong entityId)
    //{
    //    if (!s_factories.TryGetValue(typeName, out var factory))
    //        return null;

    //    var behaviour = factory();
    //    behaviour.entityId = entityId;
    //    return behaviour;
    //}

    public static object? CreateRaw(string typeName, ulong ownerId, ulong componentId)
    {
        if (!s_factories.TryGetValue(typeName, out var factory))
            return null;

        var instance = factory();

        // リフレクションで Component.Setup(ownerId, componentId) を呼ぶ
        var setupMethod = instance.GetType().GetMethod("Setup", BindingFlags.NonPublic | BindingFlags.Instance)
            ?? throw new InvalidOperationException($"{typeName} に SetUp メソッドが見つかりません");

        setupMethod.Invoke(instance, new object[] { ownerId, componentId });

        return instance;
    }


    /// <summary>型名に対応する Type を返す。存在確認用。</summary>
    public static Type? Resolve(string typeName)
    {
        //if (!s_factories.TryGetValue(typeName, out var factory))
        //    return null;

        //// ファクトリから型を逆引き (インスタンス生成せずに型だけ欲しい場合)
        //return factory().GetType();
        return s_types.TryGetValue(typeName, out var type) ? type : null;
    }

    public static bool IsRegistered(string typeName)
        => s_factories.ContainsKey(typeName);

    // ---- ホットリロード ----

    /// <summary>
    /// 新しいアセンブリで登録内容を差し替える。
    /// 旧エントリは削除し、新アセンブリのクラスで上書き。
    /// </summary>
    public static void Refresh(Assembly newAssembly)
    {
        // 新アセンブリ由来の型名を収集
        var newTypes = newAssembly.GetTypes()
            .Where(t => !t.IsAbstract && InheritsFromBehaviour(t))
            .Select(t => t.Name);

        // 旧エントリのうち新アセンブリで再定義されたものだけ削除
        foreach (var name in newTypes)
            s_factories.Remove(name);

        // C++側のキャッシュもクリア
        BehaviourMethodCache.Clear();

        // 新アセンブリを登録
        RegisterAssembly(newAssembly);
    }

    // ---- デバッグ ----

    public static IEnumerable<string> RegisteredNames
        => s_factories.Keys;


    // ---- ユーティリティ ----
    private static bool InheritsFromBehaviour(Type type)
    {
        var current = type.BaseType;
        while (current != null)
        {
            if (current.FullName == "CurryEngine.Behaviour")
                return true;
            current = current.BaseType;
        }
        return false;
    }
}