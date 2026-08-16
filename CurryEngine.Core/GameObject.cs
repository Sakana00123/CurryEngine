
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

        public bool IsValid
            => Accessor?.IsValid(objectId) ?? false;

        public void Destroy(float delay = 0.0f)
            => Accessor?.Destroy(objectId, delay);

        public override bool Equals(object? obj)
        {
            if (obj is GameObject other)
                return this == other;
            return false;
        }

        public static bool operator ==(GameObject? a, GameObject? b)
        {
            if (ReferenceEquals(a, b)) return true;
            // isValid=false は null と同等に扱う
            uint bitA = (uint)a; // 有効かどうかの判定をビット演算で行うために uint に変換
            uint bitB = (uint)b; // 有効かどうかの判定をビット演算で行うために uint に変換
            if ((bitA ^ bitB) == 1) return false; // どちらか一方が有効で、もう一方が無効の場合は等しくない
            if (bitA == 0 && bitB == 0) return true; // 両方とも無効の場合は等しいとみなす
            if (a is null || b is null) return false;
            return a.objectId == b.objectId;
        }

        public static bool operator !=(GameObject? a, GameObject? b)
            => !(a == b);

        public static implicit operator bool(GameObject? obj)
            => obj != null && obj.IsValid;


        public override int GetHashCode()
            => objectId.GetHashCode();

        public override string ToString()
            => $"GameObject({objectId})";
    }
}
