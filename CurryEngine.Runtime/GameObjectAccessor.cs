
using CurryEngine.Interfaces;
using CurryEngine.Runtime.Native;

namespace CurryEngine.Runtime
{
    public class GameObjectAccessor : IGameObjectAccessor
    {
        public GameObject GetGameObject(ulong objectId)
        {
            return GameObjectCache.GetOrCreate(objectId);
        }

        public string GetName(ulong gameObjectId)
        {
            return NativeMethods.GameObject_GetName(gameObjectId);
        }

        public void SetName(ulong gameObjectId, string name)
        {
            NativeMethods.GameObject_SetName(gameObjectId, name);
        }


        public bool IsActive(ulong gameObjectId)
        {
            return NativeMethods.GameObject_IsActive(gameObjectId);
        }

        public void SetActive(ulong gameObjectId, bool active)
        {
            NativeMethods.GameObject_SetActive(gameObjectId, active);
        }

        public void Destroy(ulong gameObjectId, float delay)
        {
            NativeMethods.Entity_Destroy(gameObjectId, delay);
        }
    }
}
