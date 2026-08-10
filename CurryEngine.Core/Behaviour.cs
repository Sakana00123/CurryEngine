
using CurryEngine.Interfaces;
using System;
using System.Runtime.CompilerServices;

namespace CurryEngine;

public abstract class Behaviour : Component
{

    // ----- ライフサイクルメソッド (override して使う) -----
    public virtual void Awake() { }
    public virtual void Start() { }
    public virtual void Update() { }
    public virtual void OnDestroy() { }

    public virtual void OnEnable() { }
    public virtual void OnDisable() { }


    public virtual void OnCollisionEnter(Collision collision) { }
    public virtual void OnCollisionStay(Collision collision) { }
    public virtual void OnCollisionExit(Collision collision) { }

    public virtual void OnTriggerEnter(Trigger trigger) { }
    public virtual void OnTriggerStay(Trigger trigger) { }
    public virtual void OnTriggerExit(Trigger trigger) { }

    // ----- C#側でのエンティティ管理 -----

    /// <summary>
    /// 指定した GameObject を破棄します。null の場合は何もしません。
    /// </summary>
    public static void Destroy(GameObject target, float delay = 0f)
    {
        if (target != null)
        {
            GameObject.Accessor?.Destroy(target.objectId, delay);
        }
        else
        {
            Debug.LogWarning("Destroy called with null GameObject.");
        }
    }

    /// <summary>
    /// 指定した Component を破棄します。null の場合は何もしません。
    /// </summary>
    /// <param name="target"></param>
    public static void Destroy(Component target)
    {
        if (target != null)
        {
            Component.Accessor?.Destroy(target.objectId);
        }
        else
        {
            Debug.LogWarning("Destroy called with null Component.");
        }
    }

    // ----- エンジンAPI へのアクセス -----
    
    public static string DebugAccessorInfo()
    {
        var asm = typeof(Component).Assembly;
        var alc = System.Runtime.Loader.AssemblyLoadContext.GetLoadContext(asm);
        return $"Behaviour.DebugAccessorInfo()\n" +
               $"AssemblyハッシュID: {RuntimeHelpers.GetHashCode(asm)}\n" +
               $"ALC名: {alc?.Name}\n\n";
    }

    public bool isValid
        => Component.Accessor?.IsValid(objectId) ?? false;

    public override string ToString()
        => $"Behavior on Entity {ownerId}";

    /// <summary>
    /// 指定されたオブジェクトを複製して新しいインスタンスを作成します。
    /// </summary>
    /// <param name="original">複製元のオブジェクト。null であってはなりません。</param>
    /// <param name="parent">新しいオブジェクトの親トランスフォーム。null の場合、新しいオブジェクトはシーンのルートに配置されます。</param>
    /// <param name="position">新しいオブジェクトの位置。親が指定されている場合、ローカル座標系での位置になります。</param>
    /// <param name="rotation">新しいオブジェクトの回転。親が指定されている場合、ローカル座標系での回転になります。</param>
    /// <returns></returns>
    /// <exception cref="InvalidOperationException"></exception>
    public static GameObject Instantiate(GameObject original, Transform? parent, Vector3 position, Quaternion rotation)
    {
        //var newId = NativeMethods.GameObject_InstantiateFromId(original.objectId, parent != null ? parent.ownerId : 0, position, rotation);
        var newId = Component.Accessor?.InstantiateFromId(original.objectId, parent != null ? parent.ownerId : 0, position, rotation) ?? 0;
        if (newId == 0) throw new InvalidOperationException("Failed to instantiate object.");
        return GameObject.Accessor?.GetOrCreate(newId) ?? throw new InvalidOperationException($"Failed to retrieve GameObject with ID: {newId}");
    }

    public static GameObject Instantiate(GameObject original, Transform? parent = null)
    {
        return Instantiate(original, parent, Vector3.zero, Quaternion.identity);
    }
    
    public static GameObject Instantiate(GameObject original, Vector3 position, Quaternion rotation)
    {
        return Instantiate(original, null, position, rotation);
    }

    public static GameObject Instantiate(GameObject original)
    {
        return Instantiate(original, null, Vector3.zero, Quaternion.identity);
    }


    public static GameObject Instantiate(string resourcePath, Transform? parent, Vector3 position, Quaternion rotation)
    {
        //var newId = NativeMethods.GameObject_InstantiateFromResource(resourcePath, parent != null ? parent.ownerId : 0, position, rotation);
        var newId = Component.Accessor?.InstantiateFromResource(resourcePath, parent != null ? parent.ownerId : 0, position, rotation) ?? 0;
        if (newId == 0) throw new InvalidOperationException($"Failed to instantiate object from resource: {resourcePath}");
        return GameObject.Accessor?.GetOrCreate(newId) ?? throw new InvalidOperationException($"Failed to retrieve GameObject with ID: {newId}");
    }

    public static GameObject Instantiate(string resourcePath, Transform? parent = null)
    {
        return Instantiate(resourcePath, parent, Vector3.zero, Quaternion.identity);
    }

    public static GameObject Instantiate(string resourcePath, Vector3 position, Quaternion rotation)
    {
        return Instantiate(resourcePath, null, position, rotation);
    }

    public static GameObject Instantiate(string resourcePath)
    {
        return Instantiate(resourcePath, null, Vector3.zero, Quaternion.identity);
    }

    
    public static GameObject? Find(string name)
    {
        //var id = NativeMethods.GameObject_FindByName(name);
        var id = Component.Accessor?.FindGameObjectByName(name) ?? 0;
        if (id == 0) return null;
        return GameObject.Accessor?.GetOrCreate(id);
    }

    /// <summary>
    /// 指定されたオブジェクトIDに対応する GameObject を取得します。存在しない場合は null を返します。
    /// </summary>
    /// <param name="objectId"> 取得したい GameObject のオブジェクトID。</param>
    /// <returns> 存在する場合は GameObject、存在しない場合は null。</returns>
    public static GameObject? GetGameObjectById(ulong objectId)
    {
        return GameObject.Accessor?.GetOrCreate(objectId);
    }

    /// <summary>
    /// 指定されたコンポーネントIDに対応する Component を取得します。存在しない場合は null を返します。
    /// </summary>
    /// <typeparam name="T"> 取得したいコンポーネントの型。</typeparam>
    /// <param name="componentId"> 取得したいコンポーネントのオブジェクトID。</param>
    /// <returns> 存在する場合は指定された型のコンポーネント、存在しない場合は null。</returns>
    public static T? GetComponentById<T>(ulong componentId) where T : Component
    {
        return Component.Accessor?.Get<T>(componentId);
    }
}
