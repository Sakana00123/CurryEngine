using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurryEngine
{
    public interface IGameObjectAccessor
    {
        public GameObject GetOrCreate(ulong objectId);

        public void Remove(ulong objectId);
    }
}
