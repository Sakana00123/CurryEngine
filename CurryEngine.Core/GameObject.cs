
using System;
using CurryEngine.Interfaces;

namespace CurryEngine
{
    public class GameObject : Object
    {
        internal static IGameObjectAccessor? Accessor { get; set; }


        public GameObject(ulong objectId)
        {
            SetObjectIdInternal(objectId);
        }


        public Transform transform
            => Component.Accessor?.Get<Transform>(objectId)!;


        public string name
        {
            get => Accessor?.GetName(objectId) ?? throw new InvalidOperationException($"GameObject {objectId} has no name");
            set => Accessor?.SetName(objectId, value);
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
            => Accessor?.IsActive(objectId) ?? throw new InvalidOperationException($"GameObject {objectId} has no active state");

        public void SetActive(bool active)
            => Accessor?.SetActive(objectId, active);


        public override bool Equals(object? obj)
        {
            if (obj is GameObject other)
                return this == other;
            return false;
        }

        public static bool operator ==(GameObject? a, GameObject? b)
        {
            if (ReferenceEquals(a, b)) return true;
            if (a is null || b is null) return false;
            return a.objectId == b.objectId;
        }

        public static bool operator !=(GameObject? a, GameObject? b)
            => !(a == b);

        public static implicit operator bool(GameObject? obj)
            => obj != null && (Accessor?.IsActive(obj.objectId) ?? false);


        public override int GetHashCode()
            => objectId.GetHashCode();

        public override string ToString()
            => $"GameObject({objectId})";
    }
}
