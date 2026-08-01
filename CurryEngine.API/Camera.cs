
using CurryEngine.Runtime.Native;
namespace CurryEngine;

public class Camera : Component
{
    internal Camera(ulong ownerId, ulong objectId)
    {
        Setup(ownerId, objectId);
    }

    /// <summary>
    /// シーン内のメインカメラを取得します。メインカメラが存在しない場合はnullを返します。
    /// </summary>
    public static Camera? main
    {
        get
        {
            var mainCameraId = NativeMethods.Camera_GetMainCameraId();
            if (mainCameraId == 0)
                return null;
            var mainCameraOwnerId = NativeMethods.Component_GetOwner(mainCameraId);
            return new Camera(mainCameraOwnerId, mainCameraId);
        }
    }

    /// <summary>
    /// カメラの投影行列。
    /// </summary>
    public Matrix4x4 projectionMatrix => NativeMethods.Camera_GetProjectionMatrix(objectId);

    /// <summary>
    /// カメラのビュー行列。
    /// </summary>
    public Matrix4x4 viewMatrix => NativeMethods.Camera_GetViewMatrix(objectId);

    /// <summary>
    /// カメラの視野角（Field of View）。単位は度。
    /// </summary>
    public float fieldOfView
    {
        get => NativeMethods.Camera_GetFieldOfView(objectId);
        set => NativeMethods.Camera_SetFieldOfView(objectId, value);
    }

    /// <summary>
    /// カメラのアスペクト比（Aspect Ratio）。通常は画面の幅を高さで割った値。
    /// </summary>
    public float aspect
    {
        get => NativeMethods.Camera_GetAspect(objectId);
        set => NativeMethods.Camera_SetAspect(objectId, value);
    }

    /// <summary>
    /// カメラの近クリップ平面までの距離。
    /// </summary>
    public float nearClipPlane
    {
        get => NativeMethods.Camera_GetNearClipPlane(objectId);
        set => NativeMethods.Camera_SetNearClipPlane(objectId, value);
    }

    /// <summary>
    /// カメラの遠クリップ平面までの距離。
    /// </summary>
    public float farClipPlane
    { 
        get => NativeMethods.Camera_GetFarClipPlane(objectId);
        set => NativeMethods.Camera_SetFarClipPlane(objectId, value);
    }
}
