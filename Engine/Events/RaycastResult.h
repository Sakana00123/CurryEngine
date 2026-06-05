#pragma once
#include <memory>
#include "Engine/Core/ObjectId.h"
#include "Engine/Core/Math/Vector2.h"
#include "Engine/UI/Graphic.h"
class GameObject;
class GraphicRaycaster;

struct RaycastResult
{
	GameObject* gameObject = nullptr;//ヒットしたGameObject(UI要素)
	ObjectId hitGraphicId = ObjectId::Invalid(); //ヒットしたGraphicのID(UI要素の描画コンポーネント)

	//float distance = 0.0f;//使ってない
	//Vector3 worldPosition{};//使ってない
	//Vector3 worldNormal{};//使ってない

	Vector2 screenPosition{};

	int sortingLayer = 0;//まだ使ってない
	int sortingOrder = 0;//まだ使ってない
	int depth = 0;//まだ使ってない
	//GraphicRaycaster* module = nullptr;
	ObjectId moduleId = ObjectId::Invalid(); //ヒットしたGraphicRaycasterのID(UI要素の描画コンポーネント) // まだ使ってない

	bool IsValid() const;

	//ヒットしたGameObjectを取得する。存在しない場合はnullptrを返す。
	GameObject* GetHitGameObject() const;

	//ヒットしたGraphicコンポーネントを取得する。存在しない場合はnullptrを返す。
	std::shared_ptr<Graphic> GetHitGraphic() const;
};