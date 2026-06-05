#include "pch.h"
#include "RaycastResult.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

bool RaycastResult::IsValid() const
{
	if (gameObject == nullptr) {
		return false;
	}
	
	// ヒットしたGraphicコンポーネントが存在するか確認
	if (Scene* scene = SceneManager::GetLoadingSceneOrCurrentScene()) {
		if (scene->FindComponentById<Graphic>(hitGraphicId) == nullptr) {
			return false;
		}
	}
	return true;
}

GameObject* RaycastResult::GetHitGameObject() const
{
	// 無効な結果の場合は nullptr を返す
	if (!IsValid()) {
		return nullptr;
	}
	return gameObject;
}

std::shared_ptr<Graphic> RaycastResult::GetHitGraphic() const
{
	// 無効な結果の場合は nullptr を返す
	if (!IsValid() || !hitGraphicId.IsValid()) {
		return nullptr;
	}

	// ヒットしたGameObjectが所属するシーンからGraphicコンポーネントを取得
	if (Scene* scene = gameObject->GetScene()) {
		return scene->FindComponentPtrById<Graphic>(hitGraphicId);
	}

	return nullptr;
}