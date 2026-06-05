using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurryEngine;

public interface IComponentAccessor
{
    T? Get<T>(ulong ownerId) where T : Component;
    T[] GetAll<T>(ulong ownerId) where T : Component;
    Transform? GetTransform(ulong ownerId);
}
