#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Physics/Physics.h"


class FixedJoint : public Component
{
	C_REFLECT(FixedJoint)
public:
	FixedJoint() = default;
	~FixedJoint() = default;

public:
	// 接続するオブジェクト
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Rigidbody"))
	ObjectId connectedBody;

private:
	physx::PxFixedJoint* pxJoint = nullptr; // PhysX の固定ジョイントへのポインタ

public:
	// ジョイントを作成する関数
	void CreateJoint();
	// ジョイントを破棄する関数
	void DestroyJoint();
	// Component のライフサイクルイベントでジョイントの管理を行う
	virtual void Start() override;
	void LateUpdate(float deltaTime) override;
	virtual void OnDestroy() override;
};