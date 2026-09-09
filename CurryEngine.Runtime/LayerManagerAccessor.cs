using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CurryEngine.Interfaces;

namespace CurryEngine.Runtime
{
    internal sealed class LayerManagerAccessor : ILayerMask
    {
        public LayerMask GetLayerMaskValue(string layerName)
        {
            return Native.NativeMethods.LayerManager_GetLayerMaskByName(layerName);
        }
        public string GetLayerName(int layer)
        {
            return Native.NativeMethods.LayerManager_GetLayerName(layer);
        }
    }
}
