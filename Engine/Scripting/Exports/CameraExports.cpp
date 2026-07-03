#include "pch.h"
#include "Engine/Rendering/Camera/CameraComponent.h"
#include "Engine/Rendering/Camera/CameraSystem.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

/** @brief オブジェクトIDからカメラコンポーネントを取得します。*/
static CameraComponent* GetCameraComponentById(uint64_t objectId)
{
	Scene* scene = SceneManager::GetLoadingSceneOrCurrentScene();
	if (!scene) return nullptr;
	const auto& compMap = scene->objectManager->GetComponentCacheMap();
	if (compMap.contains(ObjectId::FromValue(objectId))) {
		if (auto compPtr = compMap.at(ObjectId::FromValue(objectId)).lock()) {
			if (auto cameraComp = dynamic_cast<CameraComponent*>(compPtr.get())) {
				return cameraComp;
			}
		}
	}
	return nullptr;
}


ENGINE_API uint64_t Camera_GetMainCameraId()
{
	Scene* scene = SceneManager::GetLoadingSceneOrCurrentScene();
	if (!scene) return 0;
	auto mainCamera = scene->cameraSystem.GetMainCamera();
	if (!mainCamera) return 0;
	return mainCamera->GetId().Value();
}

ENGINE_API XMFLOAT4X4 Camera_GetProjectionMatrix(uint64_t objectId)
{
	if (auto cameraComp = GetCameraComponentById(objectId)) {
		auto matrix = cameraComp->GetProjectionMatrix();
		XMFLOAT4X4 m;
		XMStoreFloat4x4(&m, matrix);
		return m;
	}
	return XMFLOAT4X4();
}

ENGINE_API XMFLOAT4X4 Camera_GetViewMatrix(uint64_t objectId)
{
	if (auto cameraComp = GetCameraComponentById(objectId)) {
		auto matrix = cameraComp->GetViewMatrix();
		XMFLOAT4X4 m;
		XMStoreFloat4x4(&m, matrix);
		return m;
	}
	return XMFLOAT4X4();
}


ENGINE_API float Camera_GetFieldOfView(uint64_t objectId)
{
	if (auto cameraComp = GetCameraComponentById(objectId)) {
		return cameraComp->GetFieldOfView();
	}
	return 0.0f;
}

ENGINE_API void Camera_SetFieldOfView(uint64_t objectId, float fov)
{
	if (auto cameraComp = GetCameraComponentById(objectId)) {
		cameraComp->SetFieldOfView(fov);
	}
}

ENGINE_API float Camera_GetAspect(uint64_t objectId)
{
	if (auto cameraComp = GetCameraComponentById(objectId)) {
		return cameraComp->GetAspect();
	}
	return 0.0f;
}

ENGINE_API void Camera_SetAspect(uint64_t objectId, float aspect)
{
	if (auto cameraComp = GetCameraComponentById(objectId)) {
		cameraComp->SetAspect(aspect);
	}
}

ENGINE_API float Camera_GetNearClipPlane(uint64_t objectId)
{
	if (auto cameraComp = GetCameraComponentById(objectId)) {
		return cameraComp->GetNearClip();
	}
	return 0.0f;
}

ENGINE_API void Camera_SetNearClipPlane(uint64_t objectId, float nearClip)
{
	if (auto cameraComp = GetCameraComponentById(objectId)) {
		cameraComp->SetNearClip(nearClip);
	}
}

ENGINE_API float Camera_GetFarClipPlane(uint64_t objectId)
{
	if (auto cameraComp = GetCameraComponentById(objectId)) {
		return cameraComp->GetFarClip();
	}
	return 0.0f;
}

ENGINE_API void Camera_SetFarClipPlane(uint64_t objectId, float farClip)
{
	if (auto cameraComp = GetCameraComponentById(objectId)) {
		cameraComp->SetFarClip(farClip);
	}
}