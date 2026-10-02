#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Physics/Physics.h"

/** @brief HingeJoint を表すクラスです。 */
class HingeJoint : public Component
{
	C_REFLECT(HingeJoint)
public:
	/**
	 * @brief HingeJoint を構築します。
	 */
	HingeJoint() = default;
	/**
	 * @brief HingeJoint を破棄します。
	 */
	~HingeJoint() = default;

public:
	// 接続するオブジェクト
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Rigidbody"))
	ObjectId connectedBody;

private:
	physx::PxRevoluteJoint* pxJoint = nullptr; // PhysX の回転ジョイントへのポインタ

public:
	// ジョイントを作成する関数
	/**
	 * @brief 新しい要素を生成します。
	 */
	void CreateJoint();
	// ジョイントを破棄する関数
	/**
	 * @brief DestroyJoint の処理を行います。
	 */
	void DestroyJoint();
	// Component のライフサイクルイベントでジョイントの管理を行う
	/**
	 * @brief Start の処理を行います。
	 */
	void Start() override;
	/**
	 * @brief LateUpdate の処理を行います。
	 */
	void LateUpdate(float deltaTime) override;
	/**
	 * @brief Destroy イベントを処理します。
	 */
	void OnDestroy() override;
};