using System;
using System.Runtime.InteropServices;

namespace CurryEngine
{
    /// <summary>
    /// Prefab の参照を表す構造体。
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct PrefabReference
    {
        public string AssetId;

        public PrefabReference(string assetId)
        {
            AssetId = assetId;
        }

        public static implicit operator string(PrefabReference prefabRef) => prefabRef.AssetId;
        public static implicit operator PrefabReference(string assetId) => new PrefabReference(assetId);
        public override string ToString() => AssetId;
        public override bool Equals(object? obj) => obj is PrefabReference other && AssetId == other.AssetId;
        public override int GetHashCode() => AssetId.GetHashCode();
        public static implicit operator bool(PrefabReference prefabRef) => !string.IsNullOrEmpty(prefabRef.AssetId) || prefabRef.AssetId != "null";
    }
}
