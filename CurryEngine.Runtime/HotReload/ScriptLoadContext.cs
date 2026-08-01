using System.Reflection;
using System.Runtime.Loader;

namespace CurryEngine.Runtime.HotReload;

/// <summary>
/// アンロード可能なコンテキスト。
/// 旧バージョンと新バージョンを同時に保持できる。
/// </summary>
sealed class ScriptLoadContext : AssemblyLoadContext
{
    //private readonly AssemblyDependencyResolver m_resolver;
    private readonly string m_dir;

    public ScriptLoadContext(string dllPath)
        : base(isCollectible: true)
    {
        //m_resolver = new AssemblyDependencyResolver(dllPath);
        // DLLのあるディレクトリを保持して、Load でそこから読み込む(ロック回避のため)
        m_dir = Path.GetDirectoryName(dllPath)!;

        // Resolvingイベントで、依存アセンブリのロードをカスタマイズする
        Resolving += OnResolving;
    }

    private Assembly? OnResolving(AssemblyLoadContext context, AssemblyName name)
    {
        // CurryEngine.API と Runtime は親ALCのものを使う
        if (name.Name == "CurryEngine.API")
            return GetLoadContext(typeof(Component).Assembly)!
                .LoadFromAssemblyName(name);
        if (name.Name == "CurryEngine.Runtime") return null;
        if (name.Name?.StartsWith("System.") == true) return null;
        if (name.Name?.StartsWith("Microsoft.") == true) return null;

        var path = Path.Combine(m_dir, $"{name.Name}.dll");
        return File.Exists(path) ? LoadFromAssemblyPath(path) : null;
    }


    protected override Assembly? Load(AssemblyName name)
    {
        return null; // デフォルトのローディングに任せる
    }
}
