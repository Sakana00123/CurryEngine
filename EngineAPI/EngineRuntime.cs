using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using CurryEngine.HotReload;
using CurryEngine.Interop;

namespace CurryEngine;

public static class EngineRuntime
{
    private static HotReloadManager? s_hotReloadManager;
    internal static HotReloadManager? HotReload => s_hotReloadManager;

    internal static string? GetExecutableDirectory()
    {
        var exePath = Environment.ProcessPath;
        var exeDir = Path.GetDirectoryName(exePath);
        if (exeDir == Environment.CurrentDirectory)
        {
            // カレントディレクトリと同じ場合は、x64/$(Configuration) を返すようにする。
            var config = Environment.GetEnvironmentVariable("CONFIGURATION") ?? "Debug";
            exeDir = Path.Combine(exeDir, "x64", config);
            Debug.Log($"GetExecutableDirectory: カレントディレクトリと同じため、x64/{config} を返す: {exeDir}");
        }
        Debug.Log($"GetExecutableDirectory: {exeDir}");
        return exeDir;
    }

    internal static string? GetUserScriptsPath()
    {
        var solutionDir = Environment.GetEnvironmentVariable("SOLUTION_DIR");
        if (string.IsNullOrEmpty(solutionDir))
        {
            Debug.LogError("SOLUTION_DIR 環境変数が設定されていません。");
            return null;
        }
        // Assembly-CSharp.dll のパスを組み立てる。($(SolutionDir)Library\ScriptAssemblies\Assembly-CSharp.dll)
        var userScriptsPath = Path.Combine(solutionDir, "Library", "ScriptAssemblies", "Assembly-CSharp.dll");
        //var userScriptsPath = Path.Combine(solutionDir, "UserScripts", "x64", "Debug", "Assembly-CSharp.dll");
        if (!File.Exists(userScriptsPath))
        {
            Debug.LogError($"Assembly-CSharp.dll が見つかりません: {userScriptsPath}");
            return null;
        }
        return userScriptsPath;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void EngineInitialize()
    {
        try
        {
            // エンジンの初期化処理をここに書く。
            //Debug.Log("EngineInitialize 開始");

            Component.Accessor = new ComponentAccessorAdapter();
            // 注入後に確認
            var asm = typeof(Component).Assembly;
            var alc = AssemblyLoadContext.GetLoadContext(asm);
            //File.AppendAllText("debug.txt", $"EngineInitialize called\n");
            //File.AppendAllText("debug.txt", $"Accessor注入: {Component.Accessor != null}\n");
            //File.AppendAllText("debug.txt", $"AssemblyハッシュID: {RuntimeHelpers.GetHashCode(typeof(Component).Assembly)}\n");
            //File.AppendAllText("debug.txt", $"ALC名: {alc?.Name}\n\n");


            // EngineAPI.dll 内の全ての Behaviour 派生クラスを ScriptRegistry に登録する。
            ScriptRegistry.RegisterAssembly(typeof(Behaviour).Assembly);

            // UserScripts.dll をロードして ScriptRegistry に登録する。
            var exeDir = GetExecutableDirectory() ?? string.Empty;
            var userScriptsPath = /*GetUserScriptsPath() ?? */Path.Combine(exeDir, "Assembly-CSharp.dll");
            //Debug.Log($"Assembly-CSharp.dll のパス: {userScriptsPath}");

            // EngineRuntime と同じ ALC を使ってロードする。通常は EngineRuntime と UserScripts は同じ ALC にロードされるはずだが、これで確実になる。
            var currentAlc = AssemblyLoadContext.GetLoadContext(typeof(EngineRuntime).Assembly)!;

            // HotReloadManager で初回ロードする。これにより、後でリロードも可能になる。
            if (File.Exists(userScriptsPath))
            {
                s_hotReloadManager = new HotReloadManager(userScriptsPath);
                s_hotReloadManager.Load(currentAlc);

                // 初期化完了ログ
                Debug.Log("EngineInitialize 完了");
                foreach (var typeName in ScriptRegistry.RegisteredNames)
                    Debug.Log($"登録されたスクリプト: {typeName}");
            }
            else
            {
                Debug.LogError($"Assembly-CSharp.dll が見つかりません: {userScriptsPath}");
            }
        }
        catch (Exception ex)
        {
            // 例外が発生した場合はログに出力する。
            Debug.LogError($"EngineInitialize 例外: {ex.Message}");
            if (ex.InnerException != null)
            {
                Debug.LogError($"内部例外: {ex.InnerException.Message}");
            }
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static unsafe void ReloadScripts(byte* dllPathUtf8)
    {
        try
        {
            var dllPath = Marshal.PtrToStringUTF8((nint)dllPathUtf8);
            if (string.IsNullOrEmpty(dllPath))
            {
                // パスが無効な場合はデフォルトのパスを使用する。
                var exeDir = GetExecutableDirectory() ?? string.Empty;
                dllPath = /*GetUserScriptsPath() ?? */Path.Combine(exeDir, "Assembly-CSharp.dll");
            }
            //Debug.Log($"ReloadScripts called with path: {dllPath}");
            var currentAlc = AssemblyLoadContext.GetLoadContext(typeof(EngineRuntime).Assembly)!;
            // HotReloadManager がまだ作られていない場合は作る。通常は EngineInitialize で作られているはず。
            if (s_hotReloadManager == null)
            {
                s_hotReloadManager = new HotReloadManager(dllPath);
            }

            // HotReloadManager でリロードする。これにより、ScriptRegistry も更新される。
            s_hotReloadManager.Load(currentAlc);

            Debug.Log("ReloadScripts 完了");
            foreach (var typeName in ScriptRegistry.RegisteredNames)
                Debug.Log($"登録されたスクリプト: {typeName}");
        }
        catch (Exception ex)
        {
            Debug.LogError($"ReloadScripts 例外: {ex.Message}");
            if (ex.InnerException != null)
            {
                Debug.LogError($"内部例外: {ex.InnerException.Message}");
            }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ScriptPropertyDesc
    {
        public IntPtr name; // const char* -> UTF-8 文字列へのポインタ
        public IntPtr typeName; // const char* -> UTF-8 文字列へのポインタ
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct ScriptClassDesc
    {
        public IntPtr name; // const char* -> UTF-8 文字列へのポインタ
        public IntPtr baseClass; // const char* -> UTF-8 文字列へのポインタ (基底クラスがない場合は null)
        public IntPtr properties; // ScriptPropertyDesc* -> プロパティ配列へのポインタ
        public int propertyCount; // int -> プロパティの数
    }

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate void RegisterScriptMetaCallback(ref ScriptClassDesc classDesc);


    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static unsafe void RegisterAllScriptMeta(IntPtr callbackPtr)
    {
        var callback = Marshal.GetDelegateForFunctionPointer<RegisterScriptMetaCallback>(callbackPtr);

        foreach (var scriptName in ScriptRegistry.RegisteredNames)
        {
            Type scriptType = ScriptRegistry.Resolve(scriptName) ?? throw new InvalidOperationException($"Script type not found: {scriptName}");

            var fields = scriptType.GetFields(BindingFlags.Public | BindingFlags.Instance);

            // ネイティブメモリに確保してGCの影響を受けないようにする
            var propDescs = new ScriptPropertyDesc[fields.Length];
            var nameHandles = new List<GCHandle>();

            try
            {
                for (int i = 0; i < fields.Length; i++)
                {
                    // 文字列をネイティブメモリに固定
                    var namePtr = Marshal.StringToHGlobalAnsi(fields[i].Name);
                    var typePtr = Marshal.StringToHGlobalAnsi(fields[i].FieldType.Name);
                    nameHandles.Add(GCHandle.Alloc(namePtr));  // ← ピン留め
                    propDescs[i] = new ScriptPropertyDesc { name = namePtr, typeName = typePtr };
                }

                unsafe
                {
                    fixed (ScriptPropertyDesc* propDescsPtr = propDescs)
                    {
                        var classDesc = new ScriptClassDesc
                        {
                            name = Marshal.StringToHGlobalAnsi(scriptName),
                            baseClass = IntPtr.Zero, // 基底クラスの情報が必要ならここで設定
                            properties = (IntPtr)propDescsPtr,
                            propertyCount = fields.Length
                        };
                        callback(ref classDesc);
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.LogError($"RegisterAllScriptMeta 例外: {ex.Message}");
                continue;
            }
            finally
            {
                // 確保したネイティブメモリを解放
                foreach (var handle in nameHandles)
                {
                    if (handle.IsAllocated)
                    {
                        Marshal.FreeHGlobal(handle.AddrOfPinnedObject());
                        handle.Free();
                    }
                }
            }
        }

    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void EngineUpdate(float deltaTime)
    {
        // エンジンの更新処理をここに書く。
        try
        {
            //Debug.Log($"EngineUpdate called with deltaTime: {deltaTime}");
            // ここでスクリプトの Update メソッドを呼び出すなどの処理を行う。
        }
        catch (Exception ex)
        {
            Debug.LogError($"EngineUpdate 例外: {ex.Message}");
        }
    }



    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void EngineShutdown()
    {
        // エンジンの終了処理をここに書く。
        try
        {
            s_hotReloadManager?.Dispose();
            s_hotReloadManager = null;
            Debug.Log("EngineShutdown 完了");
        }
        catch (Exception ex)
        {
            Debug.LogError($"EngineShutdown 例外: {ex.Message}");
        }
    }

}
