#include "pch.h"
#include "FixedJoint.h"
#include "Rigidbody.h"
#include "Engine/Scenes/Scene.h"
#include <PxPhysicsAPI.h>

REGISTER_COMPONENT_WITH_ATTRIBUTES(FixedJoint, "Physics", ComponentAttributes::RequiredComponent, {"Rigidbody"})

void FixedJoint::Start()
{
	//CreateJoint();
}

void FixedJoint::LateUpdate(float deltaTime)
{
	// connectedBody が無効になった場合はジョイントを破棄します。
	if (!connectedBody.IsValid())
	{
		DestroyJoint();
	}
	if (!pxJoint)
	{
		// connectedBody が有効で、まだジョイントが作成されていない場合はジョイントを作成します。
		CreateJoint();
	}
}

void FixedJoint::OnDestroy()
{
	DestroyJoint();
}


void FixedJoint::CreateJoint()
{
	// ここで PhysX の固定ジョイントを作成する処理を実装します。
	// connectedBody が有効な場合にのみジョイントを作成します。
	if (connectedBody.IsValid())
	{
		// connectedBody から Rigidbody コンポーネントを取得して、PhysX のアクターを取得します。
		if (const auto& connectedRigidbody = std::reinterpret_pointer_cast<Rigidbody>(ObjectManager::FindComponent(connectedBody)))
		{
			physx::PxRigidActor* actorA = Physics::GetActor(Physics::GetActorHandle(GetOwner()->GetTransform()));
			physx::PxRigidActor * actorB = Physics::GetActor(Physics::GetActorHandle(connectedRigidbody->GetTransform()));
			if (actorA && actorB)
			{
				// ジョイントを作成します。ここでは、両方のアクターのローカルフレームを単位変換して渡す必要があります。
				physx::PxTransform localFrameA = physx::PxTransform(physx::PxVec3(0,0.5f, 0), physx::PxQuat(0, 0, 0, 1)); // 必要に応じてローカルフレームを設定
				physx::PxTransform localFrameB = physx::PxTransform(physx::PxVec3(0, 0, 0), physx::PxQuat(0, 0, 0, 1)); // 必要に応じてローカルフレームを設定

				// 双方の位置の中点を計算して、ローカルフレームの位置を調整します。
				{
					physx::PxVec3 posA = actorA->getGlobalPose().p;
					physx::PxVec3 posB = actorB->getGlobalPose().p;
					physx::PxVec3 midPoint = (posA + posB) * 0.5f;
					localFrameA.p = midPoint - posA;
					localFrameB.p = midPoint - posB;
				}

				// PhysX の固定ジョイントを作成します。
				pxJoint = physx::PxFixedJointCreate(*Physics::GetPhysics(), actorA, localFrameA, actorB, localFrameB);
			}
		}
	}
}

void FixedJoint::DestroyJoint()
{
	if (pxJoint)
	{
		pxJoint->release();
		pxJoint = nullptr;
	}
}