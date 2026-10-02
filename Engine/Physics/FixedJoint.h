#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Physics/Physics.h"


/** @brief FixedJoint を表すクラスです。 */
class FixedJoint : public Component
{
	C_REFLECT(FixedJoint)
public:
	/**
	 * @brief FixedJoint を構築します。
	 */
	FixedJoint() = default;
	/**
	 * @brief FixedJoint を破棄します。
	 */
	~FixedJoint() = default;

public:
	// 接続するオブジェクト
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Rigidbody"))
	ObjectId connectedBody;

private:
	physx::PxFixedJoint* pxJoint = nullptr; // PhysX の固定ジョイントへのポインタ

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
	virtual void Start() override;
	/**
	 * @brief LateUpdate の処理を行います。
	 */
	void LateUpdate(float deltaTime) override;
	/**
	 * @brief Destroy イベントを処理します。
	 */
	virtual void OnDestroy() override;
};