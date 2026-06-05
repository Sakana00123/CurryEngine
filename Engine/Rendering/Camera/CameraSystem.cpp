#include "pch.h"
#include "CameraSystem.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Rendering/Camera/CameraComponent.h"

void CameraSystem::Initialize(Scene* scene)
{
	this->scene = scene;
	mainCamera = nullptr;
}

void CameraSystem::ResolveMainCamera()
{
	// シーン内のカメラコンポーネントを検索してメインカメラを設定
	mainCamera = nullptr;

	const auto& cameras = scene->FindComponents<CameraComponent>();
	
	// カメラが一つも存在しない場合は終了
	if (cameras.empty())
	{
		return;
	}

	CameraComponent* firstEnabledCamera = nullptr; // 最初の有効なカメラを記録する変数

	for (auto* cam : cameras)
	{
		if (!cam->IsEnabled())
		{
			continue; // 無効なカメラはスキップ
		}
		// 最初の有効なカメラを記録
		if (!firstEnabledCamera)
		{
			firstEnabledCamera = cam;
		}
		// メインカメラが見つかったら設定してループを抜ける
		if (cam->IsMainCamera())
		{
			mainCamera = cam;
			break;
		}
	}

	// メインカメラが見つからなかった場合、最初の有効なカメラをメインカメラとして設定
	if (!mainCamera)
	{
		mainCamera = firstEnabledCamera;
	}

	// メインカメラのフラグを更新
	for (auto* cam : cameras)
	{
		if (cam)
		{
			cam->isMainCamera = (cam == mainCamera); // メインカメラであれば true、そうでなければ false を設定
		}
	}
}

CameraComponent* CameraSystem::GetMainCamera()
{
	if (needsMainCameraResolve)
	{
		ResolveMainCamera();
		needsMainCameraResolve = false; // 解決を行ったのでフラグをリセット
	}
	return mainCamera;
}

void CameraSystem::NotifyCameraChanged()
{
	needsMainCameraResolve = true;
}