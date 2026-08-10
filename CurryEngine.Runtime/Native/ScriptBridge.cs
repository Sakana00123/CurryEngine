using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using CurryEngine.Runtime.Reflection;

namespace CurryEngine.Runtime.Native;

/// <summary>
/// C++から直接呼ばれるエントリポイント群。
/// UnmanagedCallersOnly = vtable経由ゼロオーバーヘッド。
/// </summary>
public static unsafe class ScriptBridge
{
    // GCHandle管理テーブル — C++にはintptr_tとして渡す
    private static readonly Dictionary<nint, GCHandle> s_handles = new();

    // ---- ライフサイクル ----

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void* CreateScript(byte* typeNameUtf8, ulong ownerId, ulong componentId)
    {
        Debug.Log($"CreateScript: typeNameUtf8={Marshal.PtrToStringUTF8((nint)typeNameUtf8)} ownerId={ownerId} componentId={componentId}");
        try
        {
            var typeName = Marshal.PtrToStringUTF8((nint)typeNameUtf8)!;

            // まずはScriptRegistryに生のobjectを作らせる。これならBehaviour継承してなくてもOK。
            var instance = ScriptRegistry.CreateRaw(typeName, ownerId, componentId);
            if (instance is not Component component)
            {
                Debug.LogError($"CreateScript: {typeName} is not a Component.");
                return null;
            }

            // ComponentCacheに登録しておく。
            ComponentCache.Register(component);

            // Behaviour にキャストせず object として GCHandle に入れる
            var handle = GCHandle.Alloc(instance);
            var ptr = GCHandle.ToIntPtr(handle);
            s_handles[ptr] = handle;

            Debug.Log($"CreateScript: 成功");
            return (void*)ptr;
        }
        catch (Exception ex)
        {
            Debug.LogError($"CreateScript 例外: {ex.Message}");
            return null;
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void ReleaseScript(void* gcHandle)
    {
        var ptr = (nint)gcHandle;
        if (!s_handles.Remove(ptr, out var handle))
            return;

        try
        {
            if (handle.Target is Component component)
            {
                // キャッシュから削除
                ComponentCache.Remove(component);
            }
        }
        catch (Exception ex)
        {
            Debug.LogError($"ReleaseScript 例外: {ex.Message}, StackTrace: {ex.StackTrace}, InnerException: {ex.InnerException}");
        }
        finally
        {
            handle.Free();
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void AwakeScript(void* gcHandle)
    {
        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            methods.Awake?.Invoke(instance, null);
        }
        catch (Exception ex)
        {
            Debug.LogError($"AwakeScript 例外: {ex.Message}");
            Debug.LogError($"スタックトレース: {ex.StackTrace}");
            Debug.LogError($"InnerException: {ex.InnerException}");
            Debug.LogError($"Source: {ex.Source}");
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void StartScript(void* gcHandle)
    {
        var asm = typeof(Component).Assembly;
        var hash = RuntimeHelpers.GetHashCode(asm);
        var alc = System.Runtime.Loader.AssemblyLoadContext.GetLoadContext(asm);
        //File.AppendAllText("debug.txt", $"AssemblyハッシュID: {hash}\n");
        //File.AppendAllText("debug.txt", $"ALC名: {alc?.Name}\n");
        //File.AppendAllText("debug.txt", $"StartScript Accessor: {Component.Accessor != null}\n\n");

        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            methods.Start?.Invoke(instance, null);
        }
        catch (Exception ex)
        {
            Debug.LogError($"StartScript 例外: {ex.Message}");
            Debug.LogError($"StartScript InnerException: {ex.InnerException?.Message}");
            Debug.LogError($"StartScript StackTrace: {ex.StackTrace}");
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void UpdateScript(void* gcHandle)
    {
        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            methods.Update?.Invoke(instance, null);
        }
        catch (Exception ex)
        {
            Debug.LogError($"UpdateScript 例外: {ex.Message}\n\nスタックトレース: {ex.StackTrace}\n\nInnerException: {ex.InnerException}\n\nHelpLink: {ex.HelpLink}");
        }
    }
    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void OnEnableScript(void* gcHandle)
    {
        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            methods.OnEnable?.Invoke(instance, null);
        }
        catch (Exception ex)
        {
            Debug.LogError($"EnableScript 例外: {ex.Message}");
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void OnDisableScript(void* gcHandle)
    {
        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            methods.OnDisable?.Invoke(instance, null);
        }
        catch (Exception ex)
        {
            Debug.LogError($"DisableScript 例外: {ex.Message}");
        }
    }


    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void OnDestroyScript(void* gcHandle)
    {
        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            methods.OnDestroy?.Invoke(instance, null);

            // ここでGCHandleを解放してもいいが、C++側でReleaseScript呼ぶ前提ならそちらに任せる
        }
        catch (Exception ex)
        {
            Debug.LogError($"DestroyScript 例外: {ex.Message}");
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void* HotSwapScript(void* gcHandle, ulong ownerId, ulong componentId)
    {
        try
        {
            var old = GCHandle.FromIntPtr((nint)gcHandle).Target!;
            var manager = EngineRuntime.HotReload;
            if (manager == null) return gcHandle;

            var next = manager.HotSwap(old, ownerId, componentId);
            if (next == null || ReferenceEquals(next, old)) return gcHandle;

            // 旧ハンドルを解放
            var oldPtr = (nint)gcHandle;
            if (s_handles.Remove(oldPtr, out var oldHandle))
                oldHandle.Free();

            // 新ハンドルを登録
            var newHandle = GCHandle.Alloc(next);
            var newPtr = GCHandle.ToIntPtr(newHandle);
            s_handles[newPtr] = newHandle;

            return (void*)newPtr;
        }
        catch (Exception ex)
        {
            Debug.LogError($"ReloadScript 例外: {ex.Message}");
            return gcHandle;
        }
    }

    // フィールドアクセス。必要に応じてプロパティやメソッド呼び出しも追加できる。
    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static nint GetScriptFields(void* gcHandle)
    {
        try
        {
            // スクリプトオブジェクトを取得
            var obj = Unwrap(gcHandle);
            // ScriptInspectorを使ってフィールド情報をJSON化して返す。C++側でパースして使う想定。
            string json = ScriptInspector.GetFieldsJson(obj);
            return Marshal.StringToCoTaskMemUTF8(json);
        }
        catch (Exception ex)
        {
            Debug.LogError($"GetScriptField 例外: {ex.Message}");
            return nint.Zero;
        }
    }

    // フィールドに値をセットする。valueHandleはC++側でGCHandleから渡す想定。
    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void SetScriptField(void* gcHandle, byte* fieldNameUtf8, byte* valueJson)
    {
        try
        {
            // スクリプトオブジェクトを取得
            var obj = Unwrap(gcHandle);
            // フィールド名からFieldInfoを取得
            var fieldName = Marshal.PtrToStringUTF8((nint)fieldNameUtf8)!;
            // JSON文字列をC#の値に変換してセットする。ScriptInspector側で型に応じて適切に変換される想定。
            var value = Marshal.PtrToStringUTF8((nint)valueJson);
            if (value == null || value == "null")
            {
                Debug.LogError($"SetScriptField: valueJson is null for fieldName={fieldName}");
                return;
            }
            ScriptInspector.SetFieldValue(obj, fieldName, value);
            
        }
        catch (Exception ex)
        {
            Debug.LogError($"SetScriptField 例外: {ex.Message}");
        }
    }


    // ------------------------ Physicsイベントなどの追加コールバックもここに実装していく ------------------------

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void OnCollisionEnterScript(void* gcHandle, CollisionInfoDto* dto)
    {
        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            if (methods.OnCollisionEnter == null) return;

            var collision = CollisionConverter.ToCollision(dto);
            var parameters = new object[] { collision };
            methods.OnCollisionEnter.Invoke(instance, parameters);
        }
        catch (Exception ex)
        {
            Debug.LogError($"OnCollisionEnterScript 例外: {ex.Message}");
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void OnCollisionStayScript(void* gcHandle, CollisionInfoDto* dto)
    {
        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            if (methods.OnCollisionStay == null) return;

            var collision = CollisionConverter.ToCollision(dto);
            var parameters = new object[] { collision };
            methods.OnCollisionStay.Invoke(instance, parameters);
        }
        catch (Exception ex)
        {
            Debug.LogError($"OnCollisionStayScript 例外: {ex.Message}");
            Debug.LogError($"スタックトレース: {ex.StackTrace}");
            Debug.LogError($"InnerException: {ex.InnerException}");
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void OnCollisionExitScript(void* gcHandle, CollisionInfoDto* dto)
    {
        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            if (methods.OnCollisionExit == null) return;

            var collision = CollisionConverter.ToCollision(dto);
            var parameters = new object[] { collision };
            methods.OnCollisionExit.Invoke(instance, parameters);
        }
        catch (Exception ex)
        {
            Debug.LogError($"OnCollisionExitScript 例外: {ex.Message}");
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void OnTriggerEnterScript(void* gcHandle, TriggerInfoDto* dto)
    {
        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            if (methods.OnTriggerEnter == null) return;

            var trigger = CollisionConverter.ToTrigger(dto);
            var parameters = new object[] { trigger };
            methods.OnTriggerEnter.Invoke(instance, parameters);
        }
        catch (Exception ex)
        {
            Debug.LogError($"OnTriggerEnterScript 例外: {ex.Message}");
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void OnTriggerStayScript(void* gcHandle, TriggerInfoDto* dto)
    {
        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            if (methods.OnTriggerStay == null) return;

            var trigger = CollisionConverter.ToTrigger(dto);
            var parameters = new object[] { trigger };
            methods.OnTriggerStay.Invoke(instance, parameters);
        }
        catch (Exception ex)
        {
            Debug.LogError($"OnTriggerStayScript 例外: {ex.Message}");
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    public static void OnTriggerExitScript(void* gcHandle, TriggerInfoDto* dto)
    {
        try
        {
            var instance = Unwrap(gcHandle);
            var methods = BehaviourMethodCache.Get(instance.GetType());
            if (methods.OnTriggerExit == null) return;

            var trigger = CollisionConverter.ToTrigger(dto);
            var parameters = new object[] { trigger };
            methods.OnTriggerExit.Invoke(instance, parameters);
        }
        catch (Exception ex)
        {
            Debug.LogError($"OnTriggerExitScript 例外: {ex.Message}");
        }
    }

    // ---- ユーティリティ ----

    private static object Unwrap(void* gcHandle)
        => GCHandle.FromIntPtr((nint)gcHandle).Target!;
}
