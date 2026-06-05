// EngineAPI/HotReload/HotReloadManager.cs
using CurryEngine.Interop;
using System.Reflection;
using System.Runtime.Loader;

namespace CurryEngine.HotReload;

public sealed class HotReloadManager : IDisposable
{
    private ScriptLoadContext? m_current;
    private string? m_currentTempPath;
    private readonly string m_dllPath;

    // C++から呼ばれる: スクリプトインスタンス一覧
    public event Action<Type>? OnTypeReloaded;

    public HotReloadManager(string dllPath) { m_dllPath = dllPath; }

    /// <summary>初回ロード、またはリロード時に呼ぶ</summary>
    public Assembly Load(AssemblyLoadContext context)
    {
        // 古いコンテキストをアンロード
        Dispose();

        // DLLを一時ファイルにコピー（ロック回避のため）
        m_currentTempPath = Path.GetTempFileName();
        File.Copy(m_dllPath, m_currentTempPath, true);

        // 新しいコンテキストを作成
        m_current = new ScriptLoadContext(m_currentTempPath);

        // ファイルをバイト配列として読み込む（ロック回避のため）
        byte[] dllBytes = File.ReadAllBytes(m_dllPath);
        using var stream = new MemoryStream(dllBytes);
        var asm = m_current.LoadFromStream(stream);

        //var asm = context.LoadFromAssemblyPath(m_dllPath);

        // ScriptRegistry を更新
        ScriptRegistry.Refresh(asm);

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

    public void Dispose()
    {
        m_current?.Unload();
        m_current = null;

        // 一時ファイルを削除
        if (m_currentTempPath != null && File.Exists(m_currentTempPath))
        {
            try { File.Delete(m_currentTempPath); }
            catch { /* 失敗しても無視 */ }
        }
    }
}