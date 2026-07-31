
using CurryEngine.Interop;
using System;

namespace CurryEngine
{
    public class GameObject : Object
    {
        public GameObject(ulong objectId)
        {
            SetObjectIdInternal(objectId);
        }


        public Transform transform
            => Component.Accessor?.GetTransform(objectId)!;


        public string name
        {
            get => NativeMethods.GameObject_GetName(objectId) ?? throw new InvalidOperationException($"GameObject {objectId} has no name");
            set => NativeMethods.GameObject_SetName(objectId, value);
        }

        public T? GetComponent<T>() where T : Component
            => Component.Accessor?.Get<T>(objectId);
        
        public T GetRequiredComponent<T>() where T : Component
            => Component.Accessor?.Get<T>(objectId)
               ?? throw new InvalidOperationException(
                   $"Component {typeof(T).Name} not found on GameObject {objectId}");

        public bool TryGetComponent<T>(out T component) where T : Component
            {
            component = Component.Accessor?.Get<T>(objectId)!;
            return component != null;
        }

        public bool IsActive()
            => NativeMethods.GameObject_IsActive(objectId);

        public void SetActive(bool active)
            => NativeMethods.GameObject_SetActive(objectId, active);


        public override bool Equals(object? obj)
        {
            if (obj is GameObject other)
                return this == other;
            return false;
        }

        public override int GetHashCode()
            => objectId.GetHashCode();

        public override string ToString()
            => $"GameObject({objectId})";
    }
}
