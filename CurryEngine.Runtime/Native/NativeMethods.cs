
using CurryEngine;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace CurryEngine.Runtime.Native;
internal static partial class NativeMethods
{
    internal const string Dll = "CurryEngine.exe";

    // ------------------------------------ Console -----------------------------------------
    [LibraryImport(Dll)] internal static partial void Console_CustomLog(int logLevel, [MarshalAs(UnmanagedType.LPUTF8Str)] string message,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string file, int line);

    // ------------------------------------ Application -----------------------------------------

    //[LibraryImport(Dll)] internal static partial void Application_Quit();

    // ------------------------------------ Time -----------------------------------------

    //[LibraryImport(Dll)] internal static partial float Time_GetTime();
    [LibraryImport(Dll)] internal static partial float Time_GetDeltaTime();
    [LibraryImport(Dll)] internal static partial float Time_GetUnscaledDeltaTime();
    //[LibraryImport(Dll)] internal static partial float Time_GetFixedDeltaTime();
    [LibraryImport(Dll)] internal static partial float Time_GetTimeScale();
    [LibraryImport(Dll)] internal static partial void Time_SetTimeScale(float timeScale);

    // ------------------------------------ Input -----------------------------------------

    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool Input_GetKey(int keyCode);

    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool Input_GetKeyDown(int keyCode);
    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool Input_GetKeyUp(int keyCode);

    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool Input_GetAction([MarshalAs(UnmanagedType.LPUTF8Str)] string actionName);
    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool Input_GetActionDown([MarshalAs(UnmanagedType.LPUTF8Str)] string actionName);
    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool Input_GetActionUp([MarshalAs(UnmanagedType.LPUTF8Str)] string actionName);

    //[LibraryImport(Dll)]
    //[return: MarshalAs(UnmanagedType.Bool)]
    //internal static partial bool Input_GetMouseButton(int button);
    //[LibraryImport(Dll)]
    //[return: MarshalAs(UnmanagedType.Bool)]
    //internal static partial bool Input_GetMouseButtonDown(int button);
    //[LibraryImport(Dll)]
    //[return: MarshalAs(UnmanagedType.Bool)]
    //internal static partial bool Input_GetMouseButtonUp(int button);

    [LibraryImport(Dll)] internal static partial void Input_GetAxis(int axis, out Vector2 value);
    [LibraryImport(Dll)] internal static partial void Input_GetAxisRaw(int axis, out Vector2 value);
    [LibraryImport(Dll)] internal static partial int Input_GetMouseDeltaX();
    [LibraryImport(Dll)] internal static partial int Input_GetMouseDeltaY();
    [LibraryImport(Dll)] internal static partial int Input_GetMousePositionX();
    [LibraryImport(Dll)] internal static partial int Input_GetMousePositionY();
    [LibraryImport(Dll)] internal static partial float Input_GetMouseScrollDeltaX();

    //[LibraryImport(Dll)] internal static partial float Input_GetMouseScrollDeltaY();
    //[LibraryImport(Dll)] internal static partial int Input_GetGamepadButton(int gamepadIndex, int button);

    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool Input_SetCursorLock(int cursorLock, int changeVisible);

    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool Input_GetCursorLock();
    
    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool Input_GetCursorVisible();

    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool Input_IsGamepadConnected();

    // ------------------------------------- Debug -----------------------------------------


    // ------------------------------------- Audio -----------------------------------------

    // ------------------------------------- Camera -----------------------------------------

    //[LibraryImport(Dll)] internal static partial void Camera_WorldToScreenPoint(ulong cameraId, Vector3 worldPos, out Vector3 screenPos);

    //[LibraryImport(Dll)] internal static partial void Camera_ScreenToWorldPoint(ulong cameraId, Vector3 screenPos, out Vector3 worldPos);

    //[LibraryImport(Dll)] internal static partial void Camera_ScreenToViewportPoint(ulong cameraId, Vector3 screenPos, out Vector3 viewportPos);

    //[LibraryImport(Dll)] internal static partial void Camera_ViewportToScreenPoint(ulong cameraId, Vector3 viewportPos, out Vector3 screenPos);

    //[LibraryImport(Dll)] internal static partial void Camera_ViewportToWorldPoint(ulong cameraId, Vector3 viewportPos, out Vector3 worldPos);
    //[LibraryImport(Dll)] internal static partial void Camera_WorldToViewportPoint(ulong cameraId, Vector3 worldPos, out Vector3 viewportPos);

    //[LibraryImport(Dll)] internal static partial void Camera_ScreenPointToRay(ulong cameraId, Vector3 screenPos, out Vector3 rayOrigin, out Vector3 rayDirection);

    //[LibraryImport(Dll)] internal static partial void Camera_ViewportPointToRay(ulong cameraId, Vector3 viewportPos, out Vector3 rayOrigin, out Vector3 rayDirection);

    //[LibraryImport(Dll)] internal static partial void Camera_WorldPointToRay(ulong cameraId, Vector3 worldPos, out Vector3 rayOrigin, out Vector3 rayDirection);

    [LibraryImport(Dll)] internal static partial ulong Camera_GetMainCameraId();

    [LibraryImport(Dll)] internal static partial Matrix4x4 Camera_GetProjectionMatrix(ulong cameraId);
    //[LibraryImport(Dll)] internal static partial void Camera_GetInverseProjectionMatrix(ulong cameraId, out Matrix4x4 inverseProjectionMatrix);

    [LibraryImport(Dll)] internal static partial Matrix4x4 Camera_GetViewMatrix(ulong cameraId);
    //[LibraryImport(Dll)] internal static partial void Camera_GetInverseViewMatrix(ulong cameraId, out Matrix4x4 inverseViewMatrix);

    //[LibraryImport(Dll)] internal static partial void Camera_GetCameraToWorldMatrix(ulong cameraId, out Matrix4x4 cameraToWorldMatrix);

    //[LibraryImport(Dll)] internal static partial void Camera_GetWorldToCameraMatrix(ulong cameraId, out Matrix4x4 worldToCameraMatrix);

    [LibraryImport(Dll)] internal static partial float Camera_GetFieldOfView(ulong cameraId);

    [LibraryImport(Dll)] internal static partial void Camera_SetFieldOfView(ulong cameraId, float fieldOfView);

    //[LibraryImport(Dll)] internal static partial float Camera_GetOrthographicSize(ulong cameraId);
    //[LibraryImport(Dll)] internal static partial void Camera_SetOrthographicSize(ulong cameraId, float orthographicSize);
    //[LibraryImport(Dll)]
    //[return: MarshalAs(UnmanagedType.Bool)] internal static partial bool Camera_GetIsOrthographic(ulong cameraId);
    //[LibraryImport(Dll)] internal static partial void Camera_SetIsOrthographic(ulong cameraId, bool isOrthographic);
    [LibraryImport(Dll)] internal static partial float Camera_GetAspect(ulong cameraId);
    [LibraryImport(Dll)] internal static partial void Camera_SetAspect(ulong cameraId, float aspect);
    [LibraryImport(Dll)] internal static partial float Camera_GetNearClipPlane(ulong cameraId);
    [LibraryImport(Dll)] internal static partial void Camera_SetNearClipPlane(ulong cameraId, float nearClipPlane);
    [LibraryImport(Dll)] internal static partial float Camera_GetFarClipPlane(ulong cameraId);
    [LibraryImport(Dll)] internal static partial void Camera_SetFarClipPlane(ulong cameraId, float farClipPlane);


    // ------------------------------------ Physics -----------------------------------------
    //[LibraryImport(Dll)] internal static partial void Physics_Raycast(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hitInfo);
    //[LibraryImport(Dll)] internal static partial void Physics_RaycastAll(Vector3 origin, Vector3 direction, float maxDistance, out IntPtr hitInfos, out int hitCount);
    //[LibraryImport(Dll)] internal static partial void Physics_OverlapSphere(Vector3 center, float radius, out IntPtr colliders, out int colliderCount);


    // -------------------------------------------- Rigidbody -----------------------------------------
    //[LibraryImport(Dll)] internal static partial void Rigidbody_AddForce(ulong ownerId, Vector3 force, int forceMode);
    //[LibraryImport(Dll)] internal static partial void Rigidbody_AddTorque(ulong ownerId, Vector3 torque, int forceMode);
    //[LibraryImport(Dll)] internal static partial Vector3 Rigidbody_GetVelocity(ulong ownerId);
    //[LibraryImport(Dll)] internal static partial void Rigidbody_SetVelocity(ulong ownerId, Vector3 velocity);
    //[LibraryImport(Dll)] internal static partial Vector3 Rigidbody_GetAngularVelocity(ulong ownerId);
    //[LibraryImport(Dll)] internal static partial void Rigidbody_SetAngularVelocity(ulong ownerId, Vector3 angularVelocity);
    //[LibraryImport(Dll)] internal static partial void Rigidbody_SetMass(ulong ownerId, float mass);
    //[LibraryImport(Dll)] internal static partial float Rigidbody_GetMass(ulong ownerId);
    //[LibraryImport(Dll)] internal static partial void Rigidbody_SetDrag(ulong ownerId, float drag);
    //[LibraryImport(Dll)] internal static partial float Rigidbody_GetDrag(ulong ownerId);
    //[LibraryImport(Dll)] internal static partial void Rigidbody_SetAngularDrag(ulong ownerId, float angularDrag);
    //[LibraryImport(Dll)] internal static partial float Rigidbody_GetAngularDrag(ulong ownerId);
    //[LibraryImport(Dll)] internal static partial void Rigidbody_SetKinematicTarget(ulong ownerId, Vector3 position, Quaternion rotation);


    //[LibraryImport(Dll)]
    //internal static partial void Rigidbody_SetUseGravity(ulong ownerId,[MarshalAs(UnmanagedType.Bool)] bool useGravity);
    //[LibraryImport(Dll)]
    //[return: MarshalAs(UnmanagedType.Bool)]
    //internal static partial bool Rigidbody_GetUseGravity(ulong ownerId);

    //[LibraryImport(Dll)]
    //internal static partial void Rigidbody_SetIsKinematic(ulong ownerId, [MarshalAs(UnmanagedType.Bool)] bool isKinematic);
    //[LibraryImport(Dll)]
    //[return: MarshalAs(UnmanagedType.Bool)]
    //internal static partial bool Rigidbody_GetIsKinematic(ulong ownerId);

    //// ------------------------------------- Collider -----------------------------------------
    //[LibraryImport(Dll)] internal static partial void Collider_SetIsTrigger(ulong ownerId, ulong componentId, [MarshalAs(UnmanagedType.Bool)] bool isTrigger);
    //[LibraryImport(Dll)]
    //[return: MarshalAs(UnmanagedType.Bool)]
    //internal static partial bool Collider_GetIsTrigger(ulong ownerId, ulong componentId);



    // ------------------------------------ Object -----------------------------------------

    [LibraryImport(Dll)] internal static partial ulong Object_Instantiate(ulong objectId);

    // ------------------------------------ GameObject -----------------------------------------
    [return: MarshalAs(UnmanagedType.LPUTF8Str)]
    [LibraryImport(Dll)] internal static partial string GameObject_GetName(ulong objectId);

    [LibraryImport(Dll)] internal static partial void GameObject_SetName(ulong objectId, [MarshalAs(UnmanagedType.LPUTF8Str)] string name);

    [LibraryImport(Dll)] internal static partial ulong GameObject_FindByName([MarshalAs(UnmanagedType.LPUTF8Str)] string name);

    [LibraryImport(Dll)] internal static partial void GameObject_SetActive(ulong objectId,[MarshalAs(UnmanagedType.Bool)] bool active);
    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool GameObject_IsActive(ulong objectId);

    [LibraryImport(Dll)] 
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool GameObject_IsValid(ulong objectId);


    [LibraryImport(Dll)]
    internal static partial int GameObject_GetComponentIds(ulong ownerId, 
        [MarshalAs(UnmanagedType.LPUTF8Str)] string? typeName, // null なら全コンポーネント
        ulong[] outBuffer, // 事前に確保されたバッファを渡す方式。呼び出し側でサイズを管理する。
        int bufferSize); // バッファサイズを超える場合は、必要なサイズを返す。0以上の返り値は常に bufferSize 以下でなければならない。

    internal static ulong[] GameObject_GetComponentIdsHelper(ulong ownerId, string? typeName)
    {
        var buffer = new ulong[16]; // 最初は適当なサイズで試す
        int count = GameObject_GetComponentIds(ownerId, typeName, buffer, buffer.Length);
        return buffer[..count]; // 必要なサイズだけ切り取って返す
    }

    [LibraryImport(Dll)]
    internal static partial ulong GameObject_InstantiateFromId(ulong prefabId, ulong parentId, Vector3 position, Quaternion rotation);
    [LibraryImport(Dll)]
    internal static partial ulong GameObject_InstantiateFromResource([MarshalAs(UnmanagedType.LPUTF8Str)] string? resourcePath, ulong parentId, Vector3 position, Quaternion rotation);


    // ------------------------------------ Scene & Entity -----------------------------------------

    //[LibraryImport(Dll)] internal static partial void Scene_Load([MarshalAs(UnmanagedType.LPUTF8Str)] string sceneName);

    //[LibraryImport(Dll)] internal static partial int Entity_GetCount();

    [LibraryImport(Dll)] internal static partial void Entity_Destroy(ulong objectId, float delay); // GameObjectのDestroyと同じ。Entityの破棄はGameObjectの破棄と同義。
    //[LibraryImport(Dll)] internal static partial ulong Entity_Create();
    //[LibraryImport(Dll)] internal static partial ulong Entity_CreateWithName([MarshalAs(UnmanagedType.LPUTF8Str)] string name);
    //[LibraryImport(Dll)] internal static partial void Entity_SetName(ulong objectId, [MarshalAs(UnmanagedType.LPUTF8Str)] string name);
    //[LibraryImport(Dll)] internal static partial void Entity_GetName(ulong objectId, [MarshalAs(UnmanagedType.LPUTF8Str)] StringBuilder name, int maxLength);
    //[LibraryImport(Dll)] internal static partial void Entity_SetActive(ulong objectId, bool active);
    //[LibraryImport(Dll)] internal static partial bool Entity_IsActive(ulong objectId);
    //[LibraryImport(Dll)] internal static partial void Entity_SetStatic(ulong objectId, bool isStatic);
    //[LibraryImport(Dll)] internal static partial bool Entity_IsStatic(ulong objectId);
    //[LibraryImport(Dll)] internal static partial void Entity_SetTag(ulong objectId, [MarshalAs(UnmanagedType.LPUTF8Str)] string tag);
    //[LibraryImport(Dll)] internal static partial void Entity_GetTag(ulong objectId, [MarshalAs(UnmanagedType.LPUTF8Str)] StringBuilder tag, int maxLength);
    //[LibraryImport(Dll)] internal static partial void Entity_AddComponent(ulong ownerId, [MarshalAs(UnmanagedType.LPUTF8Str)] string typeName);
    //[LibraryImport(Dll)] internal static partial void Entity_RemoveComponent(ulong ownerId, [MarshalAs(UnmanagedType.LPUTF8Str)] string typeName);
    //[LibraryImport(Dll)] internal static partial void Entity_GetComponent(ulong ownerId, [MarshalAs(UnmanagedType.LPUTF8Str)] string typeName, out IntPtr componentData);
    [LibraryImport(Dll)] [return: MarshalAs(UnmanagedType.Bool)] internal static partial bool Entity_HasComponent(ulong ownerId, [MarshalAs(UnmanagedType.LPUTF8Str)] string typeName);


    [LibraryImport(Dll)] internal static partial int Scene_GetAllIds(ulong[] outBuffer, int bufferSize); // 事前に確保されたバッファを渡す方式。呼び出し側でサイズを管理する。

    internal static ulong[] Scene_GetAllIdsHelper(int maxBufferSize = 16)
    {
        var buffer = new ulong[maxBufferSize]; // 最初は適当なサイズで試す
        int count = Scene_GetAllIds(buffer, buffer.Length);
        return buffer[..count]; // 必要なサイズだけ切り取って返す
    }

    [LibraryImport(Dll)] internal static partial int Scene_FindAllByType([MarshalAs(UnmanagedType.LPUTF8Str)] string typeName, [Out] ulong[] outBuffer, int bufferSize);

    internal static ulong[] Scene_FindAllByTypeHelper(string typeName, int maxBufferSize = 16)
    {
        var buffer = new ulong[maxBufferSize]; // 最初は適当なサイズで試す
        int count = Scene_FindAllByType(typeName, buffer, buffer.Length);
        return buffer[..count]; // 必要なサイズだけ切り取って返す
    }



    // ------------------------------------ Transform -----------------------------------------

    [LibraryImport(Dll)] internal static partial void Transform_SetParent(ulong objectId, ulong parentId);
    [LibraryImport(Dll)] internal static partial ulong Transform_GetParent(ulong objectId);
    [LibraryImport(Dll)] internal static partial ulong Transform_GetChild(ulong objectId, int index);
    [LibraryImport(Dll)] internal static partial int Transform_GetChildCount(ulong objectId);
    [LibraryImport(Dll)] internal static partial void Transform_SetPosition(ulong objectId, Vector3 pos);
    [LibraryImport(Dll)] internal static partial Vector3 Transform_GetPosition(ulong objectId);
    [LibraryImport(Dll)] internal static partial void Transform_SetLocalPosition(ulong objectId, Vector3 pos);
    [LibraryImport(Dll)] internal static partial Vector3 Transform_GetLocalPosition(ulong objectId);
    [LibraryImport(Dll)] internal static partial void Transform_SetScale(ulong objectId, Vector3 scale);
    //[LibraryImport(Dll)] internal static partial float Transform_GetScaleUniform(ulong objectId);
    [LibraryImport(Dll)] internal static partial Vector3 Transform_GetScale(ulong objectId);
    [LibraryImport(Dll)] internal static partial void Transform_SetLocalScale(ulong objectId, Vector3 scale);
    [LibraryImport(Dll)] internal static partial Vector3 Transform_GetLocalScale(ulong objectId);
    //[LibraryImport(Dll)] internal static partial Vector3 Transform_GetLossyScale(ulong objectId);
    [LibraryImport(Dll)] internal static partial void Transform_SetRotation(ulong objectId, Quaternion rot);
    [LibraryImport(Dll)] internal static partial Quaternion Transform_GetRotation(ulong objectId);

    [LibraryImport(Dll)] internal static partial void Transform_SetLocalRotation(ulong objectId, Quaternion rot);
    [LibraryImport(Dll)] internal static partial Quaternion Transform_GetLocalRotation(ulong objectId);

    [LibraryImport(Dll)] internal static partial void Transform_SetEulerAngles(ulong objectId, Vector3 eulerAngles);
    [LibraryImport(Dll)] internal static partial Vector3 Transform_GetEulerAngles(ulong objectId);

    [LibraryImport(Dll)] internal static partial void Transform_Rotate(ulong objectId, Vector3 eulerAngles);
    [LibraryImport(Dll)] internal static partial void Transform_RotateAround(ulong objectId, Vector3 point, Vector3 axis, float angle);

    [LibraryImport(Dll)] internal static partial void Transform_LookAt(ulong objectId, Vector3 target, Vector3 up);
    [LibraryImport(Dll)] internal static partial void Transform_Translate(ulong objectId, Vector3 translation);

    [LibraryImport(Dll)] internal static partial void Transform_Scaling(ulong objectId, Vector3 scale);

    [LibraryImport(Dll)] internal static partial Vector3 Transform_GetForward(ulong objectId);
    [LibraryImport(Dll)] internal static partial Vector3 Transform_GetUp(ulong objectId);
    [LibraryImport(Dll)] internal static partial Vector3 Transform_GetRight(ulong objectId);
    //[LibraryImport(Dll)] internal static partial void Transform_GetWorldToLocalMatrix(ulong objectId, out Matrix4x4 matrix);
    //[LibraryImport(Dll)] internal static partial void Transform_GetLocalToWorldMatrix(ulong objectId, out Matrix4x4 matrix);
    //[LibraryImport(Dll)] internal static partial void Transform_GetLocalToParentMatrix(ulong objectId, out Matrix4x4 matrix);
    //[LibraryImport(Dll)] internal static partial void Transform_GetParentToLocalMatrix(ulong objectId, out Matrix4x4 matrix);

    // ------------------------------------ Component -----------------------------------------
    [LibraryImport(Dll)] internal static partial int Component_GetEnabled(ulong ownerId, ulong objectId);
    [LibraryImport(Dll)] internal static partial void Component_SetEnabled(ulong ownerId, ulong objectId, int enabled);

    [LibraryImport(Dll)] internal static partial void Component_Destroy(ulong objectId);

    [LibraryImport(Dll)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool Component_IsValid(ulong objectId);

    [LibraryImport(Dll)] internal static partial ulong Component_GetOwner(ulong objectId);



    // ---- Script | HotReload ----
    [LibraryImport(Dll, StringMarshalling = StringMarshalling.Utf8)]
    internal static partial void ScriptNames_Add(string name);

    [LibraryImport(Dll)] internal static partial void ScriptNames_Clear();

}
