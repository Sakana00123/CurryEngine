using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurryEngine.Interfaces
{
    internal interface ITransformAccessor
    {
        public Vector3 GetPosition(ulong transformId);
        public void SetPosition(ulong transformId, Vector3 position);
        public Vector3 GetLocalPosition(ulong transformId);
        public void SetLocalPosition(ulong transformId, Vector3 localPosition);
        public Quaternion GetRotation(ulong transformId);
        public void SetRotation(ulong transformId, Quaternion rotation);
        public Vector3 GetScale(ulong transformId);
        public void SetScale(ulong transformId, Vector3 scale);
        public ulong GetParent(ulong transformId);
        public void SetParent(ulong transformId, ulong parentTransformId);
        public ulong GetChild(ulong transformId, int index);

        public int GetChildCount(ulong transformId);

        public void Translate(ulong transformId, Vector3 translation);

        public void Rotate(ulong transformId, Vector3 rotation);

        public void LookAt(ulong transformId, Vector3 targetPosition, Vector3 up);

        public Vector3 GetForward(ulong transformId);

        public Vector3 GetUp(ulong transformId);

        public Vector3 GetRight(ulong transformId);

    }
}
