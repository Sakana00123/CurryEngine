using CurryEngine.Interfaces;
using CurryEngine.Runtime.Native;

namespace CurryEngine.Runtime
{
    public class TransformAccessor : ITransformAccessor
    {
        public int GetChildCount(ulong transformId)
        {
            return NativeMethods.Transform_GetChildCount(transformId);
        }

        public ulong GetChild(ulong transformId, int index)
        {
            return NativeMethods.Transform_GetChild(transformId, index);
        }

        public Vector3 GetForward(ulong transformId)
        {
            return NativeMethods.Transform_GetForward(transformId);
        }

        public Vector3 GetLocalPosition(ulong transformId)
        {
            return NativeMethods.Transform_GetLocalPosition(transformId);
        }

        public ulong GetParent(ulong transformId)
        {
            return NativeMethods.Transform_GetParent(transformId);
        }

        public Vector3 GetPosition(ulong transformId)
        {
            return NativeMethods.Transform_GetPosition(transformId);
        }

        public Vector3 GetRight(ulong transformId)
        {
            return NativeMethods.Transform_GetRight(transformId);
        }

        public Quaternion GetRotation(ulong transformId)
        {
            return NativeMethods.Transform_GetRotation(transformId);
        }

        public Vector3 GetScale(ulong transformId)
        {
            return NativeMethods.Transform_GetScale(transformId);
        }

        public Vector3 GetUp(ulong transformId)
        {
            return NativeMethods.Transform_GetUp(transformId);
        }

        public void LookAt(ulong transformId, Vector3 targetPosition, Vector3 up)
        {
            NativeMethods.Transform_LookAt(transformId, targetPosition, up);
        }

        public void Rotate(ulong transformId, Vector3 rotation)
        {
            NativeMethods.Transform_Rotate(transformId, rotation);
        }

        public void SetLocalPosition(ulong transformId, Vector3 localPosition)
        {
            NativeMethods.Transform_SetLocalPosition(transformId, localPosition);
        }

        public void SetParent(ulong transformId, ulong parentTransformId)
        {
            NativeMethods.Transform_SetParent(transformId, parentTransformId);
        }

        public void SetPosition(ulong transformId, Vector3 position)
        {
            NativeMethods.Transform_SetPosition(transformId, position);
        }

        public void SetRotation(ulong transformId, Quaternion rotation)
        {
            NativeMethods.Transform_SetRotation(transformId, rotation);
        }

        public void SetScale(ulong transformId, Vector3 scale)
        {
            NativeMethods.Transform_SetScale(transformId, scale);
        }

        public void Translate(ulong transformId, Vector3 translation)
        {
            NativeMethods.Transform_Translate(transformId, translation);
        }
    }
}
