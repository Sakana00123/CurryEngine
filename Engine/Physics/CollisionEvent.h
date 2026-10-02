#pragma once
#include "Engine/Core/Reflection/Meta.h"
#include "Engine/Core/Math/Vector3.h"
class GameObject;
class Collider;

// 衝突イベントの情報を格納する構造体

/** @brief ContactPoint を表す構造体です。 */
struct ContactPoint
{
	Vector3 point; // 接触点の位置
	Vector3 normal;   //　接触点の法線ベクトル
	float separation; // 接触点でのコライダーの距離
	Collider* thisCollider; // 自身のコライダ
	Collider* otherCollider; // 衝突相手のコライダ
};


/** @brief CollisionInfo を表す構造体です。 */
struct CollisionInfo
{
	GameObject* self; // 自身のGameObject
	Collider* selfCollider; // 自身のコライダー
	GameObject* other; // 衝突相手のGameObject
	Collider* otherCollider; // 衝突相手のコライダー

	std::vector<ContactPoint> contacts; // 複数の接触点の情報を格納する配列
	Vector3 impulse; // 衝突を解決するために互いのコライダに加えられた合計の衝撃量（ベクトル）
};

// トリガーイベントの情報を格納する構造体
/** @brief TriggerInfo を表す構造体です。 */
struct TriggerInfo
{
	GameObject* self; // 自身のGameObject
	Collider* selfCollider; // 自身のコライダー
	GameObject* other; // トリガー相手のGameObject
	Collider* otherCollider; // トリガー相手のコライダー
};

// 衝突イベントのコールバックを処理するインターフェースクラス
/** @brief ICollisionEventCallback を表すクラスです。 */
class ICollisionEventCallback
{
	C_REFLECT(ICollisionEventCallback)
public:
	/**
	 * @brief ICollisionEventCallback を破棄します。
	 */
	virtual ~ICollisionEventCallback() = default;

	/**
	 * @brief 衝突イベントのコールバック関数
	 * @param collisionInfo 衝突イベントの情報を格納する構造体への参照
	 */
	virtual void OnCollisionEnter(const CollisionInfo& collisionInfo) {}
	/**
	 * @brief 衝突イベントのコールバック関数
	 * @param collisionInfo 衝突イベントの情報を格納する構造体への参照
	 */
	virtual void OnCollisionStay(const CollisionInfo& collisionInfo) {}
	/**
	 * @brief 衝突イベントのコールバック関数
	 * @param collisionInfo 衝突イベントの情報を格納する構造体への参照
	 */
	virtual void OnCollisionExit(const CollisionInfo& collisionInfo) {}
};

// トリガーイベントのコールバックを処理するインターフェースクラス
/** @brief ITriggerEventCallback を表すクラスです。 */
class ITriggerEventCallback
{
	C_REFLECT(ITriggerEventCallback)
public:
	/**
	 * @brief ITriggerEventCallback を破棄します。
	 */
	virtual ~ITriggerEventCallback() = default;

	/**
	 * @brief トリガーイベントのコールバック関数
	 * @param triggerInfo トリガーイベントの情報を格納する構造体への参照
	 */
	virtual void OnTriggerEnter(const TriggerInfo& triggerInfo) {}

	/**
	 * @brief トリガーイベントのコールバック関数
	 * @param triggerInfo トリガーイベントの情報を格納する構造体への参照
	 */
	virtual void OnTriggerStay(const TriggerInfo& triggerInfo) {}

	/**
	 * @brief トリガーイベントのコールバック関数
	 * @param triggerInfo トリガーイベントの情報を格納する構造体への参照
	 */
	virtual void OnTriggerExit(const TriggerInfo& triggerInfo) {}
};
