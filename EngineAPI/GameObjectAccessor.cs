using CurryEngine.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurryEngine
{
    internal static class GameObjectAccessor
    {
        private static readonly Dictionary<ulong, GameObject> s_cache = new();

        public static GameObject GetOrCreate(ulong objectId)
        {
            if (!s_cache.TryGetValue(objectId, out var gameObject))
            {
                gameObject = new GameObject(objectId);
                s_cache[objectId] = gameObject;
            }
            return gameObject;
        }

        public static void Remove(ulong objectId)
        {
            s_cache.Remove(objectId);
        }
    }
}
