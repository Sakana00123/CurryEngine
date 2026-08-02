
namespace CurryEngine
{
    public class GameObjectAccessor : IGameObjectAccessor
    {
        private readonly Dictionary<ulong, GameObject> s_cache = new();

        public GameObject GetOrCreate(ulong objectId)
        {
            if (!s_cache.TryGetValue(objectId, out var gameObject))
            {
                gameObject = new GameObject(objectId);
                s_cache[objectId] = gameObject;
            }
            return gameObject;
        }

        public void Remove(ulong objectId)
        {
            s_cache.Remove(objectId);
        }
    }
}
