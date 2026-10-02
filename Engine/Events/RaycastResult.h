#pragma once
#include <memory>
#include "Engine/Core/ObjectId.h"
#include "Engine/Core/Math/Vector2.h"
#include "Engine/UI/Graphic.h"
class GameObject;
class GraphicRaycaster;

/** @brief RaycastResult を表す構造体です。 */
struct RaycastResult
{
	GameObject* gameObject = nullptr;//ヒットしたGameObject(UI要素)
	/**
	 * @brief Invalid の処理を行います。
	 */
	ObjectId hitGraphicId = ObjectId::Invalid(); //ヒットしたGraphicのID(UI要素の描画コンポーネント)

	//float distance = 0.0f;//使ってない
	//Vector3 worldPosition{};//使ってない
	//Vector3 worldNormal{};//使ってない

	Vector2 screenPosition{};

	int sortingLayer = 0;//まだ使ってない
	int sortingOrder = 0;//まだ使ってない
	int depth = 0;//まだ使ってない
	//GraphicRaycaster* module = nullptr;
	/**
	 * @brief Invalid の処理を行います。
	 */
	ObjectId moduleId = ObjectId::Invalid(); //ヒットしたGraphicRaycasterのID(UI要素の描画コンポーネント) // まだ使ってない

	/**
	 * @brief IsValid の条件を満たすか判定します。
	 * @return 処理結果を返します。
	 */
	bool IsValid() const;

	//ヒットしたGameObjectを取得する。存在しない場合はnullptrを返す。
	/**
	 * @brief GetHitGameObject に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	GameObject* GetHitGameObject() const;

	//ヒットしたGraphicコンポーネントを取得する。存在しない場合はnullptrを返す。
	/**
	 * @brief GetHitGraphic に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	std::shared_ptr<Graphic> GetHitGraphic() const;
};