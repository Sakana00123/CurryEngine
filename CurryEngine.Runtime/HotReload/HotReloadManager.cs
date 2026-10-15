
using System.Reflection;
using System.Runtime.Loader;

namespace CurryEngine.Runtime.HotReload;

public sealed class HotReloadManager : IDisposable
{
    private ScriptLoadContext? m_current;
    private readonly string m_dllPath;

    public HotReloadManager(string dllPath) { m_dllPath = dllPath; }

    /// <summary>
    /// DLLをロードして、ScriptRegistry を更新する。
    /// </summary>
    /// <returns>ロードされたアセンブリ</returns>
    public Assembly Load()
    {
        // Read the original output into memory so it can be rebuilt without locking it.
        // Dependency resolution remains relative to the project's output directory.
        var next = new ScriptLoadContext(m_dllPath);
        Assembly asm;
        try
        {
            using var dll = new MemoryStream(File.ReadAllBytes(m_dllPath));
            var pdbPath = Path.ChangeExtension(m_dllPath, ".pdb");
            using var pdb = File.Exists(pdbPath) ? new MemoryStream(File.ReadAllBytes(pdbPath)) : null;
            asm = next.LoadFromStream(dll, pdb);
            _ = asm.GetTypes(); // Validate before discarding the last successful assembly.
        }
        catch
        {
            next.Unload();
            throw;
        }
        try { ScriptRegistry.Refresh(asm); }
        catch { next.Unload(); throw; }
        Dispose();
        m_current = next;

        return asm;
    }

    /// <summary>
    /// 既存インスタンスを新型で差し替え。
    /// public フィールドとプロパティを引き継ぐ。
    /// </summary>
    public object? HotSwap(object old, ulong ownerId, ulong componentId)
    {
        var newTypeName = old.GetType().Name;
        if (!ScriptRegistry.IsRegistered(newTypeName)) return old;

        var next = ScriptRegistry.CreateRaw(newTypeName, ownerId, componentId);
        if (next == null) return old;

        // フィールド値の移植
        MigrateFields(old, next);
        return next;
    }

    /// <summary>
    /// 古いインスタンスの public フィールド値を新しいインスタンスにコピーする。
    /// </summary>
    /// <param name="src">コピー元の古いインスタンス</param>
    /// <param name="dst">コピー先の新しいインスタンス</param>
    private static void MigrateFields(object src, object dst)
    {
        var srcFields = src.GetType()
            .GetFields(BindingFlags.Public |
                       BindingFlags.Instance);

        foreach (var sf in srcFields)
        {
            var df = dst.GetType().GetField(sf.Name,
                         BindingFlags.Public |
                         BindingFlags.Instance);
            if (df != null && df.FieldType == sf.FieldType)
                df.SetValue(dst, sf.GetValue(src));
        }
    }

    /// <summary>
    /// 古いコンテキストのアンロードを要求する。
    /// </summary>
    public void Dispose()
    {
        m_current?.Unload();
        m_current = null;
    }
}
