namespace CurryEngine.Interfaces;

internal interface IComponentAccessor
{
    T? Get<T>(ulong ownerId) where T : Component;
    T[] GetAll<T>(ulong ownerId) where T : Component;
    T? GetOrCreate<T>(ulong ownerId, ulong componentId) where T : Component;

    T? GetByComponentId<T>(ulong componentId) where T : Component;

    bool IsValid(ulong componentId);

    void SetEnabled(ulong componentId, ulong ownerId, bool enabled);

    bool IsEnabled(ulong componentId, ulong ownerId);

    void Destroy(ulong componentId);

    ulong InstantiateFromId(ulong prefabId, ulong parentId, Vector3 position, Quaternion rotation);

    ulong InstantiateFromResource(string resourcePath, ulong parentId, Vector3 position, Quaternion rotation);

    ulong FindGameObjectByName(string name);

    ulong FindGameObjectById(ulong componentId);
}
