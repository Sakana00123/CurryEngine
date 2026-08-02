
namespace CurryEngine;

/// <summary>
/// C++側のコンポーネントアクセスをC#のインターフェースに適合させるアダプター。
/// </summary>
internal sealed class ComponentAccessorAdapter : IComponentAccessor
{
    public T? Get<T>(ulong ownerId) where T : Component
        => ComponentAccess.Get<T>(ownerId);

    public T[] GetAll<T>(ulong ownerId) where T : Component
        => ComponentAccess.GetAll<T>(ownerId);

    public Transform? GetTransform(ulong ownerId)
        => ComponentAccess.GetTransform(ownerId);
}
