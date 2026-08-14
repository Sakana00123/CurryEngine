using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurryEngine.Interfaces
{
    internal interface IGameObjectAccessor
    {
        GameObject GetOrCreate(ulong gameObjectId);

        string GetName(ulong gameObjectId);

        void SetName(ulong gameObjectId, string name);

        bool IsActive(ulong gameObjectId);

        void SetActive(ulong gameObjectId, bool active);

        bool IsValid(ulong gameObjectId);

        void Destroy(ulong gameObjectId, float delay);


        GameObject[] GetAllGameObjects(int maxCount = 64);

        GameObject[] FindGameObjectsByType(string typeName, int maxCount = 64);

    }
}
