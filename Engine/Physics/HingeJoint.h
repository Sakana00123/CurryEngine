#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Physics/Physics.h"

class HingeJoint : public Component
{
	C_REFLECT(HingeJoint)
public:
	HingeJoint() = default;
	~HingeJoint() = default;

public:
	// 接続するオブジェクト
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Rigidbody"))
	ObjectId connectedBody;

private:
	physx::PxRevoluteJoint* pxJoint = nullptr; // PhysX の回転ジョイントへのポインタ

public:
	// ジョイントを作成する関数
	void CreateJoint();
	// ジョイントを破棄する関数
	void DestroyJoint();
	// Component のライフサイクルイベントでジョイントの管理を行う
	void Start() override;
	void LateUpdate(float deltaTime) override;
	void OnDestroy() override;
};