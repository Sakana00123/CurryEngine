using CurryEngine.Interop;
using System;

namespace CurryEngine
{
    /// <summary>
    /// すべてのオブジェクトの基底クラス。
    /// </summary>
    public abstract class Object
    {
        internal ulong objectId { get; private set; } = 0;
        internal void SetObjectIdInternal(ulong newId)
        {
            objectId = newId;
        }

        public override bool Equals(object? obj)
        {
            if (obj is Object other)
                return this == other;
            return false;
        }
        public static bool operator ==(Object? a, Object? b)
        {
            if (ReferenceEquals(a, b)) // 同一のインスタンスを参照している場合は等しいとみなす(null同士も含む)
                return true;
            if (a is null || b is null) // どちらか一方がnullであれば等しくない
                return false;
            if (a.GetType() != b.GetType()) // 型が異なる場合は等しくない
                return false;
            return a.objectId == b.objectId; // 同じ型でobjectIdが同じなら等しいとみなす
        }
        public static bool operator !=(Object? a, Object? b)
            => !(a == b);

        public static implicit operator bool(Object? obj)
            => obj != null && obj.objectId != 0;
        public static implicit operator ulong(Object? obj)
            => obj != null ? obj.objectId : 0;

        public override int GetHashCode()
                => objectId.GetHashCode();

        public override string ToString()
                => $"{GetType().Name}({objectId})";

    }
}
