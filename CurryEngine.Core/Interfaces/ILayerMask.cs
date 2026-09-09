using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurryEngine.Interfaces
{
    internal interface ILayerMask
    {

        public LayerMask GetLayerMaskValue(string layerName);

        public string GetLayerName(int layer);

    }
}
