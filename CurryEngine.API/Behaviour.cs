using CurryEngine.Interop;
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
    /// このコンポーネントがアタッチされているエンティティを破棄する。
    /// </summary>
    public void Destroy()
    {
        NativeMethods.Entity_Destroy(ownerId);
    }

    // ----- エンジンAPI へのアクセス -----
    
    public static string DebugAccessorInfo()
    {
        var asm = typeof(Component).Assembly;
        var alc = System.Runtime.Loader.AssemblyLoadContext.GetLoadContext(asm);
        return $"Behaviour.DebugAccessorInfo()\n" +
               $"AssemblyハッシュID: {RuntimeHelpers.GetHashCode(asm)}\n" +
               $"ALC名: {alc?.Name}\n" +
               $"Accessor: {Component.Accessor != null}\n\n";
    }

    public bool isValid
        => NativeMethods.Entity_IsValid(ownerId);

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
        var newId = NativeMethods.GameObject_InstantiateFromId(original.objectId, parent != null ? parent.ownerId : 0, position, rotation);
        if (newId == 0) throw new InvalidOperationException("Failed to instantiate object.");
        return GameObject.Accessor?.GetOrCreate(newId) ?? new GameObject(newId);
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
        var newId = NativeMethods.GameObject_InstantiateFromResource(resourcePath, parent != null ? parent.ownerId : 0, position, rotation);
        if (newId == 0) throw new InvalidOperationException($"Failed to instantiate object from resource: {resourcePath}");
        return GameObject.Accessor?.GetOrCreate(newId) ?? new GameObject(newId);
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
        var id = NativeMethods.GameObject_FindByName(name);
        if (id == 0) return null;
        return GameObject.Accessor?.GetOrCreate(id) ?? new GameObject(id);
    }
}
