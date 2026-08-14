using CurryEngine.Interfaces;
using CurryEngine.Runtime.Native;

namespace CurryEngine.Runtime;

/// <summary>
/// C++側のコンポーネントアクセス用インターフェースの実装クラス。
/// </summary>
internal sealed class ComponentAccessor : IComponentAccessor
{
    public T? Get<T>(ulong ownerId) where T : Component
        => ComponentCache.Get<T>(ownerId);

    public T[] GetAll<T>(ulong ownerId) where T : Component
        => ComponentCache.GetAll<T>(ownerId);

    public T? GetOrCreate<T>(ulong ownerId, ulong componentId) where T : Component
        => (T?)ComponentCache.GetOrCreate(ownerId, componentId, typeof(T));

    public T? GetByComponentId<T>(ulong componentId) where T : Component
    {
        ulong ownerId = NativeMethods.Component_GetOwner(componentId);
        return (T?)ComponentCache.GetOrCreate(ownerId, componentId, typeof(T));
    }

    public bool IsValid(ulong componentId) =>
        NativeMethods.Component_IsValid(componentId);

    public bool IsEnabled(ulong componentId, ulong ownerId) =>
        NativeMethods.Component_GetEnabled(componentId, ownerId) != 0;

    public void SetEnabled(ulong componentId, ulong ownerId, bool enabled) =>
        NativeMethods.Component_SetEnabled(componentId, ownerId, enabled ? 1 : 0);


    public void Destroy(ulong componentId)
        => NativeMethods.Component_Destroy(componentId);

    public ulong InstantiateFromId(ulong prefabId, ulong parentId, Vector3 position, Quaternion rotation)
        => NativeMethods.GameObject_InstantiateFromId(prefabId, parentId, position, rotation);

    public ulong InstantiateFromResource(string resourcePath, ulong parentId, Vector3 position, Quaternion rotation)
        => NativeMethods.GameObject_InstantiateFromResource(resourcePath, parentId, position, rotation);

    public ulong FindGameObjectByName(string name)
        => NativeMethods.GameObject_FindByName(name);

    public ulong FindGameObjectById(ulong componentId)
        => NativeMethods.Component_GetOwner(componentId);
}
