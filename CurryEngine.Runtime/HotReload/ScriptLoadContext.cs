using CurryEngine.Scripting;
using System.Reflection;
using System.Runtime.Loader;

namespace CurryEngine.Runtime.HotReload;

sealed class ScriptLoadContext : AssemblyLoadContext
{
    private readonly string m_dir;

    public ScriptLoadContext(string dllPath)
        : base(isCollectible: true)
    {
        // DLLのあるディレクトリを保持して、Load でそこから読み込む(ロック回避のため)
        m_dir = Path.GetDirectoryName(dllPath)!;

    }

    protected override Assembly? Load(AssemblyName name)
    {
        var shared = SharedAssemblyRegistry.TryGet(name.Name!);
        if (shared != null)
        {
            Debug.Log($"[ScriptLoadContext] SharedAssemblyRegistry からロード: {name.Name}");
            return shared;
        }

        // System系アセンブリは、Default ALCと共有する
        if (IsSharedAssembly(name.Name))
        {
            Debug.Log($"[ScriptLoadContext] Default ALC からロード: {name.Name}");
            return null;
        }

        // それ以外のアセンブリは、DLLのあるディレクトリからロードする
        var path = Path.Combine(m_dir, $"{name.Name}.dll");
        if (File.Exists(path))
        {
            Debug.Log($"[ScriptLoadContext] {path} からロード");
            return LoadFromAssemblyPath(path);
        }

        // 見つからなかった場合は null を返すと、親ALCでのロードが試みられる
        Debug.Log($"[ScriptLoadContext] {name.Name} は見つからず、親ALCに委譲");
        return null;
    }

    /// <summary>
    /// Default ALC (親) と共有すべきアセンブリかどうかを判定する
    /// </summary>
    private static bool IsSharedAssembly(string? assemblyName)
    {
        if (string.IsNullOrEmpty(assemblyName)) return false;

        // システム系アセンブリも親と共有
        if (assemblyName.StartsWith("System.") ||
            assemblyName.StartsWith("Microsoft.") ||
            assemblyName == "netstandard" ||
            assemblyName == "mscorlib")
        {
            return true;
        }
        return false;
    }
}
