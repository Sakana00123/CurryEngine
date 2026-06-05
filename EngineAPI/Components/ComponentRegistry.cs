namespace CurryEngine.Components;

/// <summary>
/// C#型 ↔ C++コンポーネント名のマッピング。
/// C++側のマクロ登録名と文字列を一致させる。
/// </summary>
public static class ComponentRegistry
{
    private static readonly Dictionary<Type, string> s_typeToName = new();
    private static readonly Dictionary<string, Type> s_nameToType = new();

    // ---- 登録 ----

    /// <summary>
    /// エンジン起動時に呼ぶ。
    /// C++側の REGISTER_COMPONENT("Transform", ...) と名前を合わせる。
    /// </summary>
    public static void Register<T>(string nativeName) where T : Component
        => Register(typeof(T), nativeName);

    public static void Register(Type type, string nativeName)
    {
        s_typeToName[type] = nativeName;
        s_nameToType[nativeName] = type;
    }

    // ---- 検索 ----

    public static string GetNativeName(Type type)
        => s_typeToName.TryGetValue(type, out var name) ? name
           : throw new KeyNotFoundException(
               $"Unregistered component type: {type.Name}");

    public static Type? FindType(string nativeName)
        => s_nameToType.TryGetValue(nativeName, out var type) ? type : null;

    public static bool IsRegistered(Type type)
        => s_typeToName.ContainsKey(type);
}